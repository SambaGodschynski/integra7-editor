#include "Model.h"
#include <sstream>
#include <iostream>
#include <functional>
#include "ModelIds.h"
#include <cassert>

namespace i7
{
    namespace
    {
        void valueToBytes(UInt, ValueByteSizeType, Byte* outBytes);
        void valueToBytes(const std::string&, ValueByteSizeType, Byte* outBytes);
        UInt bytesToValue(const Byte* bytes, ValueByteSizeType byteSizeType);
        template<class TIterator>
        Byte chksum(TIterator begin, TIterator end);
        constexpr UInt ADDR_SIZE = 4;
        constexpr UInt DATA_SIZE = 4;
        constexpr UInt SIZE_F7 = 1;
        constexpr UInt SIZE_CHKSM = 1;
        constexpr Byte ROLAND = 0x41;
        constexpr Byte DEV_ID = 0x10;
        constexpr UInt DeviceIdOffset = 2;
        constexpr Byte MODEL_ID[] = { 0, 0, 0x64 };
        constexpr Byte RQ1 = 0x11;
        constexpr Byte DT1 = 0x12;
        constexpr Byte ROLAND_SYSEX[] = { 0xF0,  0x41, DEV_ID, MODEL_ID[0], MODEL_ID[1] , MODEL_ID[2] };
        constexpr Byte ROLAND_DT1[] = { ROLAND_SYSEX[0], ROLAND_SYSEX[1], ROLAND_SYSEX[2], ROLAND_SYSEX[3], ROLAND_SYSEX[4], ROLAND_SYSEX[5], DT1 };
        constexpr Byte ROLAND_RQ1[] = { ROLAND_SYSEX[0], ROLAND_SYSEX[1], ROLAND_SYSEX[2], ROLAND_SYSEX[3], ROLAND_SYSEX[4], ROLAND_SYSEX[5], RQ1 };
        constexpr UInt SizeRolandDt1 = sizeof(ROLAND_DT1) / sizeof(ROLAND_DT1[0]);
        constexpr UInt SizeRolandRq1 = sizeof(ROLAND_RQ1) / sizeof(ROLAND_RQ1[0]);
    }
    NodeInfo getNode(const char* id)
    {
        NodeInfo result;
        const Node* nodes = &root[0];
        const Node* node = nullptr;
        UInt numNodes = NumRootNodes;
        std::string partId;
        std::stringstream ss(id);
        while (std::getline(ss, partId, '-'))
        {
            if (partId == "PRM") 
            {
                // prm has no data representation
                continue;
            }
            for (UInt i = 0; i < numNodes; ++i)
            {
                const Node* n = &nodes[i];
                if (partId == std::string(n->id))
                {
                    result.addr += getModelIdAddress(partId);
                    node = n;
                    if (ss.eof()) 
                    {
                        result.node = node;
                        return result;
                    }
                    nodes = &node->node[0];
                    numNodes = node->numChildren;
                    break;
                }
            }
            if (nodes == &root[0]) 
            {
                // found nothing
                return result;
            }
        }
        return result;
    }

    NodeInfos getLeafNodes(const char* id)
    {
        NodeInfos result;
        NodeInfo parent = getNode(id);
        if (parent.node == nullptr)
        {
            return result;
        }
        std::function<UInt(NodeInfo)> walk = [&parent, &result, &walk](NodeInfo nodeInfo)
        {
            if (nodeInfo.node == nullptr)
            {
                return UInt(0);
            }
            UInt offset = 0;
            for (size_t i = 0; i < nodeInfo.node->numChildren; ++i)
            {
                
                auto childNode = &nodeInfo.node->node[i];
                bool isLeaf = childNode->numChildren == 0;
                
                NodeInfo childNodeInfo;
                childNodeInfo.node = childNode;
                childNodeInfo.addr = nodeInfo.addr + getModelIdAddress(childNode->id);
                if (isLeaf)
                {
                    result.push_back(childNodeInfo);
                    offset += getByteSize(childNode->valueByteSizeType);
                }
                else 
                {
                    offset += walk(childNodeInfo);
                }
            }
            return offset;
        };
        walk(parent);
        return result;
    }

