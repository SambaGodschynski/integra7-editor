#pragma once

#include <unordered_map>
#include <thread>
#include <components/I7Host.h>

class AI7Host : public I7Host
{
private:
	typedef std::mutex Mutex;
	typedef i7::UInt RequestAddress;
	typedef std::unordered_multimap<RequestAddress, RequestResponsePromise> OpenRequests;
	std::mutex midiRqMutex;
	OpenRequests openRequests;
public:
	virtual ~AI7Host() = default;
	virtual void requestExpansion() override;
	virtual RequestResponseFuture request(const i7::RequestInfo& requestInfo) override;
	virtual void onSysexRexecived(const unsigned char*, size_t);
};
