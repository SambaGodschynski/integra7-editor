#pragma once

#include "Integra7Defs.h"

namespace i7 
{
    constexpr ModDef SnaModTw[] = 
    {
        { "Harmonic Bar 16\"", "MOD_TW_HBAR_16", 8, 0, 8, 0 }, 
		{ "Harmonic Bar 5-1/3\"", "MOD_TW_HBAR_5_13", 8, 0, 8, 0 }, 
		{ "Harmonic Bar 8\"", "MOD_TW_HBAR_8", 8, 0, 8, 0 }, 
		{ "Harmonic Bar 4\"", "MOD_TW_HBAR_4", 0, 0, 8, 0 }, 
		{ "Harmonic Bar 2-2/3\"", "MOD_TW_HBAR_2_23", 0, 0, 8, 0 }, 
		{ "Harmonic Bar 2\"", "MOD_TW_HBAR_2", 0, 0, 8, 0 }, 
		{ "Harmonic Bar 1-3/5\"", "MOD_TW_HBAR_1_35", 0, 0, 8, 0 }, 
		{ "Harmonic Bar 1-1/3\"", "MOD_TW_HBAR_1_13", 0, 0, 8, 0 }, 
		{ "Harmonic Bar 1\"", "MOD_TW_HBAR_1", 0, 0, 8, 0 }, 
		{ "Percussion Switch", "MOD_TW_PERC_SW", 1, 0, 1, 0 }, 
		{ "Percussion Harmonic", "MOD_TW_PERC_HARM", 1, 0, 1, 0 }, 
		{ "Percussion Slow", "MOD_TW_PERC_SLOW", 0, 0, 1, 0 }, 
		{ "Key On Click Level", "MOD_TW_ON_CLICK", 5, 0, 31, 0 }, 
		{ "Key Off Click Level", "MOD_TW_OFF_CLICK", 5, 0, 31, 0 }, 
		{ "Percussion Soft Level", "MOD_TW_PERC_SOFT_LEVEL", 3, 0, 15, 0 }, 
		{ "Percussion Normal Level", "MOD_TW_PERC_NORM_LEVEL", 10, 0, 15, 0 }, 
		{ "Percussion Slow Time", "MOD_TW_PERC_SLOW_TIME", 104, 0, 127, 0 }, 
		{ "Percussion Fast Time", "MOD_TW_PERC_FAST_TIME", 43, 0, 127, 0 }, 
		{ "Percussion Recharge Time", "MOD_TW_PERC_RECHARGE", 5, 0, 10, 0 }, 
		{ "Percussion Harmonic Bar Level", "MOD_TW_PERC_HBAR_LEVEL", 64, 0, 127, 0 }, 
		{ "Percussion Soft", "MOD_TW_PERC_SOFT", 0, 0, 1, 0 }, 
		{ "Leakage Level", "MOD_TW_LEAKAGE", 20, 0, 127, 0 }, 
    };

    constexpr ModDef SnaModSnap[] = 
    {
        { "String Resonance", "SnaModSnap_STRING_RESO_LEVEL", 64, 0, 127, 0 }, 
		{ "Key Off Resonance", "SnaModSnap_KEY_OFF_RESO_LEVEL", 64, 0, 127, 0 }, 
		{ "Hammer Noise", "SnaModSnap_THUMP_LEVEL", 64, 62, 66, 0 }, 
		{ "Stereo Width", "SnaModSnap_STEREO_WIDTH", 63, 0, 63, 0 }, 
		{ "Nuance", "SnaModSnap_NUANCE", 0, 0, 2, 0 }, 
		{ "Tone Character", "SnaModSnap_SPLIT_SHIFT", 64, 59, 69, 0 }, 
    };

