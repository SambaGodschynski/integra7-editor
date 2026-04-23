#include "Midi.h"
#include <chrono>
#include <iomanip>

namespace
{
    const int ThreadIdleMillis = 1;
    template<class TMidiIO>
    void printMidiIo(std::ostream &os)
    {
        TMidiIO midiIo;
        auto nIos = midiIo.getPortCount();
        for (size_t idx = 0; idx < nIos; ++idx)
        {
            os << (idx) << ": " << midiIo.getPortName(idx) << std::endl;
        }
    }
    template<class TMidiIO>
    void openPort(TMidiIO &port, size_t portNr)
    {
        auto nIos = port.getPortCount();
        if (portNr >= nIos)
        {
            std::cerr << "invalid port number: " << portNr << std::endl;
            exit(1);
        }
        port.openPort(portNr);
        std::cout << "open: (" << portNr << ") " << port.getPortName(portNr) << std::endl;
    }
    std::ostream & operator <<(std::ostream &os, const Bytes &bytes)
    {
        if (bytes.empty())
        {
            return os;
        }
        os << std::hex << std::setfill('0');
        for (auto byte : bytes)
        {
            os << " " << std::setw(2) << (int)byte;
        }
        os << std::dec;
        return os;
    }
}


 Midi::~Midi()
 {
    if (thread)
    {
        running = false;
        thread->join();
        delete thread;
    }
 }

 void Midi::printInputs(std::ostream &os)
{
    printMidiIo<RtMidiIn>(os);
}

void Midi::printOutputs(std::ostream &os)
{
    printMidiIo<RtMidiOut>(os);
}

void Midi::openInput(int index)
{
    inport = index;    
}

void Midi::openOutput(int index)
{
    outport = index;
}

void Midi::start()
{
    if (thread == nullptr)
    {
        running = true;
        thread = new std::thread(std::bind(&Midi::runThread, this));
    }
}

void Midi::sendMessage(Bytes message)
{
    QueueItem item
    {
        .rq = std::move(message),
    };
    enqueue(std::move(item));
}

void Midi::sendAndReceive(Bytes rq, void *usrData, OnReceivedCallback callback, bool multiResponse, int gapTimeoutMs, uint32_t stopOnAddr)
{
    QueueItem item
    {
        .rq = std::move(rq),
        .callback = std::move(callback),
        .userData = usrData,
        .multiResponse = multiResponse,
        .gapTimeoutMs = gapTimeoutMs,
        .stopOnAddr   = stopOnAddr
    };
    enqueue(std::move(item));
}

void Midi::enqueue(QueueItem item)
{
    std::lock_guard<Lock> _lock(lock);
    queue.push(std::move(item));
}

int Midi::getInputPortCount()
{
    RtMidiIn tmp;
    return (int)tmp.getPortCount();
}

std::string Midi::getInputPortName(int index)
{
    RtMidiIn tmp;
    if (index < 0 || (size_t)index >= tmp.getPortCount())
    {
        return "";
    }
    return tmp.getPortName((size_t)index);
}

int Midi::getOutputPortCount()
{
    RtMidiOut tmp;
    return (int)tmp.getPortCount();
}

std::string Midi::getOutputPortName(int index)
{
    RtMidiOut tmp;
    if (index < 0 || (size_t)index >= tmp.getPortCount())
    {
        return "";
    }
    return tmp.getPortName((size_t)index);
}

void Midi::reopenInput(int index)
{
    pendingInPort.store(index);
}

void Midi::reopenOutput(int index)
{
    pendingOutPort.store(index);
}

void Midi::cancelPending()
{
    cancelRequested.store(true);
}

