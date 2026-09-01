#include "LevelScript.h"
#include "Actor.h"
#include "N64Rom.h"
#include "ScrollingTexture.h"
#include "GeoLayout.h"
#include "Collision.h"
#include "MovingTexture.h"
#include "Model.h"
#include "Segment2.h"
#include "Sound.h"
#include "MacroObject.h"
#include "BehaviorScript.h"
#include "Memory.h"

namespace ActorGroup {
    using GroupMap = std::unordered_map<u32, std::string>;

    const std::unordered_map<std::string, GroupMap> AllGroups = {
        {"common0", {
            {0x0f000000, "blue_coin_switch_geo"},
            {0x0f000028, "amp_geo"},
            {0x0f0004cc, "purple_switch_geo"},
            {0x0f0004e4, "checkerboard_platform_geo"},
            {0x0f0005d0, "breakable_box_geo"},
            {0x0f000610, "breakable_box_small_geo"},
            {0x0f000a58, "exclamation_box_outline_geo"},
            {0x0f000694, "exclamation_box_geo"},
            {0x0f0006e4, "goomba_geo"},
            {0x08025f08, "exclamation_box_outline_seg8_dl_08025F08"},
            {0x0f000ab0, "koopa_shell_geo"},
            {0x0f000a30, "metal_box_geo"},
            {0x08024bb8, "metal_box_dl"},
            {0x0f0007b8, "black_bobomb_geo"},
            {0x0f0008f4, "bobomb_buddy_geo"},
            {0x080048e0, "cannon_lid_seg8_dl_080048E0"},
            {0x0f000640, "bowling_ball_geo"},
            {0x0f0001c0, "cannon_barrel_geo"},
            {0x0f0001a8, "cannon_base_geo"},
            {0x0f0004fc, "heart_geo"},
            {0x0f000518, "flyguy_geo"},
            {0x0f0001d8, "chuckya_geo"},
            {0x0f00066c, "bowling_ball_track_geo"}
        }},
        {"common1", {
            {0x160007b4, "castle_door_0_star_geo"},
            {0x16000868, "castle_door_1_star_geo"},
            {0x1600091c, "castle_door_3_stars_geo"},
            {0x160009d0, "key_door_geo"},
            {0x16000720, "haunted_door_geo"},
            {0x1600043c, "cabin_door_geo"},
            {0x160004d0, "wooden_door_geo"},
            {0x160005f8, "metal_door_geo"},
            {0x1600068c, "hazy_maze_door_geo"},
            {0x16001048, "palm_tree_geo"},
            {0x16001018, "snow_tree_geo"},
            {0x16000388, "warp_pipe_geo"},
            {0x16001000, "spiky_tree_geo"},
            {0x16000fe8, "bubbly_tree_geo"},
            {0x160003a8, "castle_door_geo"},
            {0x1600013c, "yellow_coin_geo"},
            {0x16000ea0, "star_geo"},
            {0x16000f6c, "transparent_star_geo"},
            {0x16000fb4, "wooden_signpost_geo"},
            {0x16000b2c, "red_flame_geo"},
            {0x16000b8c, "blue_flame_geo"},
            {0x16000c8c, "leaves_geo"},
            {0x16000c44, "fish_geo"},
            {0x16000bec, "fish_shadow_geo"},
            {0x0302bcd0, "sand_seg3_dl_0302BCD0"},
            {0x160000a8, "butterfly_geo"},
            {0x0301cb00, "pebble_seg3_dl_0301CB00"},
            {0x16000000, "mist_geo"},
            {0x16000020, "white_puff_geo"},
            {0x0302c8a0, "white_particle_dl"},
            {0x16000f98, "white_particle_geo"},
            {0x160001a0, "yellow_coin_no_shadow_geo"},
            {0x16000200, "blue_coin_geo"},
            {0x16000264, "blue_coin_no_shadow_geo"},
            {0x16000da8, "marios_winged_metal_cap_geo"},
            {0x16000cf0, "marios_metal_cap_geo"},
            {0x16000d3c, "marios_wing_cap_geo"},
            {0x16000ca4, "marios_cap_geo"},
            {0x16000ab0, "bowser_key_cutscene_geo"},
            {0x16000a84, "bowser_key_geo"},
            {0x16000b10, "red_flame_shadow_geo"},
            {0x16000e84, "mushroom_1up_geo"},
            {0x160002c4, "red_coin_geo"},
            {0x16000328, "red_coin_no_shadow_geo"},
            {0x16000e14, "number_geo"},
            {0x16000040, "explosion_geo"},
            {0x16000ed4, "dirt_animation_geo"},
            {0x16000f24, "cartoon_star_geo"}
        }},
        {"group0", {
            {0x17000284, "sparkles_animation_geo"},
            {0x1700001c, "purple_marble_geo"},
            {0x17000084, "burn_smoke_geo"},
            {0x04032a18, "white_particle_small_dl"},
            {0x17002dd4, "mario_geo"},
            {0x17000038, "smoke_geo"},
            {0x170001bc, "sparkles_geo"},
            {0x17000000, "bubble_geo"},
            {0x1700009c, "small_water_splash_geo"},
            {0x17000124, "idle_water_wave_geo"},
            {0x17000230, "water_splash_geo"},
            {0x17000168, "wave_trail_geo"}
        }},
        {"group1", {
            {0x0c000264, "bullet_bill_geo"},
            {0x0c000000, "yellow_sphere_geo"},
            {0x0c000018, "hoot_geo"},
            {0x0c0001e4, "yoshi_egg_geo"},
            {0x0c000248, "thwomp_geo"},
            {0x0c00028c, "heave_ho_geo"}
        }},
        {"group10", {
            {0x0c000000, "birds_geo"},
            {0x0c000098, "peach_geo"},
            {0x0c000468, "yoshi_geo"}
        }},
        {"group11", {
            {0x0c0001bc, "enemy_lakitu_geo"},
            {0x0c000290, "spiny_ball_geo"},
            {0x0c000328, "spiny_geo"},
            {0x0c000030, "wiggler_head_geo"},
            {0x0500C778, "wiggler_body_geo"},
            {0x0c000000, "bubba_geo"}
        }},
        {"group12", {
            {0x0d000ac4, "bowser_geo"},
            {0x0d000bbc, "bowser_bomb_geo"},
            {0x0d000bfc, "bowser_impact_smoke_geo"},
            {0x0d000000, "bowser_flames_geo"},
            {0x0d000090, "invisible_bowser_accessory_geo"},
            {0x0d000b40, "bowser2_geo"}
        }},
        {"group13", {
            {0x0d00038c, "bub_geo"},
            {0x0d000450, "treasure_chest_base_geo"},
            {0x0d000468, "treasure_chest_lid_geo"},
            {0x0d000324, "cyan_fish_geo"},
            {0x0d000414, "water_ring_geo"},
            {0x0d0002f4, "water_mine_geo"},
            {0x0d000284, "seaweed_geo"},
            {0x0d000000, "skeeter_geo"}
        }},
        {"group14", {
            {0x0d000358, "piranha_plant_geo"},
            {0x0d000480, "whomp_geo"},
            {0x0d000214, "koopa_with_shell_geo"},
            {0x0d0000d0, "koopa_without_shell_geo"},
            {0x0d0005d0, "metallic_ball_geo"},
            {0x0d0005ec, "chain_chomp_geo"},
            {0x0d000000, "koopa_flag_geo"},
            {0x0d0000b8, "wooden_post_geo"}
        }},
        {"group15", {
            {0x0d000448, "mips_geo"},
            {0x0d0005b0, "boo_castle_geo"},
            {0x0d000000, "lakitu_geo"},
            {0x0d0003e4, "toad_geo"}
        }},
        {"group16", {
            {0x06003754, "chilly_chief_geo"},
            {0x06003874, "chilly_chief_big_geo"},
            {0x0D0000F0, "moneybag_geo"}
        }},
        {"group17", {
            {0x0d0000dc, "swoop_geo"},
            {0x0d000394, "scuttlebug_geo"},
            {0x0d00001c, "mr_i_iris_geo"},
            {0x0d000000, "mr_i_geo"},
            {0x0d000230, "dorrie_geo"},
            {0x0d0001a0, "snufit_geo"}
        }},
        {"group2", {
            {0x0c000240, "blargg_geo"},
            {0x0c000000, "bully_geo"},
            {0x0c000120, "bully_boss_geo"}
        }},
        {"group3", {
            {0x0c000308, "water_bomb_geo"},
            {0x0c000328, "water_bomb_shadow_geo"},
            {0x0c000000, "king_bobomb_geo"}
        }},
        {"group4", {
            {0x05008D14, "manta_seg5_geo_05008D14"},
            {0x0c00010c, "unagi_geo"},
            {0x0c000068, "sushi_geo"},
            {0x05013CB8, "whirlpool_seg5_dl_05013CB8"},
            {0x0c000000, "clam_shell_geo"}
        }},
        {"group5", {
            {0x0c000610, "pokey_head_geo"},
            {0x0c000644, "pokey_body_part_geo"},
            {0x05014630, "tweester_geo"},
            {0x0c000000, "klepto_geo"},
            {0x0c0005a8, "eyerok_left_hand_geo"},
            {0x0c0005e4, "eyerok_right_hand_geo"}
        }},
        {"group6", {
            {0x05000840, "monty_mole_hole_seg5_dl_05000840"},
            {0x0c000000, "monty_mole_geo"},
            {0x0c000110, "ukiki_geo"},
            {0x0c00036c, "fwoosh_geo"}
        }},
        {"group7", {
            {0x0c000000, "spindrift_geo"},
            {0x0c00021c, "mr_blizzard_hidden_geo"},
            {0x0c000348, "mr_blizzard_geo"},
            {0x0c000104, "penguin_geo"}
        }},
        {"group8", {
            {0x05002E00, "cap_switch_exclamation_seg5_dl_05002E00"},
            {0x0c000048, "cap_switch_geo"},
            {0x0c000030, "springboard_bottom_geo"},
            {0x05003120, "cap_switch_base_seg5_dl_05003120"}
        }},
        {"group9", {
            {0x0c000224, "boo_geo"},
            {0x0c000188, "small_key_geo"},
            {0x0c0000d8, "haunted_chair_geo"},
            {0x0c0001b4, "mad_piano_geo"},
            {0x0c000000, "bookend_part_geo"},
            {0x0c0000c0, "bookend_geo"},
            {0x0c000274, "haunted_cage_geo"}
        }}
    };

