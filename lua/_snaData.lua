require "_com"

ModVariationId = "MOD_SNSTD_VARIATION"

ToneCategories = {
	[0] = "No assign",
	[1] = "Piano",
	[2] = "E.Piano",
	[3] = "Organ",
	[4] = "Keyboards",
	[5] = "Accordion/Harmonica",
	[6] = "Bell/Mallet",
	[7] = "A.Guitar",
	[8] = "E.Guitar",
	[9] = "Dist.Guitar",
	[10] = "A.Bass",
	[11] = "E.Bass",
	[12] = "Synth Bass",
	[13] = "Plucked/Stroke",
	[14] = "Strings",
	[15] = "Brass",
	[16] = "Wind",
	[17] = "Flute",
	[18] = "Sax",
	[19] = "Recorder",
	[20] = "Vox/Choir",
	[21] = "Synth Lead",
	[22] = "Synth Brass",
	[23] = "Synth Pad/Strings",
	[24] = "Synth Bellpad",
	[25] = "Synth PolyKey",
	[26] = "FX",
	[27] = "Synth Seq/Pop",
	[28] = "Phrase",
	[29] = "Pulsating",
	[30] = "Beat&Groove",
	[31] = "Hit",
	[32] = "Sound FX",
	[33] = "Drums",
	[34] = "Percussion",
	[35] = "Combination",
}

MonoPoly = {
	[0] = "Mono",
	[1] = "Poly"
}

MOD_TW = {
    { desc = 'Harmonic Bar 16"', id = 'MOD_TW_HBAR_16', init = 8, min = 0, max = 8 },
    { desc = 'Harmonic Bar 5-1/3"', id = 'MOD_TW_HBAR_5_13', init = 8, min = 0, max = 8 },
    { desc = 'Harmonic Bar 8"', id = 'MOD_TW_HBAR_8', init = 8, min = 0, max = 8 },
    { desc = 'Harmonic Bar 4"', id = 'MOD_TW_HBAR_4', init = 0, min = 0, max = 8 },
    { desc = 'Harmonic Bar 2-2/3"', id = 'MOD_TW_HBAR_2_23', init = 0, min = 0, max = 8 },
    { desc = 'Harmonic Bar 2"', id = 'MOD_TW_HBAR_2', init = 0, min = 0, max = 8 },
    { desc = 'Harmonic Bar 1-3/5"', id = 'MOD_TW_HBAR_1_35', init = 0, min = 0, max = 8 },
    { desc = 'Harmonic Bar 1-1/3"', id = 'MOD_TW_HBAR_1_13', init = 0, min = 0, max = 8 },
    { desc = 'Harmonic Bar 1"', id = 'MOD_TW_HBAR_1', init = 0, min = 0, max = 8 },
    { desc = 'Percussion Switch', id = 'MOD_TW_PERC_SW', init = 1, min = 0, max = 1 },
    { desc = 'Percussion Harmonic', id = 'MOD_TW_PERC_HARM', init = 1, min = 0, max = 1 },
    { desc = 'Percussion Slow', id = 'MOD_TW_PERC_SLOW', init = 0, min = 0, max = 1 },
    { desc = 'Key On Click Level', id = 'MOD_TW_ON_CLICK', init = 5, min = 0, max = 31 },
    { desc = 'Key Off Click Level', id = 'MOD_TW_OFF_CLICK', init = 5, min = 0, max = 31 },
    { desc = 'Percussion Soft Level', id = 'MOD_TW_PERC_SOFT_LEVEL', init = 3, min = 0, max = 15 },
    { desc = 'Percussion Normal Level', id = 'MOD_TW_PERC_NORM_LEVEL', init = 10, min = 0, max = 15 },
    { desc = 'Percussion Slow Time', id = 'MOD_TW_PERC_SLOW_TIME', init = 104, min = 0, max = 127 },
    { desc = 'Percussion Fast Time', id = 'MOD_TW_PERC_FAST_TIME', init = 43, min = 0, max = 127 },
    { desc = 'Percussion Recharge Time', id = 'MOD_TW_PERC_RECHARGE', init = 5, min = 0, max = 10 },
    { desc = 'Percussion Harmonic Bar Level', id = 'MOD_TW_PERC_HBAR_LEVEL', init = 64, min = 0, max = 127 },
    { desc = 'Percussion Soft', id = 'MOD_TW_PERC_SOFT', init = 0, min = 0, max = 1 },
    { desc = 'Leakage Level', id = 'MOD_TW_LEAKAGE', init = 20, min = 0, max = 127 }
}

