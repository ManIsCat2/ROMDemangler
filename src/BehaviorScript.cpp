#include "BehaviorScript.h"
#include "LevelScript.h"
#include "Memory.h"
#include <string>

std::map<u32, bool> ProcessedBhvs = {};

std::string BhvCommandsName[] = {
    "BEGIN", "DELAY", "CALL", "RETURN", "GOTO", "BEGIN_REPEAT",
    "END_REPEAT", "END_REPEAT_CONTINUE", "BEGIN_LOOP", "END_LOOP",
    "BREAK", "BREAK_UNUSED", "CALL_NATIVE", "ADD_FLOAT", "SET_FLOAT",
    "ADD_INT", "SET_INT", "OR_INT", "BIT_CLEAR", "SET_INT_RAND_RSHIFT",
    "SET_RANDOM_FLOAT", "SET_RANDOM_INT", "ADD_RANDOM_FLOAT", "ADD_INT_RAND_RSHIFT",
    "CMD_NOP_1", "CMD_NOP_2", "CMD_NOP_3", "SET_MODEL", "SPAWN_CHILD",
    "DEACTIVATE", "DROP_TO_FLOOR", "SUM_FLOAT", "SUM_INT",
    "BILLBOARD", "HIDE", "SET_HITBOX", "CMD_NOP_4", "DELAY_VAR",
    "BEGIN_REPEAT_UNUSED", "LOAD_ANIMATIONS", "ANIMATE", "SPAWN_CHILD_WITH_PARAM",
    "LOAD_COLLISION_DATA", "SET_HITBOX_WITH_OFFSET", "SPAWN_OBJ", "SET_HOME",
    "SET_HURTBOX", "SET_INTERACT_TYPE", "SET_OBJ_PHYSICS", "SET_INTERACT_SUBTYPE",
    "SCALE", "PARENT_BIT_CLEAR", "ANIMATE_TEXTURE", "DISABLE_RENDERING",
    "SET_INT_UNUSED", "SPAWN_WATER_DROPLET"
};

std::string BhvCmdBegin(N64Rom &Rom, u32 &Start) {
    /* Defines the start of the behavior script as well as the object list the object belongs to.
    // Has some special behavior for certain objects.
    #define BEGIN(objList) \
        BC_BB(0x00, objList) */
    u8 ObjList = Rom.ReadBytes<u8>(Start + 1);
    return std::format("{:#x}", ObjList);
}

std::string BhvCmdDelay(N64Rom &Rom, u32 &Start) {
    /* Delays the behavior script for a certain number of frames.
    #define DELAY(num) \
        BC_B0H(0x01, num) */
    s16 Num = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{}", Num);
}

std::string BhvCmdCall(N64Rom &Rom, u32 &Start) {
    /* Jumps to a new behavior command and stores the return address in the object's stack.
    #define CALL(addr) \
        BC_B(0x02), \
        BC_PTR(addr) */
    u32 Addr = Rom.ReadBytes<u32>(Start + 4);
    std::string BhvName = GetLabelFromMap(Addr);
    return std::format("{}", BhvName);
}

std::string BhvCmdReturn(N64Rom &Rom, u32 &Start) {
    /* Jumps back to the behavior command stored in the object's stack.
    #define RETURN() \
        BC_B(0x03) */
    return "";
}

std::string BhvCmdGoto(N64Rom &Rom, u32 &Start) {
    /* Jumps to a new behavior script without saving anything.
    #define GOTO(addr) \
        BC_B(0x04), \
        BC_PTR(addr) */
    u32 Addr = Rom.ReadBytes<u32>(Start + 4);
    std::string BhvName = GetLabelFromMap(Addr);
    return std::format("{}", BhvName);
}

std::string BhvCmdBeginRepeat(N64Rom &Rom, u32 &Start) {
    /* Marks the start of a loop that will repeat a certain number of times.
    #define BEGIN_REPEAT(count) \
        BC_B0H(0x05, count) */
    s16 Count = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{}", Count);
}

std::string BhvCmdEndRepeat(N64Rom &Rom, u32 &Start) {
    /* Marks the end of a repeating loop.
    #define END_REPEAT() \
        BC_B(0x06) */
    return "";
}

std::string BhvCmdEndRepeatContinue(N64Rom &Rom, u32 &Start) {
    /* Also marks the end of a repeating loop, but continues executing commands following the loop on the same frame.
    #define END_REPEAT_CONTINUE() \
        BC_B(0x07) */
    return "";
}