    std::unordered_map<u32, std::string> GroupAddreses = {
        {0x1279B0, "group0"},
        {0x132850, "group1"},
        {0x134a70, "group2"},
        {0x13B5D0, "group3"},
        {0x145C10, "group4"},
        {0x151B70, "group5"},
        {0x1602E0, "group6"},
        {0x1656E0, "group7"},
        {0x166BD0, "group8"},
        {0x16D5C0, "group9"},
        {0x180540, "group10"},
        {0x187FA0, "group11"},
        {0x1B9070, "group12"},
        {0x1C3DB0, "group13"},
        {0x1D7C90, "group14"},
        {0x1E4BF0, "group15"},
        {0x1E7D90, "group16"},
        {0x1F1B30, "group17"},
        {0x2008D0, "group18"},
        {0x218DA0, "group19"}
    };

    std::string GetGeoName(u32 Addr) {
        for (auto &Group : AllGroups) {
            auto ModelIt = Group.second.find(Addr);

            if (ModelIt != Group.second.end()) {
                return ModelIt->second;
            }
        }

        return "";
    }

    std::string GetGeoNameWithGroup(const std::string &GroupName, u32 Addr) {
        auto GrouptIt = AllGroups.find(GroupName);
        if (GrouptIt == AllGroups.end()) {
            return "";
        }
        auto ModelIt = GrouptIt->second.find(Addr);
        if (ModelIt == GrouptIt->second.end()) {
            return "";
        }
        return ModelIt->second;
    }

