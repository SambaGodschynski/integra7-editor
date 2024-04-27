#include "Model.h"
#include <sstream>
#include <iostream>

namespace i7
{
    namespace
    {
        void valueToBytes(UInt, ValueByteSizeType, Byte* outBytes);
        void valueToBytes(const std::string&, ValueByteSizeType, Byte* outBytes);
        UInt getByteSize(ValueByteSizeType);
        constexpr UInt ADDR_SIZE = 4;
        constexpr UInt DATA_SIZE = 4;
        constexpr UInt SIZE_F7 = 1;
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
    Byte data[NumDataValues] = {0};
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
            UInt localAddr = 0;
            for (UInt i = 0; i < numNodes; ++i)
            {
                const Node* n = &nodes[i];
                if (partId == std::string(n->id))
                {
                    result.addr += n->addr;
                    result.offset += n->offset;
                    node = n;
                    if (ss.eof()) 
                    {
                        result.offset += localAddr;
                        result.node = node;
                        return result;
                    }
                    nodes = &node->node[0];
                    numNodes = node->numChildren;
                    break;
                }
                localAddr += getByteSize(n->valueByteSizeType);
            }
            if (nodes == &root[0]) 
            {
                // found nothing
                return result;
            }
        }
        return result;
    }
    void put(const NodeInfo& nodeData, UInt v)
    {
        if ((UInt)nodeData.node->valueByteSizeType > (UInt)INTEGER_MASK) {
            v = std::max(nodeData.node->min, std::min(nodeData.node->max, v));
        }
        if ((nodeData.offset + getByteSize(nodeData.node->valueByteSizeType)) > NumDataValues)
        {
            throw std::runtime_error("index overflow");
        }
        valueToBytes(v, nodeData.node->valueByteSizeType, &data[nodeData.offset]);
    }
    void put(const NodeInfo& nodeData, const std::string &str)
    {
        if ((nodeData.offset + getByteSize(nodeData.node->valueByteSizeType)) > NumDataValues)
        {
            throw std::runtime_error("index overflow");
        }
        valueToBytes(str, nodeData.node->valueByteSizeType, &data[nodeData.offset]);
    }

    UInt get(const NodeInfo& nodeInfo)
    {
        static_assert(sizeof(UInt) == 4);
        if (getByteSize(nodeInfo.node->valueByteSizeType) > 4) 
        {
            throw std::runtime_error("unexpected byte size");
        }
        UInt* pUint = (UInt*)(&data[nodeInfo.offset]);
        return *pUint;
    }
    
    std::string getString(const NodeInfo& nodeInfo)
    {
        UInt byteSize = getByteSize(nodeInfo.node->valueByteSizeType);
        char result[Integra7MaxValueBytes + 1] = {0};
        Byte* begin = &data[nodeInfo.offset];
        Byte* end = begin + byteSize - 1;
        for(; end >= begin; --end)
        {
            UInt i = end - begin;
            char c = *(end);
            if (c == 0x20 || c == 0)
            {
                continue;
            }
            result[i] = c;
        }
        return std::string(&result[0]);
    }

    Bytes createSysexData(const NodeInfo& nodeData)
    {
        UInt addr = nodeData.addr;
        UInt resultSize = getByteSize(nodeData.node->valueByteSizeType);
        Bytes result;
        result.reserve(SizeRolandDt1 + ADDR_SIZE + resultSize + SIZE_F7);
        result.insert(result.begin(), &ROLAND_DT1[0], &ROLAND_DT1[SizeRolandDt1]);
        result.push_back((addr >> 12) & 0xf);
        result.push_back((addr >> 8) & 0xf);
        result.push_back((addr >> 4) & 0xf);
        result.push_back((addr & 0xf));
        Byte* it = &data[nodeData.offset];
        Byte* end = it + resultSize;
        for (; it != end; ++it)
        {
            result.push_back(*it);
        }
        result.push_back(0xf7);
        return result;
    }

    Bytes createSysexData(const NodeInfo& nodeData, Byte deviceId)
    {
        Bytes result = createSysexData(nodeData);
        result[DeviceIdOffset] = deviceId;
        return result;
    }


    //-------------------------------------------------------------------------
    namespace
    {
        UInt getByteSize(ValueByteSizeType byteSizeType)
        {
            switch (byteSizeType)
            {
            case ZeroByteSize: return 0;
            case INTEGER1x1:
            case INTEGER1x2:
            case INTEGER1x3:
            case INTEGER1x4:
            case INTEGER1x5:
            case INTEGER1x6:
            case INTEGER1x7: return 1;
            case INTEGER2x4: return 2;
            case INTEGER4x4: return 4;
            case ByteSize12: return 12;
            case ByteSize16: return 16;
            default:
                throw std::runtime_error("unexpected byte size type");
            }
        }
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
                outBytes[1] = ((v >> 4) & 0xf);
                outBytes[0] = (v & 0xf);
                break;

            case INTEGER4x4:
                outBytes[3] = ((v >> 12) & 0xf);
                outBytes[2] = ((v >> 8) & 0xf);
                outBytes[1] = ((v >> 4) & 0xf);
                outBytes[0] = (v & 0xf);
                break;

            default: /* ASCII String */
                throw std::runtime_error("unexpected byte size type");
            }
        }
        void valueToBytes(const std::string& str, ValueByteSizeType valueByteSizeType, Byte* outBytes)
        {
            Byte* it = outBytes;
            Byte* end = outBytes + getByteSize(valueByteSizeType);
            UInt str_length = str.length();
            UInt i = 0;
            for (; it != end; ++it) 
            {
                char c = i < str_length ? str.at(i) : 0x20;
                *it = c;
                i++;
            }
        }
    }
}