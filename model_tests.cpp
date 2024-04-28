#include <iostream>
#include <integra7/Integra7.h>
#include <cassert>
#include "Helper.h"

int main(size_t, const char**)
{
    auto nodeResult = i7::getNode("PRM-_FPART1");
    assert(nodeResult.node->addr == i7::nibble(0x19000000));
    assert(std::string(nodeResult.node->desc) == std::string("Temporary Tone (Studio Mode Part 1)"));

    nodeResult = i7::getNode("PRM-_FPART1-_SNTONE-_SNTC-SNTC_MOD_PRM14");
    assert(nodeResult.node->addr == i7::nibble(0x002F));
    assert(std::string(nodeResult.node->desc) == std::string("Modify Parameter 14"));

    nodeResult = i7::getNode("PRM-_SETUP-_STP-_RC2-NESTP_SND_MODE_SD1");
    assert(nodeResult.node != nullptr);
    assert(std::string(nodeResult.node->desc) == std::string("Side 1 Sound Mode"));
    assert(i7::data[0] == 0);
    assert(i7::get(nodeResult) == 0);
    i7::put(nodeResult, 4);
    assert(i7::data[0] == 4);
    assert(i7::get(nodeResult) == 4);

    nodeResult = i7::getNode("PRM-_FPART16-_RHY-_RC2-RFRC2_TFX_SW");
    assert(nodeResult.node != nullptr);
    assert(std::string(nodeResult.node->desc) == std::string("TFX Switch"));
    assert(i7::data[i7::NumDataValues - 1] == 0);
    assert(i7::get(nodeResult) == 0);
    i7::put(nodeResult, 1);
    assert(i7::data[i7::NumDataValues - 1] == 1);
    assert(i7::get(nodeResult) == 1);

    nodeResult = i7::getNode("PRM-_FPART1-_PAT-_PC-RFPC_NAME");
    assert(nodeResult.node != nullptr);
    assert(std::string(nodeResult.node->desc) == std::string("PCM Tone Name"));
    assert(i7::getString(nodeResult) == std::string(""));
    std::string str("012345");
    assert(str.length() == 6);
    i7::put(nodeResult, str);
    assert(i7::getString(nodeResult) == std::string("012345"));


    nodeResult = i7::getNode("PRM-_PRF-_FC-NEFC_NAME");
    assert(nodeResult.node != nullptr);
    assert(std::string(nodeResult.node->desc) == std::string("Studio Set Name"));
    assert(i7::getString(nodeResult) == std::string(""));
    str = std::string("0123456789012345");
    assert(str.length() == 16);
    i7::put(nodeResult, str);
    assert(i7::getString(nodeResult) == std::string("0123456789012345"));
    
    i7::put(nodeResult, "012345678901234501234567890123450123456789012345012345678901234501234567890123450123456789012345");
    assert(i7::getString(nodeResult) == std::string("0123456789012345"));
    
    // check bounds
    i7::NodeInfo neighbour = i7::getNode("PRM-_PRF-_FC-NEFC_MFX1_CTRL_CH");
    assert(neighbour.node != nullptr);
    assert(std::string(neighbour.node->desc) == std::string("MFX1 Control Channel"));
    assert(i7::get(neighbour) == 0);
    i7::put(neighbour, 15);
    i7::put(nodeResult, "012345678901234501234567890123450123456789012345012345678901234501234567890123450123456789012345");
    assert(i7::get(neighbour) == 15);
    assert(i7::getString(nodeResult) == std::string("0123456789012345"));

    nodeResult = i7::getNode("PRM-_FPART1-_SNTONE-_SNTC-SNTC_TONE_LEVEL");
    assert(nodeResult.node != nullptr);
    assert(std::string(nodeResult.node->desc) == std::string("Tone Level"));
    i7::put(nodeResult, 127);
    assert(i7::get(nodeResult) == 127);
    i7::Bytes sysex = i7::createSysexData(nodeResult);
    std::string strSysex = bytesToString(sysex.cbegin(), sysex.cend());
    //                              f0411000006412190200107f  f7
    assert(strSysex == std::string("f0411000006412190200107f56f7"));

    // TODO CHECK INTEGER4x4, string value and value with lowest/highest address

    return 0;

}