    std::string FindNearestGroup(u32 Addr) {
        std::string NearestGroup = "";
        u32 Min = UINT32_MAX;

        for (const auto &[MapAddr, GroupName] : GroupAddreses) {
            u32 Dist = (Addr > MapAddr) ? (Addr - MapAddr) : (MapAddr - Addr);
            if (Dist < Min) {
                Min = Dist;
                NearestGroup = GroupName;
            }
        }

        return NearestGroup;
    }
}

std::map<u8, std::string> LevelNames = {
    {4, "bbh"},
    {5, "ccm"},
    {6, "castle_inside"},
    {7, "hmc"},
    {8, "ssl"},
    {9, "bob"},
    {10, "sl"},
    {11, "wdw"},
    {12, "jrb"},
    {13, "thi"},
    {14, "ttc"},
    {15, "rr"},
    {16, "castle_grounds"},
    {17, "bitdw"},
    {18, "vcutm"},
    {19, "bitfs"},
    {20, "sa"},
    {21, "bits"},
    {22, "lll"},
    {23, "ddd"},
    {24, "wf"},
    {25, "ending"},
    {26, "castle_courtyard"},
    {27, "pss"},
    {28, "cotmc"},
    {29, "totwc"},
    {30, "bowser_1"},
    {31, "wmotr"},
    {33, "bowser_2"},
    {34, "bowser_3"},
    {36, "ttm"}
};

std::string GetLevelName(u16 ID) {
    if (!LevelNames.contains(ID)) {
        return std::format("ext_level_{}", ID);
    } else {
        return LevelNames[ID];
    }
}

// these are from Quad64
bool LevelScript::IsPerAreaBank0x0E(void) {
    std::vector<u8> &Data = SegmentData[0x19];
    if (Data.empty()) return false;
    if (Data.size() < 0x6000) return false;
    u32 Offset = 0x5FFC;
    return ((Data[0 + Offset] << 24 | Data[1 + Offset] << 16 | Data[2 + Offset] << 8 | Data[3 + Offset]) == 0x4BC9189A);
}
void LevelScript::SetAreaSegmented0x0E(N64Rom &Rom, u8 AreaID) {
    if (!IsPerAreaBank0x0E()) return;

    std::vector<u8> &Data = SegmentData[0x19];

    u32 Start, End;
    u32 Offset = 0x5F00 + (u32)AreaID * 0x10;
    Start = (u32)((Data[Offset] << 24) | (Data[Offset + 1 ] << 16)| (Data[Offset + 2] << 8) | Data[Offset + 3]);
    Offset += 4;
    End = (u32)((Data[Offset] << 24) | (Data[Offset + 1] << 16) | (Data[Offset + 2] << 8) | Data[Offset + 3]);

    LoadSegment(Rom, 0x0E, Start, End);
}

void LevelScript::AddDisplayList(u32 Address, u8 Area) {
    if (!ValidateMemAddr(Address)) return;

    if (CurrentActor) {
        CurrentActor->DisplayLists.push_back(Address);
    } else {
        AreaDatas[Area].DisplayLists.push_back(Address);
    }
}

std::string LvlCommandsName[] = {
    "EXECUTE",
    "EXIT_AND_EXECUTE",
    "EXIT",
    "SLEEP",
    "SLEEP_BEFORE_EXIT",
    "JUMP",
    "JUMP_LINK",
    "RETURN",
    "JUMP_LINK_PUSH_ARG",
    "JUMP_N_TIMES",
    "LOOP_BEGIN",
    "LOOP_UNTIL",
    "JUMP_IF",
    "JUMP_LINK_IF",
    "SKIP_IF",
    "SKIP",
    "SKIP_NOP",
    "CALL",
    "CALL_LOOP",
    "SET_REG",
    "PUSH_POOL",
    "POP_POOL",
    "FIXED_LOAD",
    "LOAD_RAW",
    "LOAD_MIO0",
    "LOAD_MARIO_HEAD",
    "LOAD_MIO0_TEXTURE",
    "INIT_LEVEL",
    "CLEAR_LEVEL",
    "ALLOC_LEVEL_POOL",
    "FREE_LEVEL_POOL",
    "AREA",
    "END_AREA",
    "//LOAD_MODEL_FROM_DL", // so dynos doesnt kill itself
    "LOAD_MODEL_FROM_GEO",
    "CMD23",
    "OBJECT_WITH_ACTS",
    //"OBJECT",
    "MARIO",
    "WARP_NODE",
    "PAINTING_WARP_NODE",
    "INSTANT_WARP",
    "LOAD_AREA",
    "CMD2A",
    "MARIO_POS",
    "CMD2C",
    "CMD2D",
    "TERRAIN",
    "ROOMS",
    "SHOW_DIALOG",
    "TERRAIN_TYPE",
    "NOP",
    "TRANSITION",
    "BLACKOUT",
    "GAMMA",
    "SET_BACKGROUND_MUSIC",
    "SET_MENU_MUSIC",
    "STOP_MUSIC",
    "MACRO_OBJECTS",
    "CMD3A",
    "WHIRLPOOL",
    "GET_OR_SET",
};

std::string LvlCmdExec(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define EXECUTE(seg, script, scriptEnd, entry) \
    CMD_BBH(0x00, 0x10, seg), \
    CMD_PTR(script), \
    CMD_PTR(scriptEnd), \
    CMD_PTR(entry)
    */

    s16 Segment = Rom.ReadBytes<s16>(Start + 2, false) & 0xff;
    u32 RomStart = Rom.ReadBytes<u32>(Start + 4, false);
    u32 RomEnd = Rom.ReadBytes<u32>(Start + 8, false);
    u32 ScriptEntry = Rom.ReadBytes<u32>(Start + 12, false);

    LoadSegment(Rom, Segment, RomStart, RomEnd);

    u32 Saved = Start + 0x10;
    Script.Stack.push_back(Saved);
	Script.StackTop++;
	Script.Stack.push_back(Script.StackBase);
	Script.StackTop++;
	Script.StackBase=Script.StackTop;
    Start = ScriptEntry;

    if (VerbosePrinting) printf("Jump exec to 0x%x and save 0x%x (0x%x)\n", ScriptEntry, Saved, SegmentedToROM(Saved));

    std::string OutArgs = std::format(
        "{:#x}, {:#x}, {:#x}, {:#x}",
        Segment, RomStart, RomEnd, ScriptEntry
    );

    return OutArgs;
};