    constexpr ModDef SnModSnStd[] = 
    {
 		{ "Noise Level (CC#16)", "MOD_SNSTD_NOISE_LEVEL", 64, 0, 127, 16 }, 
		{ "Rate (CC#17)", "MOD_SNSTD_RATE", 64, 0, 127, 17 }, 
		{ "Growl Sens (CC#18)", "MOD_SNSTD_GROWL_SENS", 0, 0, 127, 18 }, 
		{ "Mode (CC#19)", "MOD_SNSTD_MODE", 0, 0, 1, 19 }, 
		{ "Drone Level", "MOD_SNSTD_DRONE_LEVEL", 64, 0, 127, 0 }, 
		{ "Drone Pitch", "MOD_SNSTD_DRONE_PITCH", 64, 52, 76, 0 }, 
		{ "Play Scale", "MOD_SNSTD_PLAY_SCALE", 0, 0, 0, 0 }, 
		{ "Scale Key", "MOD_SNSTD_SCALE_KEY", 0, 0, 11, 0 }, 
		{ "Glide", "MOD_SNSTD_GLIDE", 0, 0, 1, 0 }, 
		{ "Variation", "MOD_SNSTD_VARIATION", 0, 0, 0, 0 }, 
		{ "Picking Harmonics", "MOD_SNSTD_PICKHARM", 0, 0, 1, 0 }, 
		{ "Buzz Key Switch", "MOD_SNSTD_KEYSW", 0, 0, 1, 0 }, 
		{ "Sub String Tune", "MOD_SNSTD_OFFSET1", 64, 0, 127, 0 }, 
    };

    constexpr ModDef SnModVariMarimba = { "Variation", "VARI_MARIMBA", 0, 0, 1, 0 };
	constexpr ModDef SnModVariVibraphone = { "Variation", "VARI_VIBRAPHONE", 0, 0, 2, 0 };
	constexpr ModDef SnModVariSteelStrGuitar = { "Variation", "VARI_STEELSTRGUITAR", 0, 0, 2, 0 };
	constexpr ModDef SnModVariFlamencoGuitar = { "Variation", "VARI_FLAMENCOGUITAR", 0, 0, 2, 0 };
	constexpr ModDef SnModVariJazzGuitar = { "Variation", "VARI_JAZZGUITAR", 0, 0, 2, 0 };
	constexpr ModDef SnModVariAcousticBass = { "Variation", "VARI_ACOUSTICBASS", 0, 0, 2, 0 };
	constexpr ModDef SnModVariFingeredBass = { "Variation", "VARI_FINGEREDBASS", 0, 0, 2, 0 };
	constexpr ModDef SnModVariPickedBass = { "Variation", "VARI_PICKEDBASS", 0, 0, 2, 0 };
	constexpr ModDef SnModVariViolin = { "Variation", "VARI_VIOLIN", 0, 0, 3, 0 };
	constexpr ModDef SnModVariHarp = { "Variation", "VARI_HARP", 0, 0, 1, 0 };
	constexpr ModDef SnModVariTimpani = { "Variation", "VARI_TIMPANI", 0, 0, 2, 0 };
	constexpr ModDef SnModVariStrings = { "Variation", "VARI_STRINGS", 0, 0, 3, 0 };
	constexpr ModDef SnModVariChoir = { "Variation", "VARI_CHOIR", 0, 0, 1, 0 };
	constexpr ModDef SnModVariTrumpet = { "Variation", "VARI_TRUMPET", 0, 0, 2, 0 };
	constexpr ModDef SnModVariFrenchHorn = { "Variation", "VARI_FRENCHHORN", 0, 0, 1, 0 };
	constexpr ModDef SnModVariSax = { "Variation", "VARI_SAX", 0, 0, 3, 0 };
	constexpr ModDef SnModVariPanFlute = { "Variation", "VARI_PANFLUTE", 0, 0, 2, 0 };
	constexpr ModDef SnModVariShakuhachi = { "Variation", "VARI_SHAKUHACHI", 0, 0, 2, 0 };
	constexpr ModDef SnModVariPipes = { "Variation", "VARI_PIPES", 0, 0, 2, 0 };
	constexpr ModDef SnModVariSteelDrums = { "Variation", "VARI_STEELDRUMS", 0, 0, 1, 0 };
	constexpr ModDef SnModVariSantoor = { "Variation", "VARI_SANTOOR", 0, 0, 2, 0 };
	constexpr ModDef SnModVariYangChin = { "Variation", "VARI_YANGCHIN", 0, 0, 2, 0 };
	constexpr ModDef SnModVariTinWhistle = { "Variation", "VARI_TINWHISTLE", 0, 0, 2, 0 };
	constexpr ModDef SnModVariShamisen = { "Variation", "VARI_SHAMISEN", 0, 0, 3, 0 };
	constexpr ModDef SnModVariKoto = { "Variation", "VARI_KOTO", 0, 0, 2, 0 };
	constexpr ModDef SnModVariKalimba = { "Variation", "VARI_KALIMBA", 0, 0, 1, 0 };
	constexpr ModDef SnModVariMandolin = { "Variation", "VARI_MANDOLIN", 0, 0, 2, 0 };
	constexpr ModDef SnModVariTuba = { "Variation", "VARI_TUBA", 0, 0, 1, 0 };
	constexpr ModDef SnModScaleHarp = { "Play Scale", "SCALE_HARP", 0, 0, 6, 0 };
	constexpr ModDef SnModScaleSax = { "Play Scale", "SCALE_SAX", 0, 0, 5, 0 };
	constexpr ModDef SnModScaleKoto = { "Play Scale", "SCALE_KOTO", 0, 0, 1, 0 };

