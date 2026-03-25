#include <rtmidi/RtMidi.h>
#include <ostream>
#include "Com.h"
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <string>

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
    std::atomic<int> pendingInPort{-2};   // -2 = no-op sentinel
    std::atomic<int> pendingOutPort{-2};
    typedef std::mutex Lock;
    typedef std::queue<QueueItem> Queue;
    std::thread *thread = nullptr;
    Lock lock;
    Queue queue;
    void enqueue(QueueItem item);
    void runThread();
    void handle(const Midi::QueueItem &item, RtMidiIn &midiIn, RtMidiOut &midiOut);
public:
    bool verbose = false;
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
    int  getInputPortCount();
    std::string getInputPortName(int index);
    int  getOutputPortCount();
    std::string getOutputPortName(int index);
    void reopenInput(int index);    // -1 = close
    void reopenOutput(int index);   // -1 = close
};