#pragma once

// ext/integra7-editor-web/js/view/preset/category_name.js

#include "Integra7.h"

namespace i7
{
    constexpr CategoryId CATEG_NO = 0;
    constexpr CategoryId CATEG_APNO = 1;
    constexpr CategoryId CATEG_PPNO = 2;
    constexpr CategoryId CATEG_EGP = 3;
    constexpr CategoryId CATEG_EP1 = 4;
    constexpr CategoryId CATEG_EP2 = 5;
    constexpr CategoryId CATEG_EORG = 6;
    constexpr CategoryId CATEG_PORG = 7;
    constexpr CategoryId CATEG_RORG = 8;
    constexpr CategoryId CATEG_KEYH = 9;
    constexpr CategoryId CATEG_KEYCV = 10;
    constexpr CategoryId CATEG_KEYCL = 11;
    constexpr CategoryId CATEG_ACD = 12;
    constexpr CategoryId CATEG_HRM = 13;
    constexpr CategoryId CATEG_BEL = 14;
    constexpr CategoryId CATEG_MLT = 15;
    constexpr CategoryId CATEG_AGT = 16;
    constexpr CategoryId CATEG_EGT = 17;
    constexpr CategoryId CATEG_DGT = 18;
    constexpr CategoryId CATEG_ABS = 19;
    constexpr CategoryId CATEG_EBS = 20;
    constexpr CategoryId CATEG_SBS = 21;
    constexpr CategoryId CATEG_PLK = 22;
    constexpr CategoryId CATEG_SSTR = 23;
    constexpr CategoryId CATEG_ESTR = 24;
    constexpr CategoryId CATEG_ORC = 25;
    constexpr CategoryId CATEG_SBRS = 26;
    constexpr CategoryId CATEG_EBRS = 27;
    constexpr CategoryId CATEG_WND = 28;
    constexpr CategoryId CATEG_FLT = 29;
    constexpr CategoryId CATEG_SAX = 30;
    constexpr CategoryId CATEG_RECO = 31;
    constexpr CategoryId CATEG_VOXC = 32;
    constexpr CategoryId CATEG_VOXS = 33;
    constexpr CategoryId CATEG_SLD = 34;
    constexpr CategoryId CATEG_SBR = 35;
    constexpr CategoryId CATEG_SPD = 36;
    constexpr CategoryId CATEG_BPD = 37;
    constexpr CategoryId CATEG_SPK = 38;
    constexpr CategoryId CATEG_FX = 39;
    constexpr CategoryId CATEG_SSEQ = 40;
    constexpr CategoryId CATEG_PHRS = 41;
    constexpr CategoryId CATEG_PLS = 42;
    constexpr CategoryId CATEG_BTS = 43;
    constexpr CategoryId CATEG_HIT = 44;
    constexpr CategoryId CATEG_SFX = 45;
    constexpr CategoryId CATEG_DRM = 46;
    constexpr CategoryId CATEG_PRC = 47;
    constexpr CategoryId CATEG_STK = 48;
    constexpr CategoryId CATEG_ZONE = 49;
    constexpr CategoryId NUM_CATEGORIES = 50;
    struct CategoryName
    {
        const char *fullName;
        const char *shortName;
    };

    constexpr CategoryName CategoryNames[NUM_CATEGORIES] =
        {
            {"No Assign", "---"},             // 0
            {"Ac.Piano", "Ac.PNO"},           // 1
            {"Pop Piano", "PopPNO"},          // 2
            {"E.Grand Piano", "E.GND"},       // 3
            {"E.Piano1", "EP1"},              // 4
            {"E.Piano2", "EP2"},              // 5
            {"E.Organ", "E.ORG"},             // 6
            {"Pipe Organ", "P.ORG"},          // 7
            {"Reed Organ", "R.ORG"},          // 8
            {"Harpsichord", "HPCD"},          // 9
            {"Clav", "CLAV"},                 // 10
            {"Celesta", "CELE"},              // 11
            {"Accordion", "ACDN"},            // 12
            {"Harmonica", "HRM"},             // 13
            {"Bell", "BELL"},                 // 14
            {"Mallet", "MLT"},                // 15
            {"Ac.Guitar", "AGT"},             // 16
            {"E.Guitar", "EGT"},              // 17
            {"Dist.Guitar", "DGT"},           // 18
            {"Ac.Bass", "Ac.BS"},             // 19
            {"E.Bass", "E.BS"},               // 20
            {"Synth Bass", "Syn.BS"},         // 21
            {"Plucked/Stroke", "PLK/STK"},    // 22
            {"Solo Strings", "Solo.STR"},     // 23
            {"Ensemble Strings", "Ens.STR"},  // 24
            {"Orchestral", "ORC"},            // 25
            {"Solo Brass", "Solo.BRS"},       // 26
            {"Ensemble Brass", "Ens.BRS"},    // 27
            {"Wind", "WND"},                  // 28
            {"Flute", "FLT"},                 // 29
            {"Sax", "SAX"},                   // 30
            {"Recorder", "RCDR"},             // 31
            {"Vox/Choir", "VOX"},             // 32
            {"Scat", "SCAT"},                 // 33
            {"Synth Lead", "Syn.LD"},         // 34
            {"Synth Brass", "Syn.BRS"},       // 35
            {"Synth Pad/Strings", "Syn.PAD"}, // 36
            {"Synth Bellpad", "BELLPAD"},     // 37
            {"Synth PolyKey", "POLYKEY"},     // 38
            {"Synth FX", "Syn.FX"},           // 39
            {"Synth Seq/Pop", "Syn.SEQ"},     // 40
            {"Phrase", "PHR"},                // 41
            {"Pulsating", "PLS"},             // 42
            {"Beat&Groove", "BTS"},           // 43
            {"Hit", "HIT"},                   // 44
            {"Sound FX", "SFX"},              // 45
            {"Drums", "DRM"},                 // 46
            {"Percussion", "PRC"},            // 47
            {"Stack", "Stack"},               // 48
            {"Zone", "Zone"}                  // 49
    };
}