MOD_SNAP = {
    { desc = 'String Resonance', id = 'MOD_SNAP_STRING_RESO_LEVEL', init = 64, min = 0, max = 127 },
    { desc = 'Key Off Resonance', id = 'MOD_SNAP_KEY_OFF_RESO_LEVEL', init = 64, min = 0, max = 127 },
    { desc = 'Hammer Noise', id = 'MOD_SNAP_THUMP_LEVEL', init = 64, min = 62, max = 66 },
    { desc = 'Stereo Width', id = 'MOD_SNAP_STEREO_WIDTH', init = 63, min = 0, max = 63 },
    { desc = 'Nuance', id = 'MOD_SNAP_NUANCE', init = 0, min = 0, max = 2 },
    { desc = 'Tone Character', id = 'MOD_SNAP_SPLIT_SHIFT', init = 64, min = 59, max = 69 }
}

MOD_SNSTD = {
    { desc = 'Noise Level (CC#16)', id = 'MOD_SNSTD_NOISE_LEVEL', init = 64, min = 0, max = 127, cc =16 },
    { desc = 'Rate (CC#17)', id = 'MOD_SNSTD_RATE', init = 64, min = 0, max = 127, cc =17 },
    { desc = 'Growl Sens (CC#18)', id = 'MOD_SNSTD_GROWL_SENS', init = 0, min = 0, max = 127, cc =18 },
    { desc = 'Mode (CC#19)', id = 'MOD_SNSTD_MODE', init = 0, min = 0, max = 1, cc =19 },
    { desc = 'Drone Level', id = 'MOD_SNSTD_DRONE_LEVEL', init = 64, min = 0, max = 127 },
    { desc = 'Drone Pitch', id = 'MOD_SNSTD_DRONE_PITCH', init = 64, min = 52, max = 76 },
    { desc = 'Play Scale', id = 'MOD_SNSTD_PLAY_SCALE', init = 0, min = 0, max = 0 },
    { desc = 'Scale Key', id = 'MOD_SNSTD_SCALE_KEY', init = 0, min = 0, max = 11 },
    { desc = 'Glide', id = 'MOD_SNSTD_GLIDE', init = 0, min = 0, max = 1 },
    { desc = 'Variation', id = ModVariationId, init = 0, min = 0, max = 0 },
    { desc = 'Picking Harmonics', id = 'MOD_SNSTD_PICKHARM', init = 0, min = 0, max = 1 },
    { desc = 'Buzz Key Switch', id = 'MOD_SNSTD_KEYSW', init = 0, min = 0, max = 1 },
    { desc = 'Sub String Tune', id = 'MOD_SNSTD_OFFSET1', init = 64, min = 0, max = 127 }
}


