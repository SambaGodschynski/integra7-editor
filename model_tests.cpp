#include <iostream>
#include <integra7/Integra7.h>
#include <cassert>
#include "Helper.h"



int main(int, const char**)
{
    i7::ModelData model;
    {
        auto nodeResult = i7::getNode("PRM-_FPART1");
        assert(nodeResult.node->addr == i7::nibble(0x19000000));
        assert(std::string(nodeResult.node->desc) == std::string("Temporary Tone (Studio Mode Part 1)"));
    }
    {
        auto nodeResult = i7::getNode("PRM-_FPART1-_SNTONE-_SNTC-SNTC_MOD_PRM14");
        assert(nodeResult.node->addr == i7::nibble(0x002F));
        assert(std::string(nodeResult.node->desc) == std::string("Modify Parameter 14"));
    }
    {
        auto nodeResult = i7::getNode("PRM-_SETUP-_STP-_RC2-NESTP_SND_MODE_SD1");
        assert(nodeResult.node != nullptr);
        assert(std::string(nodeResult.node->desc) == std::string("Side 1 Sound Mode"));
        assert(i7::get(&model, nodeResult) == 0);
        i7::put(&model, nodeResult, 4);
        assert(i7::get(&model, nodeResult) == 4);
    }
    {
        auto nodeResult = i7::getNode("PRM-_FPART16-_RHY-_RC2-RFRC2_TFX_SW");
        assert(nodeResult.node != nullptr);
        assert(std::string(nodeResult.node->desc) == std::string("TFX Switch"));
        assert(i7::get(&model, nodeResult) == 0);
        i7::put(&model, nodeResult, 1);
        assert(i7::get(&model, nodeResult) == 1);
    }
    {
        auto nodeResult = i7::getNode("PRM-_FPART1-_PAT-_PC-RFPC_NAME");
        assert(nodeResult.node != nullptr);
        assert(std::string(nodeResult.node->desc) == std::string("PCM Tone Name"));
        assert(i7::getString(&model, nodeResult) == std::string(""));
        std::string str("012345");
        assert(str.length() == 6);
        i7::put(&model, nodeResult, str);
        assert(i7::getString(&model, nodeResult) == std::string("012345"));
    }
    {
        auto nodeResult = i7::getNode("PRM-_PRF-_FC-NEFC_NAME");
        assert(nodeResult.node != nullptr);
        assert(std::string(nodeResult.node->desc) == std::string("Studio Set Name"));
        assert(i7::getString(&model, nodeResult) == std::string(""));
        auto str = std::string("0123456789012345");
        assert(str.length() == 16);
        i7::put(&model, nodeResult, str);
        assert(i7::getString(&model, nodeResult) == std::string("0123456789012345"));
        i7::put(&model, nodeResult, "012345678901234501234567890123450123456789012345012345678901234501234567890123450123456789012345");
        assert(i7::getString(&model, nodeResult) == std::string("0123456789012345"));
        // check bounds
        i7::NodeInfo neighbour = i7::getNode("PRM-_PRF-_FC-NEFC_MFX1_CTRL_CH");
        assert(neighbour.node != nullptr);
        assert(std::string(neighbour.node->desc) == std::string("MFX1 Control Channel"));
        assert(i7::get(&model, neighbour) == 0);
        i7::put(&model, neighbour, 15);
        i7::put(&model, nodeResult, "012345678901234501234567890123450123456789012345012345678901234501234567890123450123456789012345");
        assert(i7::get(&model, neighbour) == 15);
        assert(i7::getString(&model, nodeResult) == std::string("0123456789012345"));
    }
    {
        auto nodeResult = i7::getNode("PRM-_FPART1-_SNTONE-_SNTC-SNTC_TONE_LEVEL");
        assert(nodeResult.node != nullptr);
        assert(std::string(nodeResult.node->desc) == std::string("Tone Level"));
        i7::put(&model, nodeResult, 127);
        assert(i7::get(&model, nodeResult) == 127);
        i7::Bytes sysex = i7::createSysexData(&model, nodeResult);
        std::string strSysex = bytesToString(sysex.cbegin(), sysex.cend());
        assert(strSysex == std::string("f0411000006412190200107f56f7"));

        i7::put(&model, nodeResult, 121);
        assert(i7::get(&model, nodeResult) == 121);
        sysex = i7::createSysexData(&model, nodeResult, 23);
        strSysex = bytesToString(sysex.cbegin(), sysex.cend());
        assert(strSysex == std::string("f041170000641219020010795cf7"));
    }
    {
        auto nodeResult = i7::getNode("PRM-_SYS-_SC-NESC_TUNE");
        assert(nodeResult.node != nullptr);
        assert(std::string(nodeResult.node->desc) == std::string("Master Tune"));
        i7::put(&model, nodeResult, 2024);
        assert(i7::get(&model, nodeResult) == 2024);
        auto sysex = i7::createSysexData(&model, nodeResult);
        auto strSysex = bytesToString(sysex.cbegin(), sysex.cend());
        assert(strSysex == std::string("f04110000064120200000000070e0861f7"));
    }
    {
        auto nodeResult = i7::getNode("PRM-_SYS-_SC-NESC_TEMPO");
        assert(nodeResult.node != nullptr);
        assert(std::string(nodeResult.node->desc) == std::string("System Tempo"));
        i7::put(&model, nodeResult, 250);
        assert(i7::get(&model, nodeResult) == 250);
        auto sysex = i7::createSysexData(&model, nodeResult);
        auto strSysex = bytesToString(sysex.cbegin(), sysex.cend());
        assert(strSysex == std::string("f0411000006412020000260f0a3ff7"));
    }
    {
        auto nodeResult = i7::getNode("PRM-_FPART1-_PAT-_PC-RFPC_NAME");
        i7::put(&model, nodeResult, "012345");
        assert(i7::getString(&model, nodeResult) == std::string("012345"));
        auto sysex = i7::createSysexData(&model, nodeResult);
        auto strSysex = bytesToString(sysex.cbegin(), sysex.cend());
        assert(strSysex == std::string("f04110000064121900000030313233343520202020202078f7"));
    }
    {
        i7::RequestInfo sendInfo;
        sendInfo.addr = i7::REQ_READ_EXP_ADDR;
        auto sysex = i7::createRq1SysexData(sendInfo);
        auto strSysex = bytesToString(sysex.cbegin(), sysex.cend());
        assert(strSysex == std::string("f04110000064110f0000100000000061f7"));
    }
    {
        i7::RequestInfo sendInfo;
        sendInfo.addr = i7::REQ_READ_EXP_ADDR;
        auto sysex = i7::createRq1SysexData(sendInfo);
        auto response = i7::getResponseData(sysex.data(), sysex.size());
        assert(response.deviceId == 0x10);
        assert(response.addr == i7::REQ_READ_EXP_ADDR);
        assert(response.modelId == 0x64);
        assert(response.requestType == 0x11);
        auto payloadString = bytesToString(response.payload, response.payload + response.numBytes);
        assert(payloadString == std::string("0000000061"));
    }
    {
        auto leafNodes = i7::getLeafNodes("");
        assert(leafNodes.empty());
    }
    {

        auto leafNodes = i7::getLeafNodes("PRM-_FPART1-_SNTONE");
        assert(leafNodes.size() == 98);
        auto someNode = i7::getNode("PRM-_FPART1-_SNTONE-_SNTC-SNTC_NAME");
        assert(leafNodes[0].addr == someNode.addr);
        assert(leafNodes[0].node == someNode.node);

        auto anotherNode = i7::getNode("PRM-_FPART1-_SNTONE-_SNTF-SNTF_MFX_PRM32");
        assert(leafNodes[97].addr == anotherNode.addr);
        assert(leafNodes[97].node == anotherNode.node);

     }
    return 0;

}