#pragma once

#include "AppTypes.h"
#include <vector>
#include <string>

void sendMessage(I7Ed& ed, const Bytes& message);
void valueChanged(I7Ed& ed, const ParameterDef& paramDef);
void valueChanged(I7Ed& ed, const ValueChangedMessage& vcMessage);
ParameterDef* getParameterDef(I7Ed& ed, const std::string& id);

std::vector<RequestMessage> buildParamRequests(
    I7Ed& ed, const std::vector<ParameterDef*>& params);
SectionDef::FGetReceiveSysex makeParamOnlyGetter(I7Ed& ed, const SectionDef& sec);

void enqueueRequest(I7Ed& ed, const RequestMessage& req);
void triggerReceive(I7Ed& ed, const std::vector<SectionDef::FGetReceiveSysex>& getters);

std::vector<std::string> getTonePrefixes(const std::string& partPrefix, int msb);
int partPrefixToNumber(const std::string& partPrefix);

void saveSysexToFile(I7Ed& ed);
void loadSysexFromFile(I7Ed& ed);

void processPendingReceives(I7Ed& ed, SectionDef::NamedSections& sections);