    void put(ModelData* model, const NodeInfo& nodeInfo, UInt v)
    {
        if ((UInt)nodeInfo.node->valueByteSizeType > (UInt)INTEGER_MASK) {
            v = std::max(nodeInfo.node->min, std::min(nodeInfo.node->max, v));
        }
        assert(nodeInfo.node->isLeaf());
        ModelValue& modelValue = (*model)[nodeInfo.addr];
        valueToBytes(v, nodeInfo.node->valueByteSizeType, &(modelValue.bytes[0]));
    }
    void put(ModelData* model, const NodeInfo& nodeInfo, const std::string &str)
    {
        assert(nodeInfo.node->isLeaf());
        ModelValue& modelValue = (*model)[nodeInfo.addr];
        valueToBytes(str, nodeInfo.node->valueByteSizeType, &(modelValue.bytes[0]));
    }

    void put(ModelData* model, const NodeInfo& nodeInfo, const Byte* src, size_t numBytes)
    {
        assert(nodeInfo.node->isLeaf());
        assert(getByteSize(nodeInfo.node->valueByteSizeType) == numBytes);
        ModelValue& value = (*model)[nodeInfo.addr];
        ::memcpy(&value.bytes[0], src, numBytes);
    }

    UInt get(const ModelData* model, const NodeInfo& nodeInfo)
    {
        assert(nodeInfo.node->isLeaf());
        auto it = model->find(nodeInfo.addr);
        if (it == model->end())
        {
            return 0;
        }
        const ModelValue& modelValue = it->second;
        return bytesToValue(&(modelValue.bytes[0]), nodeInfo.node->valueByteSizeType);
    }
    
    std::string getString(const ModelData* model, const NodeInfo& nodeInfo)
    {
        assert(nodeInfo.node->isLeaf());
        auto it = model->find(nodeInfo.addr);
        if (it == model->end())
        {
            return std::string();
        }
        const ModelValue& modelValue = it->second;

        UInt byteSize = getByteSize(nodeInfo.node->valueByteSizeType);
        char result[Integra7MaxValueBytes + 1] = {0};
        const Byte* begin = &(modelValue.bytes[0]);
        const Byte* end = begin + byteSize - 1;
        for(; end >= begin; --end)
        {
            UInt i = (UInt)(end - begin);
            Byte c = *(end);
            if (c == 0x20 || c == 0)
            {
                continue;
            }
            result[i] = c;
        }
        return std::string(&result[0]);
    }

    Bytes createRq1SysexData(const RequestInfo& sendInfo)
    {
        UInt addr = sendInfo.addr;
        Bytes result;
        result.reserve(SizeRolandRq1 + ADDR_SIZE + sendInfo.size + SIZE_CHKSM + SIZE_F7);
        result.insert(result.begin(), &ROLAND_RQ1[0], &ROLAND_RQ1[SizeRolandRq1]);
        result.push_back((addr >> 24) & 0xff);
        result.push_back((addr >> 16) & 0xff);
        result.push_back((addr >> 8) & 0xff);
        result.push_back(addr & 0xff);
        size_t bytes = sendInfo.sizeNumBytes > 0  ? sendInfo.sizeNumBytes : ADDR_SIZE;
        while(bytes-- > 0)
        {
            result.push_back((sendInfo.size >> (bytes*8)) & 0xff);
        }
        result.push_back(chksum(result.begin() + SizeRolandRq1, result.end()));
        result.push_back(0xf7);
        return result;
    }

    Bytes createSysexData(const ModelData* model, const NodeInfo& nodeInfo)
    {
        static ModelValue nullValue;
        assert(nodeInfo.node->isLeaf());
        auto modelIt = model->find(nodeInfo.addr);
        const ModelValue& modelValue = modelIt != model->end() ? modelIt->second : nullValue;

        UInt addr = nodeInfo.addr;
        UInt resultSize = getByteSize(nodeInfo.node->valueByteSizeType);
        Bytes result;
        result.reserve(SizeRolandDt1 + ADDR_SIZE + resultSize + SIZE_CHKSM + SIZE_F7);
        result.insert(result.begin(), &ROLAND_DT1[0], &ROLAND_DT1[SizeRolandDt1]);
        result.push_back((addr >> 24) & 0xff);
        result.push_back((addr >> 16) & 0xff);
        result.push_back((addr >> 8) & 0xff);
        result.push_back(addr & 0xff);
        const Byte* it = &(modelValue.bytes[0]);
        const Byte* end = it + resultSize;
        for (; it != end; ++it)
        {
            result.push_back(*it);
        }
        result.push_back(chksum(result.begin() + SizeRolandDt1, result.end()));
        result.push_back(0xf7);
        return result;
    }

