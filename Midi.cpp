#include "Midi.h"
#include <chrono>

namespace
{
    const int ThreadIdleMillis = 1;
}

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

void Midi::sendMessage(const Bytes& message)
{
    QueueItem item
    {
        .rq = std::move(message),
    };
    enqueue(std::move(item));
}

void Midi::sendAndReceive(Bytes rq, void *usrData, OnReceivedCallback callback)
{
    QueueItem item
    {
        .rq = std::move(rq),
        .callback = std::move(callback),
        .userData = usrData
    };
    enqueue(std::move(item));
}

void Midi::enqueue(QueueItem item)
{
    std::lock_guard<Lock> _lock(lock);
    queue.emplace_front(std::move(item));
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
            while(!queue.empty())
            {
                const QueueItem &item = queue.back();
                handle(item, midiIn, midiOut);
                {
                    std::lock_guard<Lock> _lock(lock);
                    queue.pop_back();
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
    midiOut.sendMessage(&item.rq);
    if (!item.callback)
    {
        return;
    }

    Bytes answer;
    const int idleMillis = 10;
    const int timeOutSeconds = 5;
    int maxTries = (int(1000 / (double)idleMillis) * timeOutSeconds);
    while (maxTries-- > 0)
    {
        midiIn.getMessage(&answer);
        if (!answer.empty())
        {
            onReceive();
            item.callback(std::move(answer), item.userData);
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(idleMillis));
    }
    item.callback(Bytes(), nullptr);
}