#include <iostream>
#include <integra7/Integra7.h>
#include <cassert>

int main(size_t, const char**)
{
    auto nodeResult = i7::getNode("PRM-_FPART1");
    assert(nodeResult.node->addr == i7::nibble(0x19000000));
    assert(nodeResult.node->offset == 1483);
    assert(std::string(nodeResult.node->desc) == std::string("Temporary Tone (Studio Mode Part 1)"));

    nodeResult = i7::getNode("PRM-_FPART1-_SNTONE-_SNTC-SNTC_MOD_PRM14");
    assert(nodeResult.node->addr == i7::nibble(0x002F));
    assert(nodeResult.node->offset == 0);
    assert(std::string(nodeResult.node->desc) == std::string("Modify Parameter 14"));

    nodeResult = i7::getNode("PRM-_FPART1-_SNTONE-_SNTC");
    assert(nodeResult.addr == 52461568);
    assert(nodeResult.offset == 2494);
    return 0;
}