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

void Midi::runThread()
{
    RtMidiIn  midiIn;
    RtMidiOut midiOut;
    if (inport >= 0)
    {
        openPort(midiIn, (size_t)inport);
        midiIn.ignoreTypes(false, false, false);
    }
    if (outport >= 0)
    {
        openPort(midiOut, (size_t)outport);
    }
    while (running)
    {
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
    std::cerr << "MIDI receive timed out." << std::endl;
    item.callback(Bytes(), nullptr);
}