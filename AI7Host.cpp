#include "AI7Host.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <Helper.h>
#include <chrono>
#include <stdexcept>
#include <components/Common.h>
#include <cassert>
namespace 
{
	constexpr long long MaxRequestTimeOutSeconds = 5;
	template<class TFuture> auto waitForRequest(TFuture& future)
	{
		assert(future.valid());
		auto result = future.wait_for(std::chrono::seconds(MaxRequestTimeOutSeconds));
		if (result != std::future_status::ready)
		{
			throw std::runtime_error("request timeout, please check your MIDI connection (in/out) to the INTEGRA7");
		}
		auto response = future.get();
		return response;
	}
}

void AI7Host::requestExpansion()
{
	i7::RequestInfo rqInfo;
	rqInfo.addr = i7::REQ_READ_EXP_ADDR;
	rqInfo.size = 0x0;
	auto future = request(rqInfo);
	auto response = waitForRequest(future);
	expansions[0] = (i7::Expansion)response.payload[0];
	expansions[1] = (i7::Expansion)response.payload[1];
	expansions[2] = (i7::Expansion)response.payload[2];
	expansions[3] = (i7::Expansion)response.payload[3];
}

void AI7Host::requestToneSna(int partNumber)
{
	assert(partNumber >=0 && partNumber<= 6);
	constexpr i7::UInt ADDR_BASE = 0x18002006;
	i7::RequestInfo rqInfo;
	rqInfo.addr = ADDR_BASE + 0x100 * (i7::UInt)partNumber;
	rqInfo.addr = 1;

}

I7Host::RequestResponseFuture AI7Host::request(const i7::RequestInfo& requestInfo)
{
	auto sysexData = i7::createRq1SysexData(requestInfo);
	RequestResponseFuture result;
	{
		const std::lock_guard<Mutex> lock(midiRqMutex);
		RequestResponsePromise promise;
		result = promise.get_future();
		openRequests.emplace(requestInfo.addr, std::move(promise));
	}
	sendSysex(sysexData.data(), sysexData.size());
	return result;
}

void AI7Host::onSysexRexecived(const unsigned char* sysexData, size_t numBytes)
{
	DEBUGONLY(std::cout << "R:" << bytesToString(sysexData, sysexData + numBytes) << std::endl);
	auto response = i7::getResponseData(sysexData, numBytes);
	OpenRequests::iterator it;
	{
		const std::lock_guard<Mutex> lock(midiRqMutex);
		it = openRequests.find(response.addr);
	}
	if (it == openRequests.end())
	{
		return;
	}
	it->second.set_value(response);
	{
		const std::lock_guard<Mutex> lock(midiRqMutex);
		openRequests.erase(it);
	}
}