    constexpr UInt ModMono = 0;
    constexpr UInt ModPoly = 1;

    constexpr UInt NumSnModInstrumentTable = 127;
    constexpr ModInstrumentTable SnModInstrumentTable[NumSnModInstrumentTable] =
    {
        { 64, 0, ModPoly, &SnaModSnap[0], sizeof(SnaModSnap)/sizeof(SnaModSnap[0]), nullptr, nullptr, "INT: Concert Grand" },
		{ 64, 1, ModPoly, &SnaModSnap[0], sizeof(SnaModSnap)/sizeof(SnaModSnap[0]), nullptr, nullptr, "INT: Grand Piano1" },
		{ 64, 2, ModPoly, &SnaModSnap[0], sizeof(SnaModSnap)/sizeof(SnaModSnap[0]), nullptr, nullptr, "INT: Grand Piano2" },
		{ 64, 3, ModPoly, &SnaModSnap[0], sizeof(SnaModSnap)/sizeof(SnaModSnap[0]), nullptr, nullptr, "INT: Grand Piano3" },
		{ 64, 4, ModPoly, &SnaModSnap[0], sizeof(SnaModSnap)/sizeof(SnaModSnap[0]), nullptr, nullptr, "INT: Mellow Piano" },
		{ 64, 5, ModPoly, &SnaModSnap[0], sizeof(SnaModSnap)/sizeof(SnaModSnap[0]), nullptr, nullptr, "INT: Bright Piano" },
		{ 64, 6, ModPoly, &SnaModSnap[0], sizeof(SnaModSnap)/sizeof(SnaModSnap[0]), nullptr, nullptr, "INT: Upright Piano" },
		{ 64, 7, ModPoly, &SnaModSnap[0], sizeof(SnaModSnap)/sizeof(SnaModSnap[0]), nullptr, nullptr, "INT: Concert Mono" },
		{ 64, 8, ModPoly, &SnaModSnap[0], sizeof(SnaModSnap)/sizeof(SnaModSnap[0]), nullptr, nullptr, "INT: Honky-tonk" },
		{ 0, 4, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Pure Vintage EP1" },
		{ 1, 4, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Pure Vintage EP2" },
		{ 2, 4, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Pure Wurly" },
		{ 3, 4, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Pure Vintage EP3" },
		{ 6, 4, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Old Hammer EP" },
		{ 7, 4, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Dyno Piano" },
		{ 0, 7, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Clav CB Flat" },
		{ 1, 7, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Clav CA Flat" },
		{ 2, 7, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Clav CB Medium" },
		{ 3, 7, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Clav CA Medium" },
		{ 4, 7, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Clav CB Brillia" },
		{ 5, 7, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Clav CA Brillia" },
		{ 6, 7, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Clav CB Combo" },
		{ 7, 7, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Clav CA Combo" },
		{ 0, 9, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariMarimba, "INT: Glockenspiel" },
		{ 0, 11, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariVibraphone, "INT: Vibraphone" },
		{ 0, 12, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariMarimba, "INT: Marimba" },
		{ 0, 13, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariMarimba, "INT: Xylophone" },
		{ 0, 14, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariMarimba, "INT: Tubular Bells" },
		{ 65, 0, ModPoly, &SnaModTw[0], sizeof(SnaModTw)/sizeof(SnaModTw[0]), nullptr,  nullptr, "INT: TW Organ" },
		{ 0, 21, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: French Accordion" },
		{ 1, 21, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: ItalianAccordion" },
		{ 0, 22, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Harmonica" },
		{ 0, 23, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Bandoneon" },
		{ 0, 24, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "INT: Nylon Guitar" },
		{ 1, 24, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariFlamencoGuitar, "INT: Flamenco Guitar" },
		{ 0, 25, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "INT: SteelStr Guitar" },
		{ 0, 26, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariJazzGuitar, "INT: Jazz Guitar" },
		{ 0, 27, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "INT: ST Guitar Half" },
		{ 1, 27, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "INT: ST Guitar Front" },
		{ 2, 27, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "INT: TC Guitar Rear" },
		{ 0, 32, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariAcousticBass, "INT: Acoustic Bass" },
		{ 0, 33, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariFingeredBass, "INT: Fingered Bass" },
		{ 0, 34, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariPickedBass, "INT: Picked Bass" },
		{ 0, 35, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariAcousticBass, "INT: Fretless Bass" },
		{ 0, 40, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariViolin, "INT: Violin" },
		{ 1, 40, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariViolin, "INT: Violin 2" },
		{ 0, 41, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariViolin, "INT: Viola" },
		{ 0, 42, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariViolin, "INT: Cello" },
		{ 1, 42, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariViolin, "INT: Cello 2" },
		{ 0, 43, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariViolin, "INT: Contrabass" },
		{ 0, 46, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleHarp,  &SnModVariHarp, "INT: Harp" },
		{ 0, 47, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTimpani, "INT: Timpani" },
		{ 0, 48, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariStrings, "INT: Strings" },
		{ 1, 48, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariStrings, "INT: Marcato Strings" },
		{ 0, 52, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariChoir, "INT: London Choir" },
		{ 1, 52, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariChoir, "INT: Boys Choir" },
		{ 0, 56, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "INT: Trumpet" },
		{ 0, 57, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "INT: Trombone" },
		{ 3, 57, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "INT: Tb2 CupMute" },
		{ 0, 59, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "INT: Mute Trumpet" },
		{ 0, 60, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariFrenchHorn, "INT: French Horn" },
		{ 1, 64, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariSax, "INT: Soprano Sax 2" },
		{ 1, 65, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariSax, "INT: Alto Sax 2" },
		{ 2, 66, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariSax, "INT: Tenor Sax 2" },
		{ 1, 67, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariSax, "INT: Baritone Sax 2" },
		{ 0, 68, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariFrenchHorn, "INT: Oboe" },
		{ 0, 70, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariFrenchHorn, "INT: Bassoon" },
		{ 0, 71, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariFrenchHorn, "INT: Clarinet" },
		{ 0, 72, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariFrenchHorn, "INT: Piccolo" },
		{ 0, 73, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariFrenchHorn, "INT: Flute" },
		{ 0, 75, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariPanFlute, "INT: Pan Flute" },
		{ 0, 77, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariShakuhachi, "INT: Shakuhachi" },
		{ 0, 104, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "INT: Sitar" },
		{ 0, 109, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariPipes, "INT: Uilleann Pipes" },
		{ 1, 109, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariPipes, "INT: Bag Pipes" },
		{ 1, 110, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariShakuhachi, "INT: Erhu" },
		{ 0, 114, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelDrums, "INT: Steel Drums" },
		{ 0, 15, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSantoor, "ExSN1: Santoor" },
		{ 1, 46, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariYangChin, "ExSN1: Yang Chin" },
		{ 1, 75, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTinWhistle, "ExSN1: Tin Whistle" },
		{ 1, 77, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariShakuhachi, "ExSN1: Ryuteki" },
		{ 0, 106, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariShamisen, "ExSN1: Tsugaru" },
		{ 1, 106, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariShamisen, "ExSN1: Sansin" },
		{ 0, 107, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleKoto,  &SnModVariKoto, "ExSN1: Koto" },
		{ 1, 107, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "ExSN1: Taishou Koto" },
		{ 0, 108, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariKalimba, "ExSN1: Kalimba" },
		{ 2, 110, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "ExSN1: Sarangi" },
		{ 0, 64, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariSax, "ExSN2: Soprano Sax" },
		{ 0, 65, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariSax, "ExSN2: Alto Sax" },
		{ 0, 66, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariSax, "ExSN2: Tenor Sax" },
		{ 0, 67, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariSax, "ExSN2: Baritone Sax" },
		{ 0, 69, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariFrenchHorn, "ExSN2: English Horn" },
		{ 1, 71, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariFrenchHorn, "ExSN2: Bass Clarinet" },
		{ 1, 73, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), &SnModScaleSax,  &SnModVariFrenchHorn, "ExSN2: Flute 2" },
		{ 0, 74, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariFrenchHorn, "ExSN2: Soprano Recorder" },
		{ 1, 74, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariFrenchHorn, "ExSN2: Alto Recorder" },
		{ 2, 74, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariFrenchHorn, "ExSN2: Tenor Recorder" },
		{ 3, 74, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariFrenchHorn, "ExSN2: Bass Recorder" },
		{ 0, 79, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariShakuhachi, "ExSN2: Ocarina SopC" },
		{ 1, 79, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariShakuhachi, "ExSN2: Ocarina SopF" },
		{ 2, 79, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariShakuhachi, "ExSN2: Ocarina Alto" },
		{ 3, 79, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariShakuhachi, "ExSN2: Ocarina Bass" },
		{ 1, 26, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariJazzGuitar, "ExSN3: TC Guitar w/Fing" },
		{ 2, 26, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariJazzGuitar, "ExSN3: 335Guitar w/Fing" },
		{ 3, 27, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "ExSN3: LP Guitar Rear" },
		{ 4, 27, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "ExSN3: LP Guitar Front" },
		{ 5, 27, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "ExSN3: 335 Guitar Half" },
		{ 1, 32, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariAcousticBass, "ExSN3: Acoustic Bass 2" },
		{ 1, 33, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariFingeredBass, "ExSN3: Fingered Bass 2" },
		{ 1, 34, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariPickedBass, "ExSN3: Picked Bass 2" },
		{ 2, 24, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  nullptr, "ExSN4: Ukulele" },
		{ 3, 24, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "ExSN4: Nylon Guitar 2" },
		{ 1, 25, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "ExSN4: 12th Steel Gtr" },
		{ 2, 25, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariMandolin, "ExSN4: Mandolin" },
		{ 3, 25, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "ExSN4: SteelFing Guitar" },
		{ 4, 25, ModPoly, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariSteelStrGuitar, "ExSN4: SteelStr Guitar2" },
		{ 1, 56, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "ExSN5: ClassicalTrumpet" },
		{ 2, 56, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "ExSN5: Flugel Horn" },
		{ 3, 56, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "ExSN5: Trumpet 2" },
		{ 4, 56, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "ExSN5: Mariachi Tp" },
		{ 1, 57, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "ExSN5: Trombone 2" },
		{ 2, 57, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "ExSN5: Bass Trombone" },
		{ 0, 58, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTuba, "ExSN5: Tuba" },
		{ 1, 59, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "ExSN5: StraightMute Tp" },
		{ 2, 59, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariTrumpet, "ExSN5: Cup Mute Trumpet" },
		{ 1, 60, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariFrenchHorn, "ExSN5: French Horn 2" },
		{ 2, 60, ModMono, &SnModSnStd[0], sizeof(SnModSnStd)/sizeof(SnModSnStd[0]), nullptr,  &SnModVariFrenchHorn, "ExSN5: Mute French Horn" }, 
    };
	template<int Index>
	constexpr const ModInstrumentTable* RefModInstrumentTable() 
	{
		static_assert(Index >= 0 && Index < NumSnModInstrumentTable - 1);
		return &SnModInstrumentTable[Index];
	}
}