#include "Model.h"
#include <sstream>
#include <iostream>

namespace i7
{
    UInt data[NumDataValues] = {0};
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
                    result.addr += n->addr;
                    result.offset += n->offset;
                    result.index = i;
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
    void put(const NodeInfo& nodeData, UInt v)
    {
        if (nodeData.node->bytes > INTEGER_MASK) {
            v = std::max(nodeData.node->min, std::min(nodeData.node->max, v));
        }
        /*
            } else if (v.length > o.leaf.bytes) { /// v is ASCII String
                v = v.substring(0, o.leaf.bytes);
            }
        */
        data[nodeData.offset + nodeData.index] = v;
    }
}