local variMarimba        = { desc= 'Variation', id= 'VARI_MARIMBA', init= 0, min= 0, max= 1 }
local variVibraphone     = { desc= 'Variation', id= 'VARI_VIBRAPHONE', init= 0, min= 0, max= 2 }
local variSteelStrGuitar = { desc= 'Variation', id= 'VARI_STEELSTRGUITAR', init= 0, min= 0, max= 2 }
local variFlamencoGuitar = { desc= 'Variation', id= 'VARI_FLAMENCOGUITAR', init= 0, min= 0, max= 2 }
local variJazzGuitar     = { desc= 'Variation', id= 'VARI_JAZZGUITAR', init= 0, min= 0, max= 2 }
local variAcousticBass   = { desc= 'Variation', id= 'VARI_ACOUSTICBASS', init= 0, min= 0, max= 2 }
local variFingeredBass   = { desc= 'Variation', id= 'VARI_FINGEREDBASS', init= 0, min= 0, max= 2 }
local variPickedBass     = { desc= 'Variation', id= 'VARI_PICKEDBASS', init= 0, min= 0, max= 2 }
local variViolin         = { desc= 'Variation', id= 'VARI_VIOLIN', init= 0, min= 0, max= 3 }
local variHarp           = { desc= 'Variation', id= 'VARI_HARP', init= 0, min= 0, max= 1 }
local variTimpani        = { desc= 'Variation', id= 'VARI_TIMPANI', init= 0, min= 0, max= 2 }
local variStrings        = { desc= 'Variation', id= 'VARI_STRINGS', init= 0, min= 0, max= 3 }
local variChoir          = { desc= 'Variation', id= 'VARI_CHOIR', init= 0, min= 0, max= 1 }
local variTrumpet        = { desc= 'Variation', id= 'VARI_TRUMPET', init= 0, min= 0, max= 2 }
local variFrenchHorn     = { desc= 'Variation', id= 'VARI_FRENCHHORN', init= 0, min= 0, max= 1 }
local variSax            = { desc= 'Variation', id= 'VARI_SAX', init= 0, min= 0, max= 3 }
local variPanFlute       = { desc= 'Variation', id= 'VARI_PANFLUTE', init= 0, min= 0, max= 2 }
local variShakuhachi     = { desc= 'Variation', id= 'VARI_SHAKUHACHI', init= 0, min= 0, max= 2 }
local variPipes          = { desc= 'Variation', id= 'VARI_PIPES', init= 0, min= 0, max= 2 }
local variSteelDrums     = { desc= 'Variation', id= 'VARI_STEELDRUMS', init= 0, min= 0, max= 1 }
local variSantoor        = { desc= 'Variation', id= 'VARI_SANTOOR', init= 0, min= 0, max= 2 }
local variYangChin       = { desc= 'Variation', id= 'VARI_YANGCHIN', init= 0, min= 0, max= 2 }
local variTinWhistle     = { desc= 'Variation', id= 'VARI_TINWHISTLE', init= 0, min= 0, max= 2 }
local variShamisen       = { desc= 'Variation', id= 'VARI_SHAMISEN', init= 0, min= 0, max= 3 }
local variKoto           = { desc= 'Variation', id= 'VARI_KOTO', init= 0, min= 0, max= 2 }
local variKalimba        = { desc= 'Variation', id= 'VARI_KALIMBA', init= 0, min= 0, max= 1 }
local variMandolin       = { desc= 'Variation', id= 'VARI_MANDOLIN', init= 0, min= 0, max= 2 }
local variTuba           = { desc= 'Variation', id= 'VARI_TUBA', init= 0, min= 0, max= 1 }

local scaleHarp = { desc= 'Play Scale', id= 'SCALE_HARP', init= 0, min= 0, max= 6 }
local scaleSax  = { desc= 'Play Scale', id= 'SCALE_SAX',  init= 0, min= 0, max= 5 }
local scaleKoto = { desc= 'Play Scale', id= 'SCALE_KOTO', init= 0, min= 0, max= 1 }

