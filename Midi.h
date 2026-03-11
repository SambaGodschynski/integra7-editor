#include <rtmidi/RtMidi.h>
#include <ostream>
#include "Com.h"
#include <functional>
#include <list> // TODO: use queue
#include <mutex>
#include <thread>

class Midi
{
public:
    typedef std::function<void(Bytes, void*)> OnReceivedCallback;
    struct QueueItem
    {
        Bytes rq;
        OnReceivedCallback callback;
        void *userData = nullptr;
    };
private:
    int inport = -1, outport = -1;
    bool running;
    typedef std::mutex Lock;
    typedef std::list<QueueItem> Queue;
    std::thread *thread = nullptr;
    Lock lock;
    Queue queue;
    void enqueue(QueueItem item);
    void runThread();
    void handle(const Midi::QueueItem &item, RtMidiIn &midiIn, RtMidiOut &midiOut);
public:
    std::function<void()> onSend = [](){};
    std::function<void()> onReceive = [](){};
    virtual ~Midi();
    void start();
    void printInputs(std::ostream &os);
    void printOutputs(std::ostream &os);
    void openInput(int index);
    void openOutput(int index);
    void sendMessage(const Bytes& message);
    void sendAndReceive(Bytes rq, void *usrData, OnReceivedCallback callback);
};