std::string LvlCmdExitAndExec(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define EXIT_AND_EXECUTE(seg, script, scriptEnd, entry) \
    CMD_BBH(0x01, 0x10, seg), \
    CMD_PTR(script), \
    CMD_PTR(scriptEnd), \
    CMD_PTR(entry)
    */

    s16 Segment = Rom.ReadBytes<s16>(Start + 2, false) & 0xff;
    u32 RomStart = Rom.ReadBytes<u32>(Start + 4, false);
    u32 RomEnd = Rom.ReadBytes<u32>(Start + 8, false);
    u32 ScriptEntry = Rom.ReadBytes<u32>(Start + 12, false);

    LoadSegment(Rom, Segment, RomStart, RomEnd);

    Script.StackTop = Script.StackBase;
    Start = (ScriptEntry);

    if (VerbosePrinting) printf("Jump exec to 0x%x\n", ScriptEntry);

    std::string OutArgs = std::format(
        "{:#x}, {:#x}, {:#x}, {:#x}",
        Segment, RomStart, RomEnd, ScriptEntry
    );

    return "";
};

std::string LvlCmdExit(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define EXIT() \
    CMD_BBH(0x02, 0x04, 0x0000)
    */

    Script.StackTop = Script.StackBase;
    Script.StackTop--;
    Script.StackBase = Script.Stack[Script.StackTop];
    Script.Stack.pop_back();
    Script.StackTop--;
    Start = Script.Stack[Script.StackTop];
    Script.Stack.pop_back();

    if (VerbosePrinting) printf("Exit to 0x%x\n", Start);
    if (Script.FoundLevel) Start = UINT32_MAX;

    return "";
}

std::string LvlCmdSleepBeforeExit(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define SLEEP_BEFORE_EXIT(frames) \
    CMD_BBH(0x04, 0x04, frames)
    */

    s16 Frames = Rom.ReadBytes<s16>(Start + 2, false);

    std::string OutArgs = std::format(
        "{}",
        Frames
    );

    return OutArgs;
};

std::string LvlCmdJump(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define JUMP(target) \
    CMD_BBH(0x05, 0x08, 0x0000), \
    CMD_PTR(target)
    */

    u32 Target = Rom.ReadBytes<u32>(Start + 4, false);

    std::string TargetName = GetLabelFromMap(Target);
    std::string OutArgs = std::format(
        "{}",
        TargetName
    );

    Start = Target;

    if (VerbosePrinting) printf("Jump no link to 0x%x (0x%x)\n", Target, Start);

    return OutArgs;
};


std::string LvlCmdJumpLink(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define JUMP_LINK(target) \
    CMD_BBH(0x06, 0x08, 0x0000), \
    CMD_PTR(target)
    */

    u32 Target = Rom.ReadBytes<u32>(Start + 4, false);

    std::string TargetName = GetLabelFromMap(Target);
    std::string OutArgs = std::format(
        "{}",
        TargetName
    );

    u32 Saved = Start + 0x08;
    if ((Target >> 24) == 0) {
        Start += 8;
    } else {
        Script.Stack.push_back(Saved);
        Script.StackTop++;
        Start = Target;
    }

    if (VerbosePrinting) printf("Push & Jump to 0x%x and save 0x%x (0x%x)\n", Target, Saved, SegmentedToROM(Saved));

    return OutArgs;
};

std::string LvlCmdReturn(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define RETURN() \
    CMD_BBH(0x07, 0x04, 0x0000)
    */

    Script.StackTop -= 1;
    Start = Script.Stack[Script.StackTop];
    Script.Stack.pop_back();

    if (VerbosePrinting) printf("Return from Jump to 0x%x\n", Start);

    return "";
};

std::string LvlCmdJumpIf(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define JUMP_IF(op, arg, target) \
    CMD_BBBB(0x0C, 0x0C, op, 0x00), \
    CMD_W(arg), \
    CMD_PTR(target)
    */

    u8 Op = Rom.ReadBytes<u8>(Start + 2, false);
    u32 Arg = Rom.ReadBytes<u32>(Start + 4, false);
    u32 Target = Rom.ReadBytes<u32>(Start + 8, false);

    if (Arg == Script.LevelID) {
        Start = Target;
        Script.FoundLevel = true;
        printf("Found %s at address 0x%x\n", Script.Name.c_str(), Target);
    }

    std::string OutArgs = std::format(
        "{}, {}, {:#x}",
        Op, Arg, Target
    );

    return OutArgs;
};

std::string LvlCmdCallAsm(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define CALL(arg, func) \
    CMD_BBH(0x11, 0x08, arg), \
    CMD_PTR(func)
    */

    s16 Arg = Rom.ReadBytes<s16>(Start + 2, false);
    u32 Func = Rom.ReadBytes<u32>(Start + 4, false);

    std::string FuncName = GetLabelFromMap(Func);
    if (Script.FoundLevel) FuncName = "lvl_init_or_update";
    std::string OutArgs = std::format(
        "{}, /* Func */ {}",
        Arg, FuncName
    );

    return OutArgs;
};

std::string LvlCmdCallLoop(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define CALL_LOOP(arg, func) \
    CMD_BBH(0x12, 0x08, arg), \
    CMD_PTR(func)
    */

    return LvlCmdCallAsm(Rom, Script, Start);
};

std::string LvlCmdLoadRaw(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define LOAD_RAW(seg, romStart, romEnd) \
    CMD_BBH(0x17, 0x0C, seg), \
    CMD_PTR(romStart), \
    CMD_PTR(romEnd)
    */

    s16 Segment = Rom.ReadBytes<s16>(Start + 2, false) & 0xff;
    u32 RomStart = Rom.ReadBytes<u32>(Start + 4, false);
    u32 RomEnd = Rom.ReadBytes<u32>(Start + 8, false);

    LoadSegment(Rom, Segment, RomStart, RomEnd);

    std::string OutArgs = std::format(
        "{:#x}, {:#x}, {:#x}",
        Segment, RomStart, RomEnd
    );

    return OutArgs;
};