std::string BhvCmdBeginLoop(N64Rom &Rom, u32 &Start) {
    /* Marks the beginning of an infinite loop.
    #define BEGIN_LOOP() \
        BC_B(0x08) */
    return "";
}

std::string BhvCmdEndLoop(N64Rom &Rom, u32 &Start) {
    /* Marks the end of an infinite loop.
    #define END_LOOP() \
        BC_B(0x09) */
    return "";
}

std::string BhvCmdBreak(N64Rom &Rom, u32 &Start) {
    /* Exits the behavior script.
    // Often used to end behavior scripts that do not contain an infinite loop.
    #define BREAK() \
        BC_B(0x0A) */
    return "";
}

std::string BhvCmdBreakUnused(N64Rom &Rom, u32 &Start) {
    /* Exits the behavior script, unused.
    #define BREAK_UNUSED() \
        BC_B(0x0B) */
    return "";
}

std::string BhvCmdCallNative(N64Rom &Rom, u32 &Start) {
    /* Executes a native game function.
    #define CALL_NATIVE(func) \
        BC_B(0x0C), \
        BC_PTR(func) */
    u32 Func = Rom.ReadBytes<u32>(Start + 4);
    std::string FuncName = GetLabelFromMap(Func);
    return std::format("{}", FuncName);
}

std::string BhvCmdAddFloat(N64Rom &Rom, u32 &Start) {
    /* Adds a float to the specified field.
    #define ADD_FLOAT(field, value) \
        BC_BBH(0x0D, field, value) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Value = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{:#x}, {}", Field, Value);
}

std::string BhvCmdSetFloat(N64Rom &Rom, u32 &Start) {
    /* Sets the specified field to a float.
    #define SET_FLOAT(field, value) \
        BC_BBH(0x0E, field, value) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Value = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{:#x}, {}", Field, Value);
}

std::string BhvCmdAddInt(N64Rom &Rom, u32 &Start) {
    /* Adds an integer to the specified field.
    #define ADD_INT(field, value) \
        BC_BBH(0x0F, field, value) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Value = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{:#x}, {}", Field, Value);
}

std::string BhvCmdSetInt(N64Rom &Rom, u32 &Start) {
    /* Sets the specified field to an integer.
    #define SET_INT(field, value) \
        BC_BBH(0x10, field, value) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Value = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{:#x}, {}", Field, Value);
}

std::string BhvCmdOrInt(N64Rom &Rom, u32 &Start) {
    /* Performs a bitwise OR with the specified field and the given integer.
    // Usually used to set an object's flags.
    #define OR_INT(field, value) \
        BC_BBH(0x11, field, value) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Value = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{:#x}, {}", Field, Value);
}

std::string BhvCmdBitClear(N64Rom &Rom, u32 &Start) {
    /* Performs a bit clear with the specified short. Unused in favor of the 32-bit version.
    #define BIT_CLEAR(field, value) \
        BC_BBH(0x12, field, value) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Value = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{:#x}, {}", Field, Value);
}

std::string BhvCmdSetIntRandRshift(N64Rom &Rom, u32 &Start) {
    /* Gets a random short, right shifts it the specified amount and adds min to it, then sets the specified field to that value.
    #define SET_INT_RAND_RSHIFT(field, min, rshift) \
        BC_BBH(0x13, field, min), \
        BC_H(rshift) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Min = Rom.ReadBytes<s16>(Start + 2);
    s16 Rshift = Rom.ReadBytes<s16>(Start + 4);
    return std::format("{:#x}, {}, {}", Field, Min, Rshift);
}

std::string BhvCmdSetRandomFloat(N64Rom &Rom, u32 &Start) {
    /* Sets the specified field to a random float in the given range.
    #define SET_RANDOM_FLOAT(field, min, range) \
        BC_BBH(0x14, field, min), \
        BC_H(range) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Min = Rom.ReadBytes<s16>(Start + 2);
    s16 Range = Rom.ReadBytes<s16>(Start + 4);
    return std::format("{:#x}, {}, {}", Field, Min, Range);
}

std::string BhvCmdSetRandomInt(N64Rom &Rom, u32 &Start) {
    /* Sets the specified field to a random integer in the given range.
    #define SET_RANDOM_INT(field, min, range) \
        BC_BBH(0x15, field, min), \
        BC_H(range) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Min = Rom.ReadBytes<s16>(Start + 2);
    s16 Range = Rom.ReadBytes<s16>(Start + 4);
    return std::format("{:#x}, {}, {}", Field, Min, Range);
}

std::string BhvCmdAddRandomFloat(N64Rom &Rom, u32 &Start) {
    /* Adds a random float in the given range to the specified field.
    #define ADD_RANDOM_FLOAT(field, min, range) \
        BC_BBH(0x16, field, min), \
        BC_H(range) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Min = Rom.ReadBytes<s16>(Start + 2);
    s16 Range = Rom.ReadBytes<s16>(Start + 4);
    return std::format("{:#x}, {}, {}", Field, Min, Range);
}

std::string BhvCmdAddIntRandRshift(N64Rom &Rom, u32 &Start) {
    /* Gets a random short, right shifts it the specified amount and adds min to it, then adds the value to the specified field. Unused.
    #define ADD_INT_RAND_RSHIFT(field, min, rshift) \
        BC_BBH(0x17, field, min), \
        BC_H(rshift) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Min = Rom.ReadBytes<s16>(Start + 2);
    s16 Rshift = Rom.ReadBytes<s16>(Start + 4);
    return std::format("{:#x}, {}, {}", Field, Min, Rshift);
}

std::string BhvCmdNop1(N64Rom &Rom, u32 &Start) {
    /* No operation. Unused.
    #define CMD_NOP_1(field) \
        BC_BB(0x18, field) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    return std::format("{:#x}", Field);
}

std::string BhvCmdNop2(N64Rom &Rom, u32 &Start) {
    /* No operation. Unused.
    #define CMD_NOP_2(field) \
        BC_BB(0x19, field) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    return std::format("{:#x}", Field);
}

std::string BhvCmdNop3(N64Rom &Rom, u32 &Start) {
    /* No operation. Unused.
    #define CMD_NOP_3(field) \
        BC_BB(0x1A, field) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    return std::format("{:#x}", Field);
}

std::string BhvCmdSetModel(N64Rom &Rom, u32 &Start) {
    /* Sets the current model ID of the object.
    #define SET_MODEL(modelID) \
        BC_B0H(0x1B, modelID) */
    s16 ModelID = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{}", ModelID);
}

