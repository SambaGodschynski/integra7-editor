package.path = package.path .. ';/home/samba/workspace/integra7-editor/lua/?.lua' -- TODO

require "_model"
require "_com"
require "_snaSection"
require "_sysex"
require "_patchesSection"
require "_partsSection"
require "_mfxSection"
require "_mfxPcmSSection"
require "_pcmsCommonSection"
require "_pcmsWaveSection"
require "_pcmsPmtSection"
require "_pcmsPitchSection"
require "_pcmsPitchEnvSection"
require "_pcmsTvfSection"
require "_pcmsTvfEnvSection"
require "_pcmsTvaSection"
require "_pcmsTvaEnvSection"
require "_pcmsOutputSection"
require "_pcmsLfoSection"
require "_pcmsCtrlSection"
require "_pcmsMtrxCtrlSection"
require "_pcmsTabsSection"
require "_snsCommonSection"
require "_snsMiscSection"
require "_snsOscSection"
require "_snsPitchSection"
require "_snsFilterSection"
require "_snsAmpSection"
require "_snsLfoSection"
require "_snsTabsSection"

Main = {}

CreateSnaSections(Main)
CreatePatchesSections(Main)
CreatePartsSections(Main)
CreateMfxSections(Main)
CreateMfxPcmSSections(Main)
CreatePcmsCommonSections(Main)
CreatePcmsWaveSections(Main)
CreatePcmsPmtSections(Main)
CreatePcmsPitchSections(Main)
CreatePcmsPitchEnvSections(Main)
CreatePcmsTvfSections(Main)
CreatePcmsTvfEnvSections(Main)
CreatePcmsTvaSections(Main)
CreatePcmsTvaEnvSections(Main)
CreatePcmsOutputSections(Main)
CreatePcmsLfoSections(Main)
CreatePcmsCtrlSections(Main)
CreatePcmsMtrxCtrlSections(Main)
CreatePcmsTabSections(Main)
CreateSnsCommonSections(Main)
CreateSnsMiscSections(Main)
CreateSnsOscSections(Main)
CreateSnsPitchSections(Main)
CreateSnsFilterSections(Main)
CreateSnsAmpSections(Main)
CreateSnsLfoSections(Main)
CreateSnsTabSections(Main)