SnaInstPresetData = {
    { bank= 'INT',   desc= 'Concert Grand',    scale=nil, vari=nil, lsb= 64, pc= 0, cate= 'Ac.Piano', modData = MOD_SNAP},
    { bank= 'INT',   desc= 'Grand Piano1',     scale=nil, vari=nil, lsb= 64, pc= 1, cate= 'Ac.Piano', modData = MOD_SNAP},
    { bank= 'INT',   desc= 'Grand Piano2',     scale=nil, vari=nil, lsb= 64, pc= 2, cate= 'Ac.Piano', modData = MOD_SNAP},
    { bank= 'INT',   desc= 'Grand Piano3',     scale=nil, vari=nil, lsb= 64, pc= 3, cate= 'Ac.Piano', modData = MOD_SNAP},
    { bank= 'INT',   desc= 'Mellow Piano',     scale=nil, vari=nil, lsb= 64, pc= 4, cate= 'Ac.Piano', modData = MOD_SNAP},
    { bank= 'INT',   desc= 'Bright Piano',     scale=nil, vari=nil, lsb= 64, pc= 5, cate= 'Ac.Piano', modData = MOD_SNAP},
    { bank= 'INT',   desc= 'Upright Piano',    scale=nil, vari=nil, lsb= 64, pc= 6, cate= 'Ac.Piano', modData = MOD_SNAP},
    { bank= 'INT',   desc= 'Concert Mono',     scale=nil, vari=nil, lsb= 64, pc= 7, cate= 'Ac.Piano', modData = MOD_SNAP},
    { bank= 'INT',   desc= 'Honky-tonk',       scale=nil, vari=nil, lsb= 64, pc= 8, cate= 'Ac.Piano', modData = MOD_SNAP},
    { bank= 'INT',   desc= 'Pure Vintage EP1', scale=nil, vari=nil, lsb= 0, pc= 4, mod1= 'Noise Level', cate= 'E.Piano', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Pure Vintage EP2', scale=nil, vari=nil, lsb= 1, pc= 4, mod1= 'Noise Level', cate= 'E.Piano', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Pure Wurly',       scale=nil, vari=nil, lsb= 2, pc= 4, mod1= 'Noise Level', cate= 'E.Piano', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Pure Vintage EP3', scale=nil, vari=nil, lsb= 3, pc= 4, mod1= 'Noise Level', cate= 'E.Piano', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Old Hammer EP',    scale=nil, vari=nil, lsb= 6, pc= 4, mod1= 'Noise Level', cate= 'E.Piano', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Dyno Piano',       scale=nil, vari=nil, lsb= 7, pc= 4, mod1= 'Noise Level', cate= 'E.Piano', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Clav CB Flat',     scale=nil, vari=nil, lsb= 0, pc= 7, mod1= 'Noise Level', cate= 'Other Keyboards', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Clav CA Flat',     scale=nil, vari=nil, lsb= 1, pc= 7, mod1= 'Noise Level', cate= 'Other Keyboards', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Clav CB Medium',   scale=nil, vari=nil, lsb= 2, pc= 7, mod1= 'Noise Level', cate= 'Other Keyboards', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Clav CA Medium',   scale=nil, vari=nil, lsb= 3, pc= 7, mod1= 'Noise Level', cate= 'Other Keyboards', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Clav CB Brillia',  scale=nil, vari=nil, lsb= 4, pc= 7, mod1= 'Noise Level', cate= 'Other Keyboards', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Clav CA Brillia',  scale=nil, vari=nil, lsb= 5, pc= 7, mod1= 'Noise Level', cate= 'Other Keyboards', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Clav CB Combo',    scale=nil, vari=nil, lsb= 6, pc= 7, mod1= 'Noise Level', cate= 'Other Keyboards', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Clav CA Combo',    scale=nil, vari=nil, lsb= 7, pc= 7, mod1= 'Noise Level', cate= 'Other Keyboards', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Glockenspiel',     scale=nil, vari=variMarimba, lsb= 0, pc= 9, mod1= 'Mallet Hardness', mod2= 'Roll Speed', mod10= 'Variation', cate= 'Bell/Mallet', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Vibraphone',       scale=nil, vari=variVibraphone, lsb= 0, pc= 11, mod1= 'Mallet Hardness', mod2= 'Roll Speed', mod10= 'Variation', cate= 'Bell/Mallet', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Marimba',          scale=nil, vari=variMarimba, lsb= 0, pc= 12, mod1= 'Mallet Hardness', mod2= 'Roll Speed', mod10= 'Variation', cate= 'Bell/Mallet', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Xylophone',        scale=nil, vari=variMarimba, lsb= 0, pc= 13, mod1= 'Mallet Hardness', mod2= 'Roll Speed', mod10= 'Variation', cate= 'Bell/Mallet', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Tubular Bells',    scale=nil, vari=variMarimba, lsb= 0, pc= 14, mod1= 'Mallet Hardness', mod2= 'Roll Speed', mod10= 'Variation', cate= 'Bell/Mallet', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'TW Organ',         scale=nil, vari=nil, lsb= 65, pc= 0, cate= 'Organ', modData=MOD_TW },
    { bank= 'INT',   desc= 'French Accordion', scale=nil, vari=nil, lsb= 0, pc= 21, mod1= 'Noise Level', cate= 'Accordion/Harmonica', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'ItalianAccordion', scale=nil, vari=nil, lsb= 1, pc= 21, mod1= 'Noise Level', cate= 'Accordion/Harmonica', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Harmonica',        scale=nil, vari=nil, lsb= 0, pc= 22, mod1= 'Noise Level', mod3= 'Growl Sens', cate= 'Accordion/Harmonica', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Bandoneon',        scale=nil, vari=nil, lsb= 0, pc= 23, mod1= 'Noise Level', cate= 'Accordion/Harmonica', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Nylon Guitar',     scale=nil, vari=variSteelStrGuitar, lsb= 0, pc= 24, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Flamenco Guitar',  scale=nil, vari=variFlamencoGuitar, lsb= 1, pc= 24, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'SteelStr Guitar',  scale=nil, vari=variSteelStrGuitar, lsb= 0, pc= 25, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Jazz Guitar',      scale=nil, vari=variJazzGuitar, lsb= 0, pc= 26, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', cate= 'E.Guitar', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'ST Guitar Half',   scale=nil, vari=variSteelStrGuitar, lsb= 0, pc= 27, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', mod11= 'Picking Harmonics', cate= 'E.Guitar', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'ST Guitar Front',  scale=nil, vari=variSteelStrGuitar, lsb= 1, pc= 27, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', mod11= 'Picking Harmonics', cate= 'E.Guitar', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'TC Guitar Rear',   scale=nil, vari=variSteelStrGuitar, lsb= 2, pc= 27, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', mod11= 'Picking Harmonics', cate= 'E.Guitar', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Acoustic Bass',    scale=nil, vari=variAcousticBass, lsb= 0, pc= 32, mod1= 'Noise Level', mod10= 'Variation', cate= 'Ac.Bass', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Fingered Bass',    scale=nil, vari=variFingeredBass, lsb= 0, pc= 33, mod1= 'Noise Level', mod10= 'Variation', cate= 'E.Piano', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Picked Bass',      scale=nil, vari=variPickedBass, lsb= 0, pc= 34, mod1= 'Noise Level', mod10= 'Variation', cate= 'E.Piano', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Fretless Bass',    scale=nil, vari=variAcousticBass, lsb= 0, pc= 35, mod1= 'Noise Level', mod10= 'Variation', cate= 'E.Piano', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Violin',           scale=nil, vari=variViolin, lsb= 0, pc= 40, mod1= 'Noise Level', mod10= 'Variation', cate= 'Strings', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Violin 2',         scale=nil, vari=variViolin, lsb= 1, pc= 40, mod1= 'Noise Level', mod10= 'Variation', cate= 'Strings', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Viola',            scale=nil, vari=variViolin, lsb= 0, pc= 41, mod1= 'Noise Level', mod10= 'Variation', cate= 'Strings', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Cello',            scale=nil, vari=variViolin, lsb= 0, pc= 42, mod1= 'Noise Level', mod10= 'Variation', cate= 'Strings', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Cello 2',          scale=nil, vari=variViolin, lsb= 1, pc= 42, mod1= 'Noise Level', mod10= 'Variation', cate= 'Strings', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Contrabass',       scale=nil, vari=variViolin, lsb= 0, pc= 43, mod1= 'Noise Level', mod10= 'Variation', cate= 'Strings', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Harp',             scale=scaleHarp, vari=variHarp, lsb= 0, pc= 46, mod4= 'Glissando Mode', mod7= 'Play Scale', mod8= 'Scale Key', mod10= 'Variation', cate= 'Plucked/Stroke', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Timpani',          scale=nil, vari= variTimpani, lsb= 0, pc= 47, mod2= 'Roll Speed', mod10= 'Variation', cate= 'Percussion', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Strings',          scale=nil, vari= variStrings, lsb= 0, pc= 48, mod4= 'Hold Legato Mode', mod10= 'Variation', cate= 'Strings', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Marcato Strings',  scale=nil, vari= variStrings, lsb= 1, pc= 48, mod4= 'Hold Legato Mode', mod10= 'Variation', cate= 'Strings', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'London Choir',     scale=nil, vari= variChoir, lsb= 0, pc= 52, mod4= 'Hold Legato Mode', mod10= 'Variation', cate= 'Vox/Choir', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Boys Choir',       scale=nil, vari= variChoir, lsb= 1, pc= 52, mod4= 'Hold Legato Mode', mod10= 'Variation', cate= 'Vox/Choir', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Trumpet',          scale=nil, vari= variTrumpet, lsb= 0, pc= 56, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Trombone',         scale=nil, vari= variTrumpet, lsb= 0, pc= 57, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Tb2 CupMute',      scale=nil, vari= variTrumpet, lsb= 3, pc= 57, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Mute Trumpet',     scale=nil, vari= variTrumpet, lsb= 0, pc= 59, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'French Horn',      scale=nil, vari= variFrenchHorn, lsb= 0, pc= 60, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Soprano Sax 2',    scale=scaleSax, vari= variSax, lsb= 1, pc= 64, mod1= 'Noise Level', mod3= 'Growl Sens', mod7= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Sax', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Alto Sax 2',       scale=scaleSax, vari= variSax, lsb= 1, pc= 65, mod1= 'Noise Level', mod3= 'Growl Sens', mod7= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Sax', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Tenor Sax 2',      scale=scaleSax, vari= variSax, lsb= 2, pc= 66, mod1= 'Noise Level', mod3= 'Growl Sens', mod7= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Sax', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Baritone Sax 2',   scale=scaleSax, vari= variSax, lsb= 1, pc= 67, mod1= 'Noise Level', mod3= 'Growl Sens', mod7= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Sax', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Oboe',             scale=scaleSax, vari= variFrenchHorn, lsb= 0, pc= 68, mod1= 'Noise Level', mod3= 'Growl Sens', mod7= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Wind', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Bassoon',          scale=scaleSax, vari= variFrenchHorn, lsb= 0, pc= 70, mod1= 'Noise Level', mod3= 'Growl Sens', mod7= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Wind', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Clarinet',         scale=scaleSax, vari= variFrenchHorn, lsb= 0, pc= 71, mod1= 'Noise Level', mod3= 'Growl Sens', mod7= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Wind', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Piccolo',          scale=scaleSax, vari= variFrenchHorn, lsb= 0, pc= 72, mod1= 'Noise Level', mod3= 'Growl Sens', mod7= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Flute', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Flute',            scale=scaleSax, vari= variFrenchHorn, lsb= 0, pc= 73, mod1= 'Noise Level', mod3= 'Growl Sens', mod7= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Flute', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Pan Flute',        scale=nil, vari= variPanFlute, lsb= 0, pc= 75, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Flute', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Shakuhachi',       scale=nil, vari= variShakuhachi, lsb= 0, pc= 77, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Flute', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Sitar',            scale=nil, vari= nil, lsb= 0, pc= 104, mod1= 'Resonance Level', mod5= 'Tambura Level', mod6= 'Tambura Pitch', cate= 'Plucked/Stroke', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Uilleann Pipes',   scale=nil, vari= variPipes, lsb= 0, pc= 109, mod5= 'Drone Level', mod6= 'Drone Pitch', mod10= 'Variation', cate= 'Wind', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Bag Pipes',        scale=nil, vari= variPipes, lsb= 1, pc= 109, mod5= 'Drone Level', mod6= 'Drone Pitch', mod10= 'Variation', cate= 'Wind', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Erhu',             scale=nil, vari= variShakuhachi, lsb= 1, pc= 110, mod1= 'Noise Level', mod10= 'Variation', cate= 'Strings', modData=MOD_SNSTD },
    { bank= 'INT',   desc= 'Steel Drums',      scale=nil, vari= variSteelDrums, lsb= 0, pc= 114, mod1= 'Resonance Level', mod2= 'Roll Speed', mod10= 'Variation', cate= 'Percussion', modData=MOD_SNSTD },
    { bank= 'ExSN1', desc= 'Santoor',          scale=nil, vari= variSantoor, lsb= 0, pc= 15, mod1= 'Resonance Level', mod2= 'Roll Speed', mod10= 'Variation', cate= 'Bell/Mallet', modData=MOD_SNSTD },
    { bank= 'ExSN1', desc= 'Yang Chin',        scale=nil, vari= variYangChin, lsb= 1, pc= 46, mod1= 'Resonance Level', mod2= 'Roll Speed', mod10= 'Variation', cate= 'Bell/Mallet', modData=MOD_SNSTD },
    { bank= 'ExSN1', desc= 'Tin Whistle',      scale=nil, vari= variTinWhistle, lsb= 1, pc= 75, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Flute', modData=MOD_SNSTD },
    { bank= 'ExSN1', desc= 'Ryuteki',          scale=nil, vari= variShakuhachi, lsb= 1, pc= 77, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Flute', modData=MOD_SNSTD },
    { bank= 'ExSN1', desc= 'Tsugaru',          scale=nil, vari= variShamisen, lsb= 0, pc= 106, mod1= 'Resonance Level', mod2= 'Bend Depth', mod10= 'Variation', mod12= 'Buzz Key Sw', cate= 'Plucked/Stroke', modData=MOD_SNSTD },
    { bank= 'ExSN1', desc= 'Sansin',           scale=nil, vari= variShamisen, lsb= 1, pc= 106, mod1= 'Resonance Level', mod2= 'Bend Depth', mod10= 'Variation', mod12= 'Buzz Key Sw', cate= 'Plucked/Stroke', modData=MOD_SNSTD },
    { bank= 'ExSN1', desc= 'Koto',             scale=scaleKoto, vari= variKoto, lsb= 0, pc= 107, mod2= 'Tremolo Speed', mod4= 'Glissando Mode', mod7= 'Play Scale', mod8= 'Scale Key', mod10= 'Variation', mod12= 'Buzz Key Sw', cate= 'Plucked/Stroke', modData=MOD_SNSTD },
    { bank= 'ExSN1', desc= 'Taishou Koto',     scale=nil, vari= nil, lsb= 1, pc= 107, mod1= 'Noise Level', mod2= 'Tremolo Speed', cate= 'Plucked/Stroke', modData=MOD_SNSTD },
    { bank= 'ExSN1', desc= 'Kalimba',          scale=nil, vari= variKalimba, lsb= 0, pc= 108, mod1= 'Resonance Level', mod10= 'Variation', cate= 'Plucked/Stroke', modData=MOD_SNSTD },
    { bank= 'ExSN1', desc= 'Sarangi',          scale=nil, vari= nil, lsb= 2, pc= 110, mod1= 'Resonance Level', mod5= 'Tambura Level', mod6= 'Tambura Pitch', cate= 'Strings', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Soprano Sax',      scale=scaleSax, vari= variSax, lsb= 0, pc= 64, mod1= 'Noise Level', mod3= 'Growl Sens', mod6= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Sax', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Alto Sax',         scale=scaleSax, vari= variSax, lsb= 0, pc= 65, mod1= 'Noise Level', mod3= 'Growl Sens', mod6= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Sax', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Tenor Sax',        scale=scaleSax, vari= variSax, lsb= 0, pc= 66, mod1= 'Noise Level', mod3= 'Growl Sens', mod6= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Sax', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Baritone Sax',     scale=scaleSax, vari= variSax, lsb= 0, pc= 67, mod1= 'Noise Level', mod3= 'Growl Sens', mod6= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Sax', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'English Horn',     scale=scaleSax, vari= variFrenchHorn, lsb= 0, pc= 69, mod1= 'Noise Level', mod3= 'Growl Sens', mod6= 'Play Scale', mod8= 'Scale Key', mod9= 'Glide', mod10= 'Variation', cate= 'Wind', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Bass Clarinet',    scale=scaleSax, vari= variFrenchHorn, lsb= 1, pc= 71, mod1= 'Noise Level', mod3= 'Growl Sens', mod6= 'Play Scale', mod8= 'Scale Key', mod10= 'Variation', cate= 'Wind', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Flute 2',          scale=scaleSax, vari= variFrenchHorn, lsb= 1, pc= 73, mod1= 'Noise Level', mod3= 'Growl Sens', mod6= 'Play Scale', mod8= 'Scale Key', mod10= 'Variation', cate= 'Flute', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Soprano Recorder', scale=nil, vari= variFrenchHorn, lsb= 0, pc= 74, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Recorder', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Alto Recorder',    scale=nil, vari= variFrenchHorn, lsb= 1, pc= 74, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Recorder', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Tenor Recorder',   scale=nil, vari= variFrenchHorn, lsb= 2, pc= 74, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Recorder', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Bass Recorder',    scale=nil, vari= variFrenchHorn, lsb= 3, pc= 74, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Recorder', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Ocarina SopC',     scale=nil, vari= variShakuhachi, lsb= 0, pc= 79, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Recorder', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Ocarina SopF',     scale=nil, vari= variShakuhachi, lsb= 1, pc= 79, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Recorder', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Ocarina Alto',     scale=nil, vari= variShakuhachi, lsb= 2, pc= 79, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Recorder', modData=MOD_SNSTD },
    { bank= 'ExSN2', desc= 'Ocarina Bass',     scale=nil, vari= variShakuhachi, lsb= 3, pc= 79, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Recorder', modData=MOD_SNSTD },
    { bank= 'ExSN3', desc= 'TC Guitar w/Fing', scale=nil, vari= variJazzGuitar, lsb= 1, pc= 26, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN3', desc= '335Guitar w/Fing', scale=nil, vari= variJazzGuitar, lsb= 2, pc= 26, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN3', desc= 'LP Guitar Rear',   scale=nil, vari= variSteelStrGuitar, lsb= 3, pc= 27, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', mod11= 'Picking Harmonics', cate= 'E.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN3', desc= 'LP Guitar Front',  scale=nil, vari= variSteelStrGuitar, lsb= 4, pc= 27, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', mod11= 'Picking Harmonics', cate= 'E.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN3', desc= '335 Guitar Half',  scale=nil, vari= variSteelStrGuitar, lsb= 5, pc= 27, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', mod11= 'Picking Harmonics', cate= 'E.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN3', desc= 'Acoustic Bass 2',  scale=nil, vari= variAcousticBass, lsb= 1, pc= 32, mod1= 'Noise Level', mod10= 'Variation', cate= 'Ac.Bass', modData=MOD_SNSTD },
    { bank= 'ExSN3', desc= 'Fingered Bass 2',  scale=nil, vari= variFingeredBass, lsb= 1, pc= 33, mod1= 'Noise Level', mod10= 'Variation', cate= 'E.Bass', modData=MOD_SNSTD },
    { bank= 'ExSN3', desc= 'Picked Bass 2',    scale=nil, vari= variPickedBass, lsb= 1, pc= 34, mod1= 'Noise Level', mod10= 'Variation', cate= 'E.Bass', modData=MOD_SNSTD },
    { bank= 'ExSN4', desc= 'Ukulele',          scale=nil, vari= nil, lsb= 2, pc= 24, mod2= 'Strum Speed', mod4= 'Strum Mode', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN4', desc= 'Nylon Guitar 2',   scale=nil, vari= variSteelStrGuitar, lsb= 3, pc= 24, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN4', desc= '12th Steel Gtr',   scale=nil, vari= variSteelStrGuitar, lsb= 1, pc= 25, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', mod13= 'Sub String Tune', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN4', desc= 'Mandolin',         scale=nil, vari= variMandolin, lsb= 2, pc= 25, mod1= 'Noise Level', mod2= 'Tremolo Speed', mod4= 'Strum Mode', mod10= 'Variation', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN4', desc= 'SteelFing Guitar', scale=nil, vari= variSteelStrGuitar, lsb= 3, pc= 25, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN4', desc= 'SteelStr Guitar2', scale=nil, vari= variSteelStrGuitar, lsb= 4, pc= 25, mod1= 'Noise Level', mod2= 'Strum Speed', mod4= 'Strum Mode', mod10= 'Variation', cate= 'Ac.Guitar', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'ClassicalTrumpet', scale=nil, vari= variTrumpet, lsb= 1, pc= 56, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'Flugel Horn',      scale=nil, vari= variTrumpet, lsb= 2, pc= 56, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'Trumpet 2',        scale=nil, vari= variTrumpet, lsb= 3, pc= 56, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'Mariachi Tp',      scale=nil, vari= variTrumpet, lsb= 4, pc= 56, mod1= 'Noise Level', mod2= 'Crescendo Depth', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'Trombone 2',       scale=nil, vari= variTrumpet, lsb= 1, pc= 57, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'Bass Trombone',    scale=nil, vari= variTrumpet, lsb= 2, pc= 57, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'Tuba',             scale=nil, vari= variTuba, lsb= 0, pc= 58, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'StraightMute Tp',  scale=nil, vari= variTrumpet, lsb= 1, pc= 59, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'Cup Mute Trumpet', scale=nil, vari= variTrumpet, lsb= 2, pc= 59, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'French Horn 2',    scale=nil, vari= variFrenchHorn, lsb= 1, pc= 60, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass', modData=MOD_SNSTD },
    { bank= 'ExSN5', desc= 'Mute French Horn', scale=nil, vari= variFrenchHorn, lsb= 2, pc= 60, mod1= 'Noise Level', mod3= 'Growl Sens', mod10= 'Variation', cate= 'Brass' }
}

SnaInstPresets = MapArray(SnaInstPresetData, function (v)
	return v.desc
end)