std::string BhvCmdSpawnChild(N64Rom &Rom, u32 &Start) {
    /* Spawns a child object with the specified model and behavior.
    #define SPAWN_CHILD(modelID, behavior) \
        BC_B(0x1C), \
        BC_W(modelID), \
        BC_PTR(behavior) */
    u32 ModelID = Rom.ReadBytes<u32>(Start + 4);
    u32 Behavior = Rom.ReadBytes<u32>(Start + 8);
    std::string BehaviorName = GetLabelFromMap(Behavior);
    return std::format("{:#x}, {}", ModelID, BehaviorName);
}

std::string BhvCmdDeactivate(N64Rom &Rom, u32 &Start) {
    /* Exits the behavior script and despawns the object.
    #define DEACTIVATE() \
        BC_B(0x1D) */
    return "";
}

std::string BhvCmdDropToFloor(N64Rom &Rom, u32 &Start) {
    /* Finds the floor triangle directly under the object and moves the object down to it.
    #define DROP_TO_FLOOR() \
        BC_B(0x1E) */
    return "";
}

std::string BhvCmdSumFloat(N64Rom &Rom, u32 &Start) {
    /* Sets the destination float field to the sum of the values of the given float fields.
    #define SUM_FLOAT(fieldDst, fieldSrc1, fieldSrc2) \
        BC_BBBB(0x1F, fieldDst, fieldSrc1, fieldSrc2) */
    u8 FieldDst = Rom.ReadBytes<u8>(Start + 1);
    u8 FieldSrc1 = Rom.ReadBytes<u8>(Start + 2);
    u8 FieldSrc2 = Rom.ReadBytes<u8>(Start + 3);
    return std::format("{:#x}, {:#x}, {:#x}", FieldDst, FieldSrc1, FieldSrc2);
}

