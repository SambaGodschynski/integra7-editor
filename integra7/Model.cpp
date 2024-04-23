#include "Model.h"
#include <sstream>
#include <iostream>

namespace i7
{
    const Node* getNode(const char* id)
    {
        const Node* node = nullptr;
        std::string partId;
        std::stringstream ss(id); 
        while (std::getline(ss, partId, '-'))
        {

        }
        return nullptr;
    }
}