void Midi::runThread()
{
    RtMidiIn  midiIn;
    RtMidiOut midiOut;
    if (inport >= 0 && (size_t)inport < midiIn.getPortCount())
    {
        openPort(midiIn, (size_t)inport);
        midiIn.ignoreTypes(false, false, false);
    }
    else if (inport >= 0)
    {
        std::cerr << "MIDI in port " << inport << " not available, skipping." << std::endl;
    }
    if (outport >= 0 && (size_t)outport < midiOut.getPortCount())
    {
        openPort(midiOut, (size_t)outport);
    }
    else if (outport >= 0)
    {
        std::cerr << "MIDI out port " << outport << " not available, skipping." << std::endl;
    }
    while (running)
    {
        // Handle pending port hot-swap requests
        {
            int wantIn = pendingInPort.exchange(-2);
            if (wantIn != -2)
            {
                if (midiIn.isPortOpen())
                {
                    midiIn.closePort();
                }
                if (wantIn >= 0 && (size_t)wantIn < midiIn.getPortCount())
                {
                    midiIn.openPort((size_t)wantIn);
                    midiIn.ignoreTypes(false, false, false);
                    std::cout << "reopen in: (" << wantIn << ") "
                              << midiIn.getPortName((size_t)wantIn) << std::endl;
                }
            }
            int wantOut = pendingOutPort.exchange(-2);
            if (wantOut != -2)
            {
                if (midiOut.isPortOpen())
                {
                    midiOut.closePort();
                }
                if (wantOut >= 0 && (size_t)wantOut < midiOut.getPortCount())
                {
                    midiOut.openPort((size_t)wantOut);
                    std::cout << "reopen out: (" << wantOut << ") "
                              << midiOut.getPortName((size_t)wantOut) << std::endl;
                }
            }
        }
        {
            while (true)
            {
                QueueItem item;
                {
                    std::lock_guard<Lock> _lock(lock);
                    if (queue.empty()) { break; }
                    item = std::move(queue.front());
                    queue.pop();
                }
                handle(item, midiIn, midiOut);
                if (cancelRequested.exchange(false))
                {
                    std::lock_guard<Lock> _lock(lock);
                    while (!queue.empty()) { queue.pop(); }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(ThreadIdleMillis));
    }
    if (midiIn.isPortOpen())
    {
        midiIn.closePort();
    }
    if (midiOut.isPortOpen())
    {
        midiOut.closePort();
    }
}

void Midi::handle(const Midi::QueueItem &item, RtMidiIn &midiIn, RtMidiOut &midiOut)
{
    if (item.callback)
    {
        // Sleep briefly so any echo from a preceding DT1 (sendMessage) has time
        // to arrive at MIDI in, then flush it before sending the RQ1.
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        { Bytes stale; do { midiIn.getMessage(&stale); } while (!stale.empty()); }
    }
    onSend();
    if (verbose)
    {
        std::cout << "[TX]" << item.rq << std::endl;
    }
    midiOut.sendMessage(&item.rq);
    if (!item.callback)
    {
        return;
    }

    Bytes answer;
    const int idleMillis = 10;

    if (item.multiResponse)
    {
        // Collect multiple responses until a gap (no new message) or total timeout.
        const int gapTimeoutMs  = item.gapTimeoutMs;
        const int totalTimeoutMs = 15000;
        int gapMs   = 0;
        int totalMs = 0;
        bool stoppedOnAddr = false;
        while (totalMs < totalTimeoutMs && gapMs < gapTimeoutMs && !cancelRequested.load())
        {
            midiIn.getMessage(&answer);
            if (!answer.empty())
            {
                if (verbose)
                {
                    std::cout << "[RX]" << answer << std::endl;
                }
                onReceive();
                item.callback(answer, item.userData);
                if (item.stopOnAddr != 0 && answer.size() >= 11)
                {
                    uint32_t addr = ((uint32_t)answer[7] << 24) | ((uint32_t)answer[8] << 16)
                                  | ((uint32_t)answer[9]  <<  8) |  (uint32_t)answer[10];
                    if (addr == item.stopOnAddr) { stoppedOnAddr = true; break; }
                }
                answer.clear();
                gapMs = 0;
            }
            else
            {
                gapMs   += idleMillis;
                totalMs += idleMillis;
                std::this_thread::sleep_for(std::chrono::milliseconds(idleMillis));
            }
        }
        if (!stoppedOnAddr)
        {
            item.callback(Bytes(), item.userData);  // gap/total timeout: push done sentinel
        }
        return;
    }

    const int timeOutSeconds = 5;
    int maxTries = (int(1000 / (double)idleMillis) * timeOutSeconds);
    while (maxTries-- > 0 && !cancelRequested.load())
    {
        midiIn.getMessage(&answer);
        if (!answer.empty())
        {
            if (verbose)
            {
                std::cout << "[RX]" << answer << std::endl;
            }
            onReceive();
            item.callback(std::move(answer), item.userData);
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(idleMillis));
    }
    item.callback(Bytes(), nullptr);
}