std::string BhvCmdSumInt(N64Rom &Rom, u32 &Start) {
    /* Sets the destination integer field to the sum of the values of the given integer fields. Unused.
    #define SUM_INT(fieldDst, fieldSrc1, fieldSrc2) \
        BC_BBBB(0x20, fieldDst, fieldSrc1, fieldSrc2) */
    u8 FieldDst = Rom.ReadBytes<u8>(Start + 1);
    u8 FieldSrc1 = Rom.ReadBytes<u8>(Start + 2);
    u8 FieldSrc2 = Rom.ReadBytes<u8>(Start + 3);
    return std::format("{:#x}, {:#x}, {:#x}", FieldDst, FieldSrc1, FieldSrc2);
}

std::string BhvCmdBillboard(N64Rom &Rom, u32 &Start) {
    /* Billboards the current object, making it always face the camera.
    #define BILLBOARD() \
        BC_B(0x21) */
    return "";
}

std::string BhvCmdHide(N64Rom &Rom, u32 &Start) {
    /* Hides the current object.
    #define HIDE() \
        BC_B(0x22) */
    return "";
}

std::string BhvCmdSetHitbox(N64Rom &Rom, u32 &Start) {
    /* Sets the size of the object's cylindrical hitbox.
    #define SET_HITBOX(radius, height) \
        BC_B(0x23), \
        BC_HH(radius, height) */
    s16 Radius = Rom.ReadBytes<s16>(Start + 4);
    s16 Height = Rom.ReadBytes<s16>(Start + 6);
    return std::format("{}, {}", Radius, Height);
}

std::string BhvCmdNop4(N64Rom &Rom, u32 &Start) {
    /* No operation. Unused.
    #define CMD_NOP_4(field, value) \
        BC_BBH(0x24, field, value) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Value = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{:#x}, {}", Field, Value);
}

std::string BhvCmdDelayVar(N64Rom &Rom, u32 &Start) {
    /* Delays the behavior script for the number of frames given by the value of the specified field.
    #define DELAY_VAR(field) \
        BC_BB(0x25, field) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    return std::format("{:#x}", Field);
}

std::string BhvCmdBeginRepeatUnused(N64Rom &Rom, u32 &Start) {
    /* Unused. Marks the start of a loop that will repeat a certain number of times.
    #define BEGIN_REPEAT_UNUSED(count) \
        BC_BB(0x26, count) */
    u8 Count = Rom.ReadBytes<u8>(Start + 1);
    return std::format("{}", Count);
}

std::string BhvCmdLoadAnimations(N64Rom &Rom, u32 &Start) {
    /* Loads the animations for the object. <field> is always set to oAnimations.
    #define LOAD_ANIMATIONS(field, anims) \
        BC_BB(0x27, field), \
        BC_PTR(anims) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    u32 Anims = Rom.ReadBytes<u32>(Start + 4);
    return std::format("{:#x}, {:#010x}", Field, Anims);
}

std::string BhvCmdAnimate(N64Rom &Rom, u32 &Start) {
    /* Begins animation and sets the object's current animation index to the specified value.
    #define ANIMATE(animIndex) \
        BC_BB(0x28, animIndex) */
    u8 AnimIndex = Rom.ReadBytes<u8>(Start + 1);
    return std::format("{:#x}", AnimIndex);
}

std::string BhvCmdSpawnChildWithParam(N64Rom &Rom, u32 &Start) {
    /* Spawns a child object with the specified model and behavior, plus a behavior param.
    #define SPAWN_CHILD_WITH_PARAM(bhvParam, modelID, behavior) \
        BC_B0H(0x29, bhvParam), \
        BC_W(modelID), \
        BC_PTR(behavior) */
    s16 BhvParam = Rom.ReadBytes<s16>(Start + 2);
    u32 ModelID = Rom.ReadBytes<u32>(Start + 4);
    u32 Behavior = Rom.ReadBytes<u32>(Start + 8);
    return std::format("{}, {:#x}, {:#010x}", BhvParam, ModelID, Behavior);
}

std::string BhvCmdLoadCollisionData(N64Rom &Rom, u32 &Start) {
    /* Loads collision data for the object.
    #define LOAD_COLLISION_DATA(collisionData) \
        BC_B(0x2A), \
        BC_PTR(collisionData) */
    u32 CollisionData = Rom.ReadBytes<u32>(Start + 4);
    return std::format("{:#010x}", CollisionData);
}

