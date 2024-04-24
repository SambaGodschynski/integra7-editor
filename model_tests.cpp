#include <iostream>
#include <integra7/Integra7.h>
#include <cassert>

int main(size_t, const char**)
{
    auto nodeResult = i7::getNode("PRM-_FPART1");
    assert(nodeResult.node->addr == i7::nibble(0x19000000));
    assert(nodeResult.node->offset == 1483);
    assert(nodeResult.index == 4);
    assert(std::string(nodeResult.node->desc) == std::string("Temporary Tone (Studio Mode Part 1)"));

    nodeResult = i7::getNode("PRM-_FPART1-_SNTONE-_SNTC-SNTC_MOD_PRM14");
    assert(nodeResult.node->addr == i7::nibble(0x002F));
    assert(nodeResult.node->offset == 0);
    assert(nodeResult.index == 31);
    assert(std::string(nodeResult.node->desc) == std::string("Modify Parameter 14"));

    nodeResult = i7::getNode("PRM-_FPART1-_SNTONE-_SNTC");
    assert(nodeResult.addr == 52461568);
    assert(nodeResult.offset == 2494);

    nodeResult = i7::getNode("PRM-_SETUP-_STP-_RC2-NESTP_SND_MODE_SD1");
    assert(nodeResult.node != nullptr);
    assert(std::string(nodeResult.node->desc) == std::string("Side 1 Sound Mode"));
    assert(i7::data[0] == 0);
    i7::put(nodeResult, 4);
    assert(i7::data[0] == 4);

    nodeResult = i7::getNode("PRM-_FPART16-_RHY-_RC2-RFRC2_TFX_SW");
    assert(nodeResult.node != nullptr);
    assert(std::string(nodeResult.node->desc) == std::string("TFX Switch"));
    assert(i7::data[i7::NumDataValues - 1] == 0);
    i7::put(nodeResult, 1);
    assert(i7::data[i7::NumDataValues - 1] == 1);

    return 0;
}