std::string LvlCmdLoadMio0(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define LOAD_MIO0(seg, romStart, romEnd) \
    CMD_BBH(0x18, 0x0C, seg), \
    CMD_PTR(romStart), \
    CMD_PTR(romEnd)
    */

    s16 Segment = Rom.ReadBytes<s16>(Start + 2, false) & 0xff;
    u32 RomStart = Rom.ReadBytes<u32>(Start + 4, false);
    u32 RomEnd = Rom.ReadBytes<u32>(Start + 8, false);

    LoadSegment(Rom, Segment, RomStart, RomEnd, true);

    std::string OutArgs = std::format(
        "{:#x}, {:#x}, {:#x}",
        Segment, RomStart, RomEnd
    );

    return OutArgs;
};

std::string LvlCmdLoadMio0Tex(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define LOAD_MIO0_TEXTURE(seg, romStart, romEnd) \
    CMD_BBH(0x1A, 0x0C, seg), \
    CMD_PTR(romStart), \
    CMD_PTR(romEnd)
    */

    return LvlCmdLoadMio0(Rom, Script, Start);
};

std::string LvlCmdInitLevel(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define INIT_LEVEL() \
    CMD_BBH(0x1B, 0x04, 0x0000)
    */

    return "";
};

std::string LvlCmdClearLevel(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define CLEAR_LEVEL() \
    CMD_BBH(0x1C, 0x04, 0x0000)
    */

    return "";
};