std::string BhvCmdSetHitboxWithOffset(N64Rom &Rom, u32 &Start) {
    /* Sets the size of the object's cylindrical hitbox, and applies a downwards offset.
    #define SET_HITBOX_WITH_OFFSET(radius, height, downOffset) \
        BC_B(0x2B), \
        BC_HH(radius, height), \
        BC_H(downOffset) */
    s16 Radius = Rom.ReadBytes<s16>(Start + 4);
    s16 Height = Rom.ReadBytes<s16>(Start + 6);
    s16 DownOffset = Rom.ReadBytes<s16>(Start + 8);
    return std::format("{}, {}, {}", Radius, Height, DownOffset);
}

std::string BhvCmdSpawnObj(N64Rom &Rom, u32 &Start) {
    /* Spawns a new object with the specified model and behavior.
    #define SPAWN_OBJ(modelID, behavior) \
        BC_B(0x2C), \
        BC_W(modelID), \
        BC_PTR(behavior) */
    u32 ModelID = Rom.ReadBytes<u32>(Start + 4);
    u32 Behavior = Rom.ReadBytes<u32>(Start + 8);
    return std::format("{:#x}, {:#010x}", ModelID, Behavior);
}

std::string BhvCmdSetHome(N64Rom &Rom, u32 &Start) {
    /* Sets the home position of the object to its current position.
    #define SET_HOME() \
        BC_B(0x2D) */
    return "";
}

std::string BhvCmdSetHurtbox(N64Rom &Rom, u32 &Start) {
    /* Sets the size of the object's cylindrical hurtbox.
    #define SET_HURTBOX(radius, height) \
        BC_B(0x2E), \
        BC_HH(radius, height) */
    s16 Radius = Rom.ReadBytes<s16>(Start + 4);
    s16 Height = Rom.ReadBytes<s16>(Start + 6);
    return std::format("{}, {}", Radius, Height);
}

std::string BhvCmdSetInteractType(N64Rom &Rom, u32 &Start) {
    /* Sets the object's interaction type.
    #define SET_INTERACT_TYPE(type) \
        BC_B(0x2F), \
        BC_W(type) */
    u32 Type = Rom.ReadBytes<u32>(Start + 4);
    return std::format("{:#x}", Type);
}

std::string BhvCmdSetObjPhysics(N64Rom &Rom, u32 &Start) {
    /* Sets various parameters that the object uses for calculating physics.
    #define SET_OBJ_PHYSICS(wallHitboxRadius, gravity, bounciness, dragStrength, friction, buoyancy, unused1, unused2) \
        BC_B(0x30), \
        BC_HH(wallHitboxRadius, gravity), \
        BC_HH(bounciness, dragStrength), \
        BC_HH(friction, buoyancy), \
        BC_HH(unused1, unused2) */
    s16 WallHitboxRadius = Rom.ReadBytes<s16>(Start + 4);
    s16 Gravity = Rom.ReadBytes<s16>(Start + 6);
    s16 Bounciness = Rom.ReadBytes<s16>(Start + 8);
    s16 DragStrength = Rom.ReadBytes<s16>(Start + 10);
    s16 Friction = Rom.ReadBytes<s16>(Start + 12);
    s16 Buoyancy = Rom.ReadBytes<s16>(Start + 14);
    s16 Unused1 = Rom.ReadBytes<s16>(Start + 16);
    s16 Unused2 = Rom.ReadBytes<s16>(Start + 18);
    return std::format("{}, {}, {}, {}, {}, {}, {}, {}", 
        WallHitboxRadius, Gravity, Bounciness, DragStrength, Friction, Buoyancy, Unused1, Unused2);
}

std::string BhvCmdSetInteractSubtype(N64Rom &Rom, u32 &Start) {
    /* Sets the object's interaction subtype. Unused.
    #define SET_INTERACT_SUBTYPE(subtype) \
        BC_B(0x31), \
        BC_W(subtype) */
    u32 Subtype = Rom.ReadBytes<u32>(Start + 4);
    return std::format("{:#x}", Subtype);
}