    Bytes createSysexData(const ModelData* model, const NodeInfo& nodeData, Byte deviceId)
    {
        Bytes result = createSysexData(model, nodeData);
        result[DeviceIdOffset] = deviceId;
        return result;
    }

    RequestResponse getResponseData(const Byte* bytes, size_t numBytes)
    {
        const size_t responseOverhead = 11;
        RequestResponse response;
        if (((int)numBytes - (int)responseOverhead) <= 0)
        {
            return response;
        }
        size_t i = 0;
        ++i; // 0xF0
        ++i; // 0x41
        response.deviceId = bytes[i++];
        response.modelId = (bytes[i++] << 16) + (bytes[i++] << 8) + bytes[i++];
        response.requestType = bytes[i++];
        response.addr = (bytes[i++] << 24) + (bytes[i++] << 16) + (bytes[i++] << 8) + bytes[i++];
        response.payload = &bytes[i];
        response.numBytes = numBytes - i - 1 - 1; // -1 = f7 -1 = checksum
        return response;
    }
    //-------------------------------------------------------------------------
    namespace
    {
        void valueToBytes(UInt v, ValueByteSizeType byteSizeType, Byte* outBytes)
        {
            switch (byteSizeType)
            {
            case INTEGER1x1:
            case INTEGER1x2:
            case INTEGER1x3:
            case INTEGER1x4:
            case INTEGER1x5:
            case INTEGER1x6:
            case INTEGER1x7:
                outBytes[0] = (v & 0x7f);
                break;

            case INTEGER2x4:
                outBytes[0] = ((v >> 4) & 0xf);
                outBytes[1] = (v & 0xf);
                break;

            case INTEGER4x4:
                outBytes[0] = ((v >> 12) & 0xf);
                outBytes[1] = ((v >> 8) & 0xf);
                outBytes[2] = ((v >> 4) & 0xf);
                outBytes[3] = (v & 0xf);
                break;
            case ZeroByteSize:
            case INTEGER_MASK:
            case ByteSize12:
            case ByteSize16:
            case UndefinedByteType:
            default: /* ASCII String */
                throw std::runtime_error("unexpected byte size type");
            }
        }
        UInt bytesToValue(const Byte* bytes, ValueByteSizeType byteSizeType)
        {
            switch (byteSizeType)
            {
            case INTEGER1x1:
            case INTEGER1x2:
            case INTEGER1x3:
            case INTEGER1x4:
            case INTEGER1x5:
            case INTEGER1x6:
            case INTEGER1x7: return (UInt)bytes[0];

            case INTEGER2x4: {
                return (((UInt)bytes[0]) << 4)
                     + (((UInt)bytes[1]));
                break;
            }

            case INTEGER4x4:
                return (((UInt)bytes[0]) << 12)
                     + (((UInt)bytes[1]) << 8)
                     + (((UInt)bytes[2]) << 4)
                     + (((UInt)bytes[3]));

            case ZeroByteSize:
            case INTEGER_MASK:
            case ByteSize12:
            case ByteSize16:
            case UndefinedByteType:
            default: /* ASCII String */
                throw std::runtime_error("unexpected byte size type");
            }
        }
        void valueToBytes(const std::string& str, ValueByteSizeType valueByteSizeType, Byte* outBytes)
        {
            Byte* it = outBytes;
            Byte* end = outBytes + getByteSize(valueByteSizeType);
            UInt str_length = (UInt)str.length();
            UInt i = 0;
            for (; it != end; ++it) 
            {
                Byte c = i < str_length ? (Byte)str.at(i) : 0x20;
                *it = c;
                i++;
            }
        }
        template<class TIterator>
        Byte chksum(TIterator begin, TIterator end) 
        {
            TIterator it = begin;
            Byte sum = 0;
            for (; it != end; ++it) {
                sum += *it;
            }
            sum = 128 - (sum & 0x7f);
            sum &= 0x7F;
            return sum;
        }
    }
}