std::string LvlCmdAllocLevelPool(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define ALLOC_LEVEL_POOL() \
    CMD_BBH(0x1D, 0x04, 0x0000
    */

    return "";
};

std::string LvlCmdFreeLevelPool(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define FREE_LEVEL_POOL() \
    CMD_BBH(0x1E, 0x04, 0x0000)
    */

    return "";
};

std::string LvlCmdStartArea(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define AREA(index, geo) \
    CMD_BBBB(0x1F, 0x08, index, 0), \
    CMD_PTR(geo)    
    */

    u8 Index = Rom.ReadBytes<u8>(Start + 2, false);
    u32 Geo = Rom.ReadBytes<u32>(Start + 4, false);

    std::string AreaGeoName = std::format(
        "{}_area_{}_geo_{:x}",
        Script.Name, Index, Geo
    );
    std::string OutArgs = std::format(
        "/* Index */ {}, /* Geo */ {}",
        Index, AreaGeoName
    );

    if (Index > MAX_AREA) {
        printf("Area Index for command is bigger than MAX_AREA\n");
        Index &= (MAX_AREA-1);
    }

    Script.AreaDatas[Index].GeoLayout = Geo;
    if (Script.FoundLevel) Script.Areas.push_back(Index);
    Script.CurrArea = Index;

    return OutArgs;
};

std::string LvlCmdEndArea(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define END_AREA() \
    CMD_BBH(0x20, 0x04, 0x0000)
    */

    return "";
};

std::string LvlCmdLoadModelFromDL(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define LOAD_MODEL_FROM_DL(model, dl, layer) \
    CMD_BBH(0x21, 0x08, ((layer << 12) | model)), \
    CMD_PTR(dl)
    */

    s16 Data = Rom.ReadBytes<s16>(Start + 2, false);
    s16 ModelID = Data & 0xFFF;
    s16 Layer = (Data >> 12) & 0xF;
    u32 DisplayList = Rom.ReadBytes<u32>(Start + 4, false);
    std::string DisplayListName;

    if (DisplayList) {
        Actor NewActor;
        NewActor.IsDL = true;
        DisplayListName = NewActor.Name = std::format("{}_actor_dl_{:x}", Script.Name, DisplayList);
        NewActor.Addr = DisplayList;
        Script.Actors.push_back(NewActor);
    }

    std::string OutArgs = std::format(
        "{:#x}, {}, {:#x}",
        ModelID, DisplayListName, Layer
    );

    return OutArgs;
};

std::string LvlCmdLoadModelFromGeo(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define LOAD_MODEL_FROM_GEO(model, geo) \
    CMD_BBH(0x22, 0x08, model), \
    CMD_PTR(geo)
    */

    s16 ModelID = Rom.ReadBytes<s16>(Start + 2, false);
    u32 Geo = Rom.ReadBytes<u32>(Start + 4, false);

    u8 GeoBank = Geo >> 24;
    std::string GeoName = GetLabelFromMap(Geo);

    if ((!GeoName.starts_with("Custom_") || GeoBank != 0x19) && (GameType.IsBinary())) {
        if (GeoBank == 0x0D || GeoBank == 0x0C) {
            std::string Group = ActorGroup::FindNearestGroup(SegmentOffsets[GeoBank][0]);
            const std::string BuiltinName = ActorGroup::GetGeoNameWithGroup(Group, Geo);
            if (BuiltinName != "") {
                GeoName = BuiltinName;
            }
        } else {
            if (GeoBank != 0x0F && GeoBank != 0x00 && GeoBank != 0x03 && GeoBank != 0x12) {
                const std::string BuiltinName = ActorGroup::GetGeoName(Geo);
                if (BuiltinName != "") {
                    GeoName = BuiltinName;
                }
            }
        }
    }

    if (Geo) {
        Actor NewActor;
        NewActor.IsDL = false;
        bool AddActor = true;
        
        if (GeoName.starts_with("Custom_")) {
            GeoName = std::format("{}_actor_geo_{:x}", Script.Name, Geo);
            if (ActorsExport == "vanilla") {
                AddActor = false;
            }
        } else {
            if (ActorsExport == "custom") {
                AddActor = false;
            }
        }
        
        NewActor.Name = GeoName;
        NewActor.Addr = Geo;
        if (AddActor && (Geo >> 24) != 0x14) Script.Actors.push_back(NewActor);
    }

    std::string OutArgs = std::format(
        "{:#x}, {}",
        ModelID, GeoName
    );

    return OutArgs;
};

std::string LvlCmdPlaceObject(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define OBJECT_WITH_ACTS(model, posX, posY, posZ, angleX, angleY, angleZ, bhvParam, bhv, acts) \
    CMD_BBBB(0x24, 0x18, acts, model), \
    CMD_HHHHHH(posX, posY, posZ, angleX, angleY, angleZ), \
    CMD_W(bhvParam), \
    CMD_PTR(bhv)
    */

    u8 Acts = Rom.ReadBytes<u8>(Start + 2, false);
    u8 ModelID = Rom.ReadBytes<u8>(Start + 3, false);
    s16 PosX = Rom.ReadBytes<s16>(Start + 4, false);
    s16 PosY = Rom.ReadBytes<s16>(Start + 6, false);
    s16 PosZ = Rom.ReadBytes<s16>(Start + 8, false);
    s16 AngleX = Rom.ReadBytes<s16>(Start + 10, false);
    s16 AngleY = Rom.ReadBytes<s16>(Start + 12, false);
    s16 AngleZ = Rom.ReadBytes<s16>(Start + 14, false);
    u32 BhvParam = Rom.ReadBytes<u32>(Start + 16, false);
    u32 Bhv = Rom.ReadBytes<u32>(Start + 20, false);

    std::string BhvName = GetLabelFromMap(Bhv);
    if (GameType.IsOldBinary()) {
        if (BhvName == "RM_Scroll_Texture" || BhvName == "editor_Scroll_Texture" || BhvName == "editor_Scroll_Texture2" || (BhvName == "bhvBetaHoldableObject" && GameType.GetID() == GT_EDITOR)) {
            ScrollTexture Scroll;

            if (BhvName == "bhvBetaHoldableObject") BhvName = "editor_Scroll_Texture2";
            
            if (BhvName == "RM_Scroll_Texture") {
                Scroll = ConvertRMTexScrolls(Script, BhvParam, PosX, PosY, PosZ);
            } else {
                Scroll = ConvertEditorTexScrolls(Script, BhvParam, PosX, PosY, PosZ, BhvName, Rom);
            }

            Script.ScrollTargets.push_back(Scroll);

            std::string OutArgs = std::format(
                "/* Model */ 0x0, /* Speed */ {}, /* Axis */ {}, /* VCount */ {}, 0, /* Type */ {}, /* Cycle */ {}, /* Index */ {:#x}, {}, /* Act */ {}",
                Scroll.Speed, Scroll.Axis, Scroll.NumVtx,  Scroll.Type, Scroll.Cycle, Scroll.Id, BhvName, Acts
            );
            return OutArgs;
        }
    }

    Script.Behaviors.push_back(Bhv);

    std::string OutArgs = std::format(
        "/* Model */ {:#x}, /* Pos */ {}, {}, {}, /* Angle */ {}, {}, {}, /* Param */ {:#x}, /* Behavior */ {}, /* Act */ {}",
        ModelID, PosX, PosY, PosZ, AngleX, AngleY, AngleZ, BhvParam, BhvName, Acts
    );

    return OutArgs;
};

std::string LvlCmdPlaceMario(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define MARIO(model, bhvArg, bhv) \
    CMD_BBBB(0x25, 0x0C, 0x00, model), \
    CMD_W(bhvArg), \
    CMD_PTR(bhv)
    */
    
    u8 ModelID = Rom.ReadBytes<u8>(Start + 3, false);
    u32 BhvArg = Rom.ReadBytes<u32>(Start + 4, false);
    u32 Bhv = Rom.ReadBytes<u32>(Start + 8, false);

    std::string BhvName = "bhvMario";
    std::string OutArgs = std::format(
        "/* Model */ {:#x}, /* Param */ {:#x}, /* Behavior */ {}",
        ModelID, BhvArg, BhvName
    );

    return OutArgs;
};

std::string LvlCmdWarpNode(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define WARP_NODE(id, destLevel, destArea, destNode, flags) \
    CMD_BBBB(0x26, 0x08, id, destLevel), \
    CMD_BBBB(destArea, destNode, flags, 0x00)
    */

    u8 Id = Rom.ReadBytes<u8>(Start + 2, false);
    u8 DestLevel = Rom.ReadBytes<u8>(Start + 3, false);
    u8 DestArea = Rom.ReadBytes<u8>(Start + 4, false);
    u8 DestNode = Rom.ReadBytes<u8>(Start + 5, false);
    u8 Flags = Rom.ReadBytes<u8>(Start + 6, false);

    std::string OutArgs = std::format(
        "{:#x}, {:#x}, {:#x}, {:#x}, {:#x}",
        Id, DestLevel, DestArea, DestNode, Flags
    );

    return OutArgs;
};


std::string LvlCmdPlaceInstaWarp(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define INSTANT_WARP(index, destArea, displaceX, displaceY, displaceZ) \
    CMD_BBBB(0x28, 0x0C, index, destArea), \
    CMD_HH(displaceX, displaceY), \
    CMD_HH(displaceZ, 0x0000)
    */

    u8 Id = Rom.ReadBytes<u8>(Start + 2, false);
    u8 DestArea = Rom.ReadBytes<u8>(Start + 3, false);
    s16 DispX = Rom.ReadBytes<s16>(Start + 4, false);
    s16 DispY = Rom.ReadBytes<s16>(Start + 6, false);
    s16 DispZ = Rom.ReadBytes<s16>(Start + 8, false);

    std::string OutArgs = std::format(
        "{}, {}, {}, {}, {}",
        Id, DestArea, DispX, DispY, DispZ
    );

    return OutArgs;
};

std::string LvlCmdSetMarioPos(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define MARIO_POS(area, yaw, posX, posY, posZ) \
    CMD_BBBB(0x2B, 0x0C, area, 0x00), \
    CMD_HH(yaw, posX), \
    CMD_HH(posY, posZ)
    */

    u8 Area = Rom.ReadBytes<u8>(Start + 2, false);
    s16 Yaw = Rom.ReadBytes<s16>(Start + 4, false);
    s16 PosX = Rom.ReadBytes<s16>(Start + 6, false);
    s16 PosY = Rom.ReadBytes<s16>(Start + 8, false);
    s16 PosZ = Rom.ReadBytes<s16>(Start + 10, false);

    std::string OutArgs = std::format(
        "{}, {}, {}, {}, {}",
        Area, Yaw, PosX, PosY, PosZ
    );

    return OutArgs;
};

std::string LvlCmdSetTerrain(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define TERRAIN(terrainData) \
    CMD_BBH(0x2E, 0x08, 0x0000), \
    CMD_PTR(terrainData)
    */

    u32 Collision = Rom.ReadBytes<u32>(Start + 4, false);

    std::string AreaColName = std::format(
        "{}_area_{}_collision_{:#x}",
        Script.Name, Script.CurrArea, Collision
    );
    std::string OutArgs = std::format(
        "/* Col */ {}",
        AreaColName
    );

    Script.AreaDatas[Script.CurrArea].Collision = Collision;

    return OutArgs;
};

std::string LvlCmdShowDialog(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define SHOW_DIALOG(index, dialogId) \
    CMD_BBBB(0x30, 0x04, index, dialogId)
    */

    u8 Index = Rom.ReadBytes<u8>(Start + 2, false);
    u8 DialogID = Rom.ReadBytes<u8>(Start + 3, false);

    std::string OutArgs = std::format(
        "{}, {}",
        Index, DialogID
    );

    return OutArgs;
};

std::string LvlCmdSetTerrainType(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define TERRAIN_TYPE(terrainType) \
    CMD_BBH(0x31, 0x04, terrainType)
    */

    s16 Type = Rom.ReadBytes<s16>(Start + 2, false);

    std::string OutArgs = std::format(
        "{:#x}",
        Type
    );

    return OutArgs;
};

std::string LvlCmdSetMusic(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define SET_BACKGROUND_MUSIC(settingsPreset, seq) \
    CMD_BBH(0x36, 0x08, settingsPreset), \
    CMD_HH(seq, 0x0000)
    */

    s16 Preset = Rom.ReadBytes<s16>(Start + 2, false);
    s16 Sequence = Rom.ReadBytes<s16>(Start + 4, false);

    if (Sequence) {
        if (!SequenceMusics.count(Sequence)) {
            SequenceMusics.insert(Sequence);
        }
    }

    std::string OutArgs = std::format(
        "{:#x}, {:#x}",
        Preset, Sequence
    );

    return OutArgs;
};

std::string LvlCmdStopMusic(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define STOP_MUSIC(fadeOutTime) \
    CMD_BBH(0x38, 0x04, fadeOutTime)
    */

    s16 FadeOutTime = Rom.ReadBytes<s16>(Start + 2, false);

    std::string OutArgs = std::format(
        "{:#x}",
        FadeOutTime
    );

    return OutArgs;
};

std::string LvlCmdMacroObjects(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    /*
    #define MACRO_OBJECTS(objList) \
    CMD_BBH(0x39, 0x08, 0x0000), \
    CMD_PTR(objList)
    */

    u32 MacroObjList = Rom.ReadBytes<u32>(Start + 4, false);

    std::string OutArgs = std::format(
        "{}_{}_macro_objects_{:#x}",
        Script.Name, Script.CurrArea, MacroObjList
    );

    Script.AreaDatas[Script.CurrArea].MacroObjects = MacroObjList;

    return OutArgs;
};

std::string LvlCmdStub(N64Rom &Rom, LevelScript &Script, u32 &Start) {
    return "";
};

std::string (*LvlCommandsFunctions[])(N64Rom &Rom, LevelScript &Script, u32 &Start) = {
    LvlCmdExec,             LvlCmdExitAndExec,      LvlCmdExit,             LvlCmdSleepBeforeExit,
    LvlCmdSleepBeforeExit,  LvlCmdJump,             LvlCmdJumpLink,         LvlCmdReturn,
    (LvlCmdStub),           (LvlCmdStub),           (LvlCmdStub),           (LvlCmdStub),
    LvlCmdJumpIf,           (LvlCmdStub),           (LvlCmdStub),           (LvlCmdStub),
    (LvlCmdStub),           LvlCmdCallAsm,          LvlCmdCallLoop,         (LvlCmdStub),
    (LvlCmdStub),           (LvlCmdStub),           (LvlCmdStub),           LvlCmdLoadRaw,
    LvlCmdLoadMio0,         (LvlCmdStub),           LvlCmdLoadMio0Tex,      LvlCmdInitLevel,
    LvlCmdClearLevel,       LvlCmdAllocLevelPool,   LvlCmdFreeLevelPool,    LvlCmdStartArea,
    LvlCmdEndArea,          LvlCmdLoadModelFromDL,  LvlCmdLoadModelFromGeo, (LvlCmdStub),
    LvlCmdPlaceObject,      LvlCmdPlaceMario,       LvlCmdWarpNode,         (LvlCmdStub),
    LvlCmdPlaceInstaWarp,   (LvlCmdStub),           (LvlCmdStub),           LvlCmdSetMarioPos,
    (LvlCmdStub),           (LvlCmdStub),           LvlCmdSetTerrain,       (LvlCmdStub),
    LvlCmdShowDialog,       LvlCmdSetTerrainType,   (LvlCmdStub),           (LvlCmdStub),
    (LvlCmdStub),           (LvlCmdStub),           LvlCmdSetMusic,         (LvlCmdStub),
    LvlCmdStopMusic,        LvlCmdMacroObjects,     (LvlCmdStub),           (LvlCmdStub),
    (LvlCmdStub),
};

bool IsJumpLvlCmd(u8 Cmd) {
    return Cmd == 0x00 || Cmd == 0x01 || Cmd == 0x02 || Cmd == 0x05 || Cmd == 0x06 || Cmd == 0x07 || Cmd == 0x0C || Cmd == 0x10 || Cmd == 0x09;
}

void ExportAreas(N64Rom &Rom, LevelScript &Script, const std::string &LvlName) {
    std::string AreasPath = "output/levels/"+LvlName+"/areas";
    fs::create_directories(AreasPath);
    for (auto &I : Script.Areas) {
        u32 GeoSegAddr = Script.AreaDatas[I].GeoLayout;
        u32 ColSegAddr = Script.AreaDatas[I].Collision;
        u32 MacroSegAddr = Script.AreaDatas[I].MacroObjects;

        Script.SetAreaSegmented0x0E(Rom, I);

        std::string AreaStrNum = AreasPath+"/"+std::to_string(I);
        fs::create_directories(AreaStrNum);
        std::string GeoDumpPath = AreaStrNum + "/geo.inc.c";
        ExportGeolayout(Rom, I, LvlName, GeoSegAddr, GeoSegAddr, Script, GeoDumpPath.c_str());
        std::string ColDumpPath = AreaStrNum + "/collision.inc.c";
        ExportCollision(Rom, I, LvlName, ColSegAddr, Script, ColDumpPath.c_str());
        if (GameType.IsOldBinary()) {
            std::string MovTexDumpPath = AreaStrNum + "/movtext.inc.c";
            ExportMovTex(Rom, I, LvlName, Script, MovTexDumpPath.c_str());
        }
        std::string ModelDumpPath = AreaStrNum + "/model.inc.c";
        ExportModels(Rom, Script, LvlName, I, ModelDumpPath.c_str());
        std::string MacroDumpPath = AreaStrNum + "/macro.inc.c";
        ExportMacroObjects(Rom, I, LvlName, MacroSegAddr, MacroDumpPath.c_str());

        printf("%s Area %u done\n", LvlName.c_str(), I);
    }
}

void ExportLevel(N64Rom &Rom, u8 LvlID) {
    std::string LvlName = GetLevelName(LvlID);
    fs::create_directories("output/levels");
    fs::create_directories("output/levels/" + LvlName);
    std::string ScriptPath = "output/levels/" + LvlName + "/script.c";
    FILE *ScriptDump = fopen(ScriptPath.c_str(), "w");
    LevelScript Script;
    Script.LevelID = LvlID;
    Script.Name = LvlName;

    for (s32 I = 0; I < MAX_SEGMENT; I++) {
        if (I == 2) continue;

        auto &SegOff = SegmentOffsets[I];
        SegOff[0] = 0;
        SegOff[1] = 0;

        auto &SegData = SegmentData[I];
        SegData.clear();
    }

    //b'\x1b\x04\x00\x00\x03\x04\x00\x024\x04\x00\x00'
    u32 Entry = 0;
    if (!FoundScriptEntry) {
        const u8 Pattern[] = {0x1b, 0x04, 0x00, 0x00, 0x03, 0x04, 0x00, 0x02, 0x34, 0x04, 0x00, 0x00};
        size_t PatternLen = sizeof(Pattern);
        u8 *Start = Rom.Data;
        u8 *End = Rom.Data + Rom.Size;
        u8 *Found = std::search(Start, End, Pattern, Pattern + PatternLen);
        if (Found != End) {
            Entry = (u32)(Found - Start);
            if (!FoundScriptEntry) printf("Script Entry found at address: 0x%x\n", Entry);
            FoundScriptEntry = Entry;

            FindAndLoadSegment2(Rom);
        } else {
            printf("No Script Entries could be found.\n");
            exit(1);
        }
        SegmentOffsets[0x10][0] = Entry;
    } else {
        Entry = SegmentOffsets[0x10][0] = FoundScriptEntry;
    }

    bool ForceDontPrint = false;
    bool BinaryHack = GameType.IsBinary();

    auto ShouldPrintCmd = [&](u8 Cmd) {
        if (!Script.FoundLevel) {
            return false;
        }

        if ((Entry >> 24) == 0x15 && ForceDontPrint && BinaryHack) {
            return false;
        }

        if (IsJumpLvlCmd(Cmd)) {
            if (BinaryHack) {
                if (Cmd == 0x06) {
                    u32 Target = Rom.ReadBytes<u32>(Entry + 4, false);
                    if ((Target >> 24) == 0x15) {
                        ForceDontPrint = true;
                        return true;
                    }
                }

                if (Cmd == 0x07) {
                    ForceDontPrint = false;
                }
            }

            if (Cmd != 0x02) {
                return false;
            }
        }
        return true;
    };

    std::string EntryName = "level_" + LvlName + "_entry";
    fprintf(ScriptDump, "const LevelScript %s[] = {\n", EntryName.c_str());
    printf("Exporting %s\n", LvlName.c_str());
    while (true) {
        u8 Cmd = Rom.ReadBytes<u8>(Entry, false);
        if (Cmd > 60) {
            printf("Unknown LevelScript Command 0x%x at address 0x%x\n", Cmd, Entry);
            break;
        }
        //printf("0x%x CMD: 0x%x\n", Entry, Cmd);
        u8 Len = Rom.ReadBytes<u8>(Entry + 1, false);
        bool ShouldPrint = ShouldPrintCmd(Cmd);
        if (ShouldPrint) fprintf(ScriptDump, "    %s(", LvlCommandsName[Cmd].c_str());
        if (LvlCommandsFunctions[Cmd]) {
            std::string Args = LvlCommandsFunctions[Cmd](Rom, Script, Entry);
            if (ShouldPrint) fprintf(ScriptDump, "%s", Args.c_str());
        } else {
            break;
        }
        if (ShouldPrint) fprintf(ScriptDump, "),\n");
        if (Entry == UINT32_MAX) {
            if (VerbosePrinting) printf("Exit Level\n");
            break;
        }
        if ((Cmd == 0x0C && !Script.FoundLevel) || (Cmd != 0x06 && Cmd != 0x02 && Cmd != 0x07 && Cmd != 0x0C && Cmd != 0x00 && Cmd != 0x01 && Cmd != 0x05)) {
            Entry += Len;
        }
    }
    fprintf(ScriptDump, "};\n");
    fclose(ScriptDump);

    ExportAreas(Rom, Script, LvlName);
    ResolveScrollTargets(Script);
    
    if (ActorsExport != "none") ExportActors(Rom, Script);
    if (BehaviorsExport) ExportBehaviorScripts(Rom, Script);
}