std::string BhvCmdScale(N64Rom &Rom, u32 &Start) {
    /* Sets the object's size to the specified percentage.
    #define SCALE(unusedField, percent) \
        BC_BBH(0x32, unusedField, percent) */
    u8 UnusedField = Rom.ReadBytes<u8>(Start + 1);
    s16 Percent = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{:#x}, {}", UnusedField, Percent);
}

std::string BhvCmdParentBitClear(N64Rom &Rom, u32 &Start) {
    /* Performs a bit clear on the object's parent's field with the specified value.
    // Used for clearing active particle flags fron Mario's object.
    #define PARENT_BIT_CLEAR(field, flags) \
        BC_BB(0x33, field), \
        BC_W(flags) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    u32 Flags = Rom.ReadBytes<u32>(Start + 4);
    return std::format("{:#x}, {:#x}", Field, Flags);
}

std::string BhvCmdAnimateTexture(N64Rom &Rom, u32 &Start) {
    /* Animates an object using texture animation. <field> is always set to oAnimState.
    #define ANIMATE_TEXTURE(field, rate) \
        BC_BBH(0x34, field, rate) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Rate = Rom.ReadBytes<s16>(Start + 2);
    return std::format("{:#x}, {}", Field, Rate);
}

std::string BhvCmdDisableRendering(N64Rom &Rom, u32 &Start) {
    /* Disables rendering for the object.
    #define DISABLE_RENDERING() \
        BC_B(0x35) */
    return "";
}

std::string BhvCmdSetIntUnused(N64Rom &Rom, u32 &Start) {
    /* Unused. Sets the specified field to an integer. Wastes 4 bytes of space for no reason at all.
    #define SET_INT_UNUSED(field, value) \
        BC_BB(0x36, field), \
        BC_HH(0, value) */
    u8 Field = Rom.ReadBytes<u8>(Start + 1);
    s16 Value = Rom.ReadBytes<s16>(Start + 6);
    return std::format("{:#x}, {}", Field, Value);
}

std::string BhvCmdSpawnWaterDroplet(N64Rom &Rom, u32 &Start) {
    /* Spawns a water droplet with the given parameters.
    #define SPAWN_WATER_DROPLET(dropletParams) \
        BC_B(0x37), \
        BC_PTR(dropletParams) */
    u32 DropletParams = Rom.ReadBytes<u32>(Start + 4);
    return std::format("{:#010x}", DropletParams);
}

std::string (*BhvCommandsFunctions[])(N64Rom &Rom, u32 &Start) = {
    BhvCmdBegin,                // 0x00
    BhvCmdDelay,                // 0x01
    BhvCmdCall,                 // 0x02
    BhvCmdReturn,               // 0x03
    BhvCmdGoto,                 // 0x04
    BhvCmdBeginRepeat,          // 0x05
    BhvCmdEndRepeat,            // 0x06
    BhvCmdEndRepeatContinue,    // 0x07
    BhvCmdBeginLoop,            // 0x08
    BhvCmdEndLoop,              // 0x09
    BhvCmdBreak,                // 0x0A
    BhvCmdBreakUnused,          // 0x0B
    BhvCmdCallNative,           // 0x0C
    BhvCmdAddFloat,             // 0x0D
    BhvCmdSetFloat,             // 0x0E
    BhvCmdAddInt,               // 0x0F
    BhvCmdSetInt,               // 0x10
    BhvCmdOrInt,                // 0x11
    BhvCmdBitClear,             // 0x12
    BhvCmdSetIntRandRshift,     // 0x13
    BhvCmdSetRandomFloat,       // 0x14
    BhvCmdSetRandomInt,         // 0x15
    BhvCmdAddRandomFloat,       // 0x16
    BhvCmdAddIntRandRshift,     // 0x17
    BhvCmdNop1,                 // 0x18
    BhvCmdNop2,                 // 0x19
    BhvCmdNop3,                 // 0x1A
    BhvCmdSetModel,             // 0x1B
    BhvCmdSpawnChild,           // 0x1C
    BhvCmdDeactivate,           // 0x1D
    BhvCmdDropToFloor,          // 0x1E
    BhvCmdSumFloat,             // 0x1F
    BhvCmdSumInt,               // 0x20
    BhvCmdBillboard,            // 0x21
    BhvCmdHide,                 // 0x22
    BhvCmdSetHitbox,            // 0x23
    BhvCmdNop4,                 // 0x24
    BhvCmdDelayVar,             // 0x25
    BhvCmdBeginRepeatUnused,    // 0x26
    BhvCmdLoadAnimations,       // 0x27
    BhvCmdAnimate,              // 0x28
    BhvCmdSpawnChildWithParam,  // 0x29
    BhvCmdLoadCollisionData,    // 0x2A
    BhvCmdSetHitboxWithOffset,  // 0x2B
    BhvCmdSpawnObj,             // 0x2C
    BhvCmdSetHome,              // 0x2D
    BhvCmdSetHurtbox,           // 0x2E
    BhvCmdSetInteractType,      // 0x2F
    BhvCmdSetObjPhysics,        // 0x30
    BhvCmdSetInteractSubtype,   // 0x31
    BhvCmdScale,                // 0x32
    BhvCmdParentBitClear,       // 0x33
    BhvCmdAnimateTexture,       // 0x34
    BhvCmdDisableRendering,     // 0x35
    BhvCmdSetIntUnused,         // 0x36
    BhvCmdSpawnWaterDroplet     // 0x37
};

u8 GetBehaviorScriptCmdSize(N64Rom &Rom, u32 Entry) {
    u8 Cmd = Rom.ReadBytes<u8>(Entry);
    switch (Cmd) {
        case 0x02: case 0x04: case 0x0C: case 0x13: 
        case 0x14: case 0x15: case 0x16: case 0x17: 
        case 0x23: case 0x27: case 0x2A: case 0x2E: 
        case 0x2F: case 0x31: case 0x33: case 0x36: 
        case 0x37:
            return 8;
        case 0x1C: case 0x29: case 0x2B: case 0x2C:
            return 12;
        case 0x30:
            return 20;
        default:
            return 4;
    }
}

static void WriteBehaviorScriptRecursive(FILE *BhvDump, N64Rom &Rom, u32 Entry, u32 SegAddr) {
    if (!SegAddr || ProcessedBhvs[SegAddr]) return;
    ProcessedBhvs[SegAddr] = true;
    u32 ScanEntry = Entry;
    u32 CommandCount = 0;

    while (true) {
        u8 Cmd = Rom.ReadBytes<u8>(ScanEntry);
        if (Cmd > 0x36) {
            printf("Unknown BehaviorScript Command 0x%x at address 0x%x\n", Cmd, Entry);
            break;
        }
        u8 Len = GetBehaviorScriptCmdSize(Rom, ScanEntry);
        
        if (Cmd == 0x02 || Cmd == 0x04) {
            u32 NewSegAddr = Rom.ReadBytes<u32>(ScanEntry + 4);
            if (ValidateMemAddr(NewSegAddr)) {
                WriteBehaviorScriptRecursive(BhvDump, Rom, NewSegAddr, NewSegAddr);
            }
        }

        if (Cmd == 0x09 || Cmd == 0x0A || Cmd == 0x1D || Cmd == 0x04) break;
        if (++CommandCount > 10000) {
            printf("BehaviorScript 0x%x is broken, ignoring export\n", SegAddr);
            break;
        }
        ScanEntry += Len;
    }

    fprintf(BhvDump, "const BehaviorScript %s[] = {\n", GetLabelFromMap(SegAddr).c_str());

    CommandCount = 0;
    while (true) {
        u8 Cmd = Rom.ReadBytes<u8>(Entry);
        if (Cmd > 0x36) {
            printf("Unknown BehaviorScript Command 0x%x at address 0x%x\n", Cmd, Entry);
            break;
        }
        u8 Len = GetBehaviorScriptCmdSize(Rom, Entry);

        if (BhvCommandsFunctions[Cmd]) {
            std::string CmdName = BhvCommandsName[Cmd];
            std::string Args = BhvCommandsFunctions[Cmd](Rom, Entry);
            fprintf(BhvDump, "    %s(%s),\n", CmdName.c_str(), Args.c_str());

            if (Cmd == 0x09 || Cmd == 0x0A || Cmd == 0x1D || Cmd == 0x04) break;
        } else {
            printf("Unimplemented BehaviorScript command 0x%x at address 0x%x\n", Cmd, Entry);
            break;
        }
        if (++CommandCount > 10000) {
            printf("BehaviorScript 0x%x is broken, ignoring export\n", SegAddr);
            break;
        }
        Entry += Len;
    }

    fprintf(BhvDump, "};\n\n");
}

void ExportBehaviorScripts(N64Rom &Rom, LevelScript &Script) {
    std::string DataPath = "output/data/";
    fs::create_directories(DataPath);

    std::string BhvPath = DataPath + "behaviors.c";
    FILE *BhvDump = fopen(BhvPath.c_str(), "a");

    for (auto &Bhv : Script.Behaviors) {
        if (ProcessedBhvs[Bhv]) continue;
        printf("Exporting behavior 0x%x\n", Bhv);
        WriteBehaviorScriptRecursive(BhvDump, Rom, Bhv, Bhv);
    }

    fclose(BhvDump);
}