#include "GeoLayout.h"
#include "Actor.h"
#include "LevelScript.h"
#include "Memory.h"
#include "Skybox.h"

std::map<u32, bool> ProcessedGeos;

std::string GeoCommandsName[] = {
    "GEO_BRANCH_AND_LINK",
    "GEO_END",
    "GEO_BRANCH",
    "GEO_RETURN",
    "GEO_OPEN_NODE",
    "GEO_CLOSE_NODE",
    "GEO_ASSIGN_AS_VIEW",
    "GEO_UPDATE_NODE_FLAGS",
    "GEO_NODE_SCREEN_AREA",
    "GEO_NODE_ORTHO",
    "GEO_CAMERA_FRUSTUM",
    "GEO_NODE_START",
    "GEO_ZBUFFER",
    "GEO_RENDER_RANGE",
    "GEO_SWITCH_CASE",
    "GEO_CAMERA",
    "GEO_TRANSLATE_ROTATE",
    "GEO_TRANSLATE_NODE",
    "GEO_ROTATION_NODE",
    "GEO_ANIMATED_PART",
    "GEO_BILLBOARD",
    "GEO_DISPLAY_LIST",
    "GEO_SHADOW",
    "GEO_RENDER_OBJ",
    "GEO_ASM",
    "GEO_BACKGROUND",
    "GEO_NOP_1A",
    "GEO_COPY_VIEW",
    "GEO_HELD_OBJECT",
    "GEO_SCALE",
    "GEO_NOP_1E",
    "GEO_NOP_1F",
    "GEO_CULLING_RADIUS",
};

std::string GetGeoLayerName(u8 Layer) {
    if (!GameType.IsHacker()) {
        switch (Layer) {
            case 0: return "LAYER_FORCE";
            case 1: return "LAYER_OPAQUE";
            case 2: return "LAYER_OPAQUE_DECAL";
            case 3: return "LAYER_OPAQUE_INTER";
            case 4: return "LAYER_ALPHA";
            case 5: return "LAYER_TRANSPARENT";
            case 6: return "LAYER_TRANSPARENT_DECAL";
            case 7: return "LAYER_TRANSPARENT_INTER";
        }
    } else {
        switch (Layer) {
            case 0: return "LAYER_FORCE";
            case 1: return "LAYER_OPAQUE";
            case 2: return "LAYER_OPAQUE_INTER";
            case 3: return "LAYER_OPAQUE_DECAL";
            case 4: return "LAYER_ALPHA";
            case 5: return "LAYER_ALPHA_DECAL";
            case 6: return "LAYER_SILHOUETTE_OPAQUE";
            case 7: return "LAYER_SILHOUETTE_ALPHA";
            case 8: return "LAYER_OCCLUDE_SILHOUETTE_OPAQUE";
            case 9: return "LAYER_OCCLUDE_SILHOUETTE_ALPHA";
            case 10: return "LAYER_TRANSPARENT_DECAL";
            case 11: return "LAYER_TRANSPARENT";
            case 12: return "LAYER_TRANSPARENT_INTER";
        }
    }
    return "LAYER_OPAQUE";
}

std::string GetGeoDLName(LevelScript &Script, u8 Area, u32 DisplayList) {
    if (Script.CurrentActor) {
        return std::format("{}_displaylist_{:#x}", Script.CurrentActor->Name, DisplayList);
    }
    return std::format("{}_{}_displaylist_{:#x}", Script.Name, Area, DisplayList);
}

std::string GeoCmdStub(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    return "";
};

std::string GeoCmdBranchAndLink(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
    * 0x00: Branch and store return address
    *   0x04: scriptTarget, segment address of geo layout
    *
    #define GEO_BRANCH_AND_LINK(scriptTarget) \
        CMD_BBH(0x00, 0x00, 0x0000), \
        CMD_PTR(scriptTarget)
    */

    u32 ScriptTarget = Rom.ReadBytes<u32>(Start + 4);

    std::string ScriptTargetName;
    if (Script.CurrentActor) {
        ScriptTargetName = std::format("{}_geo_{:x}", Script.CurrentActor->Name, ScriptTarget);
    } else {
        ScriptTargetName = std::format("{}_area_{}_geo_{:x}", Script.Name, Area, ScriptTarget);
    }

    std::string OutArgs = std::format(
        "{}",
        ScriptTargetName
    );

    return OutArgs;
};

std::string GeoCmdEnd(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x01: Terminate geo layout
     *   0x01-0x03: unused
     *
    #define GEO_END() \
        CMD_BBH(0x01, 0x00, 0x0000)
    */

    return "";
};

std::string GeoCmdBranch(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x02: Branch
     *   0x01: if 1, store next geo layout address on stack
     *   0x02-0x03: unused
     *   0x04: scriptTarget, segment address of geo layout
     *
    #define GEO_BRANCH(type, scriptTarget) \
        CMD_BBH(0x02, type, 0x0000), \
        CMD_PTR(scriptTarget)
    */

    u8 Branch = Rom.ReadBytes<u8>(Start + 1);
    u32 ScriptTarget = Rom.ReadBytes<u32>(Start + 4);

    std::string ScriptTargetName;
    if (Script.CurrentActor) {
        ScriptTargetName = std::format("{}_geo_{:x}", Script.CurrentActor->Name, ScriptTarget);
    } else {
        ScriptTargetName = std::format("{}_area_{}_geo_{:x}", Script.Name, Area, ScriptTarget);
    }

    std::string OutArgs = std::format(
        "{}, {}",
        Branch, ScriptTargetName
    );

    return OutArgs;
};


std::string GeoCmdReturn(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x03: Return from branch
     *   0x01-0x03: unused
     *
    #define GEO_RETURN() \
        CMD_BBH(0x03, 0x00, 0x0000)
    */

    return "";
};

std::string GeoCmdScreenArea(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /*
    * 0x08: Create screen area scene graph node
    *   0x01: unused
    *   0x02: s16 num entries (+2) to allocate
    *   0x04: s16 x
    *   0x06: s16 y
    *   0x08: s16 width
    *   0x0A: s16 height
    *
    #define GEO_NODE_SCREEN_AREA(numEntries, x, y, width, height) \
        CMD_BBH(0x08, 0x00, numEntries), \
        CMD_HH(x, y), \
        CMD_HH(width, height)
    */

    s16 NumEntries = Rom.ReadBytes<s16>(Start + 2);
    s16 X = Rom.ReadBytes<s16>(Start + 4);
    s16 Y = Rom.ReadBytes<s16>(Start + 6);
    s16 Width = Rom.ReadBytes<s16>(Start + 8);
    s16 Height = Rom.ReadBytes<s16>(Start + 10);

    std::string OutArgs = std::format(
        "{}, {}, {}, {}, {}",
        NumEntries, X, Y, Width, Height
    );

    return OutArgs;
};

std::string GeoCmdOrtho(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x09: Create orthographic projection scene graph node
     *   0x02: s16 scale as percentage
     *
    #define GEO_NODE_ORTHO(scale) \
        CMD_BBH(0x09, 0x00, scale)
    */

    s16 Scale = Rom.ReadBytes<s16>(Start + 2);

    std::string OutArgs = std::format(
        "{}",
        Scale
    );

    return OutArgs;
};

std::string GeoCmdCamFrustum(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x0A: Create camera frustum scene graph node
     *   0x01: u8  if nonzero, enable function field
     *   0x02: s16 field of view
     *   0x04: s16 near
     *   0x06: s16 far
     *   0x08: [GraphNodeFunc function]
    *
    #define GEO_CAMERA_FRUSTUM(fov, near, far) \
        CMD_BBH(0x0A, 0x00, fov), \
        CMD_HH(near, far)
    #define GEO_CAMERA_FRUSTUM_WITH_FUNC(fov, near, far, func) \
        CMD_BBH(0x0A, 0x01, fov), \
        CMD_HH(near, far), \
        CMD_PTR(func)
    */

    u8 FuncEnable = Rom.ReadBytes<u8>(Start + 1);
    s16 FOV = Rom.ReadBytes<s16>(Start + 2);
    s16 Near = Rom.ReadBytes<s16>(Start + 4);
    s16 Far = Rom.ReadBytes<s16>(Start + 6);

    std::string OutArgs = "";
    if (FuncEnable) {
        u32 Func = Rom.ReadBytes<u32>(Start + 8);
        std::string FuncName = "geo_camera_fov";
        OutArgs = std::format(
            "{}, {}, {}, {}",
            FOV, Near, Far, FuncName
        );
    } else {
        OutArgs = std::format(
            "{}, {}, {}",
            FOV, Near, Far
        );
    }

    return OutArgs;
};

std::string GeoCmdZbuffer(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x0C: Create zbuffer-toggling scene graph node
     *   0x01: u8 enableZBuffer (1 = on, 0 = off)
     *   0x02-0x03: unused
     *
    #define GEO_ZBUFFER(enable) \
        CMD_BBH(0x0C, enable, 0x0000))
    */

    u8 Enable = Rom.ReadBytes<u8>(Start + 1);

    std::string OutArgs = std::format(
        "{}",
        Enable
    );

    return OutArgs;
};

std::string GeoCmdRenderRange(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x0D: Create render range scene graph node
     *   0x01-0x03: unused
     *   0x04: s16 minDistance
     *   0x06: s16 maxDistance
     *
    #define GEO_RENDER_RANGE(minDistance, maxDistance) \
        CMD_BBH(0x0D, 0x00, 0x0000), \
        CMD_HH(minDistance, maxDistance)
    */

    s16 MinDist = Rom.ReadBytes<s16>(Start + 4);
    s16 MaxDist = Rom.ReadBytes<s16>(Start + 6);

    std::string OutArgs = std::format(
        "{}, {}",
        MinDist, MaxDist
    );

    return OutArgs;
};

std::string GeoCmdSwitchCase(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x0E: Create switch-case scene graph node
     *   0x01: unused
     *   0x02: s16 parameter
     *   0x04: GraphNodeFunc caseSelectorFunc
     *
    #define GEO_SWITCH_CASE(count, function) \
        CMD_BBH(0x0E, 0x00, count), \
        CMD_PTR(function)
    */


    s16 Param = Rom.ReadBytes<s16>(Start + 2);
    u32 Func = Rom.ReadBytes<u32>(Start + 4);

    std::string FuncName = GetLabelFromMap(Func);

    std::string OutArgs = std::format(
        "{}, {}",
        Param, FuncName
    );

    return OutArgs;
};

std::string GeoCmdCamera(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x0F: Create a camera scene graph node.
     *   0x01: unused
     *   0x02: s16 camera type
     *   0x04: s16 posX
     *   0x06: s16 posY
     *   0x08: s16 posZ
     *   0x0A: s16 focusX
     *   0x0C: s16 focusY
     *   0x0E: s16 focusZ
     *   0x10: GraphNodeFunc function
     *
    #define GEO_CAMERA(type, x1, y1, z1, x2, y2, z2, function) \
        CMD_BBH(0x0F, 0x00, type), \
        CMD_HHHHHH(x1, y1, z1, x2, y2, z2), \
        CMD_PTR(function)
    */

    s16 Type = Rom.ReadBytes<s16>(Start + 2);
    s16 PosX = Rom.ReadBytes<s16>(Start + 4);
    s16 PosY = Rom.ReadBytes<s16>(Start + 6);
    s16 PosZ = Rom.ReadBytes<s16>(Start + 8);
    s16 FocX = Rom.ReadBytes<s16>(Start + 10);
    s16 FocY = Rom.ReadBytes<s16>(Start + 12);
    s16 FocZ = Rom.ReadBytes<s16>(Start + 14);
    u32 Func = Rom.ReadBytes<u32>(Start + 16);

    std::string FuncName = "geo_camera_main";
    std::string OutArgs = std::format(
        "{}, {}, {}, {}, {}, {}, {}, {}",
        Type, PosX, PosY, PosZ, FocX, FocY, FocZ, FuncName
    );

    return OutArgs;
};

std::string GeoCmdTransRot(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x10: Create translation & rotation scene graph node with optional display list
     * Four different versions of 0x10
     *   cmd+0x01: u8 params
     *     0b1000_0000: if set, enable displayList field and drawingLayer
     *     0b0111_0000: fieldLayout (determines how rest of data is formatted
     *     0b0000_1111: drawingLayer
     *
     *   fieldLayout = 0: Translate & Rotate
     *     0x04: s16 xTranslation
     *     0x06: s16 yTranslation
     *     0x08: s16 zTranslation
     *     0x0A: s16 xRotation
     *     0x0C: s16 yRotation
     *     0x0E: s16 zRotation
     *     0x10: [u32 displayList: if MSbit of params set, display list segmented address]
     *
    #define GEO_TRANSLATE_ROTATE(layer, tx, ty, tz, rx, ry, rz) \
        CMD_BBH(0x10, (0x00 | layer), 0x0000), \
        CMD_HHHHHH(tx, ty, tz, rx, ry, rz)
    #define GEO_TRANSLATE_ROTATE_WITH_DL(layer, tx, ty, tz, rx, ry, rz, displayList) \
        CMD_BBH(0x10, (0x00 | layer | 0x80), 0x0000), \
        CMD_HHHHHH(tx, ty, tz, rx, ry, rz), \
        CMD_PTR(displayList)

     *   fieldLayout = 1: Translate
     *     0x02: s16 xTranslation
     *     0x04: s16 yTranslation
     *     0x06: s16 zTranslation
     *     0x08: [u32 displayList: if MSbit of params set, display list segmented address]
     *
    #define GEO_TRANSLATE(layer, tx, ty, tz) \
        CMD_BBH(0x10, (0x10 | layer), tx), \
        CMD_HH(ty, tz)
    #define GEO_TRANSLATE_WITH_DL(layer, tx, ty, tz, displayList) \
        CMD_BBH(0x10, (0x10 | layer | 0x80), tx), \
        CMD_HH(ty, tz), \
        CMD_PTR(displayList)

     *   fieldLayout = 2: Rotate
     *     0x02: s16 xRotation
     *     0x04: s16 yRotation
     *     0x06: s16 zRotation
     *     0x08: [u32 displayList: if MSbit of params set, display list segmented address]
     *
    #define GEO_ROTATE(layer, rx, ry, rz) \
        CMD_BBH(0x10, (0x20 | layer), rx), \
        CMD_HH(ry, rz)
    #define GEO_ROTATE_WITH_DL(layer, rx, ry, rz, displayList) \
        CMD_BBH(0x10, (0x20 | layer | 0x80), rx), \
        CMD_HH(ry, rz), \
        CMD_PTR(displayList)

     *   fieldLayout = 3: Rotate Y
     *     0x02: s16 yRotation
     *     0x04: [u32 displayList: if MSbit of params set, display list segmented address]
     *
    #define GEO_ROTATE_Y(layer, ry) \
        CMD_BBH(0x10, (0x30 | layer), ry)
    #define GEO_ROTATE_Y_WITH_DL(layer, ry, displayList) \
        CMD_BBH(0x10, (0x30 | layer | 0x80), ry), \
        CMD_PTR(displayList)
    */


    u8 Layer = Rom.ReadBytes<u8>(Start + 1);

    std::string OutArgs = "";
    if (Layer & 0x30) {
        s16 YRot = Rom.ReadBytes<s16>(Start + 2);

        if (Layer & 0x80) {
            u32 DisplayList = Rom.ReadBytes<u32>(Start + 4);
            std::string DisplayListName = "0";
            if (DisplayList) {
                DisplayListName = GetGeoDLName(Script, Area, DisplayList);
                Script.AddDisplayList(DisplayList, Area);
            }
            OutArgs = std::format(
                "{}, {}, {}",
                GetGeoLayerName(Layer & ~(0x80 | 0x30)), YRot, DisplayListName
            );
        } else {
            OutArgs = std::format(
                "{}, {}",
                GetGeoLayerName(Layer & ~0x30), YRot
            );
        }
    } else if (Layer & 0x20) {
        s16 XRot = Rom.ReadBytes<s16>(Start + 2);
        s16 YRot = Rom.ReadBytes<s16>(Start + 4);
        s16 ZRot = Rom.ReadBytes<s16>(Start + 6);

        if (Layer & 0x80) {
            u32 DisplayList = Rom.ReadBytes<u32>(Start + 8);
            std::string DisplayListName = "0";
            if (DisplayList) {
                DisplayListName = GetGeoDLName(Script, Area, DisplayList);
                Script.AddDisplayList(DisplayList, Area);
            }
            OutArgs = std::format(
                "{}, {}, {}, {}, {}",
                GetGeoLayerName(Layer & ~(0x80 | 0x20)), XRot, YRot, ZRot, DisplayListName
            );
        } else {
            OutArgs = std::format(
                "{}, {}, {}, {}",
                GetGeoLayerName(Layer & ~0x20), XRot, YRot, ZRot
            );
        }
    } else if (Layer & 0x10) {
        s16 XTrans = Rom.ReadBytes<s16>(Start + 2);
        s16 YTrans = Rom.ReadBytes<s16>(Start + 4);
        s16 ZTrans = Rom.ReadBytes<s16>(Start + 6);

        if (Layer & 0x80) {
            u32 DisplayList = Rom.ReadBytes<u32>(Start + 8);
            std::string DisplayListName = "0";
            if (DisplayList) {
                DisplayListName = GetGeoDLName(Script, Area, DisplayList);
                Script.AddDisplayList(DisplayList, Area);
            }
            OutArgs = std::format(
                "{}, {}, {}, {}, {}",
                GetGeoLayerName(Layer & ~(0x80 | 0x10)), XTrans, YTrans, ZTrans, DisplayListName
            );
        } else {
            OutArgs = std::format(
                "{}, {}, {}, {}",
                GetGeoLayerName(Layer & ~0x10), XTrans, YTrans, ZTrans
            );
        }
    } else {
        s16 XTrans = Rom.ReadBytes<s16>(Start + 4);
        s16 YTrans = Rom.ReadBytes<s16>(Start + 6);
        s16 ZTrans = Rom.ReadBytes<s16>(Start + 8);
        s16 XRot = Rom.ReadBytes<s16>(Start + 10);
        s16 YRot = Rom.ReadBytes<s16>(Start + 12);
        s16 ZRot = Rom.ReadBytes<s16>(Start + 14);

        if (Layer & 0x80) {
            u32 DisplayList = Rom.ReadBytes<u32>(Start + 16);
            std::string DisplayListName = "0";
            if (DisplayList) {
                DisplayListName = GetGeoDLName(Script, Area, DisplayList);
                Script.AddDisplayList(DisplayList, Area);
            }
            OutArgs = std::format(
                "{}, {}, {}, {}, {}, {}, {}, {}",
                GetGeoLayerName(Layer & ~0x80), XTrans, YTrans, ZTrans, XRot, YRot, ZRot, DisplayListName
            );
        } else {
            OutArgs = std::format(
                "{}, {}, {}, {}, {}, {}, {}",
                GetGeoLayerName(Layer), XTrans, YTrans, ZTrans, XRot, YRot, ZRot
            );
        }
    }

    return OutArgs;
};

std::string GeoCmdTransNode(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x11: Create translation scene graph node with optional display list
     *   0x01: u8 params
     *     0b1000_0000: if set, enable displayList field and drawingLayer
     *     0b0000_1111: drawingLayer
     *   0x02: s16 translationX
     *   0x04: s16 translationY
     *   0x06: s16 translationZ
     *   0x08: [u32 displayList: if MSbit of params set, display list segmented address]
     *
    #define GEO_TRANSLATE_NODE(layer, ux, uy, uz) \
        CMD_BBH(0x11, layer, ux), \
        CMD_HH(uy, uz)
    #define GEO_TRANSLATE_NODE_WITH_DL(layer, ux, uy, uz, displayList) \
        CMD_BBH(0x11, (layer | 0x80), ux), \
        CMD_HH(uy, uz), \
        CMD_PTR(displayList)
    */

    u8 Layer = Rom.ReadBytes<u8>(Start + 1);
    s16 TransX = Rom.ReadBytes<s16>(Start + 2);
    s16 TransY = Rom.ReadBytes<s16>(Start + 4);
    s16 TransZ = Rom.ReadBytes<s16>(Start + 6);

    std::string OutArgs = "";
    if (Layer & 0x80) {
        u32 DisplayList = Rom.ReadBytes<u32>(Start + 8);
        std::string DisplayListName = "0";
        if (DisplayList) {
            DisplayListName = GetGeoDLName(Script, Area, DisplayList);
            Script.AddDisplayList(DisplayList, Area);
        }

        OutArgs = std::format(
            "{}, {}, {}, {}, {}",
            GetGeoLayerName(Layer & ~0x80), TransX, TransY, TransZ, DisplayListName
        );
    } else {
        OutArgs = std::format(
            "{}, {}, {}, {}",
            GetGeoLayerName(Layer), TransX, TransY, TransZ
        );
    }

    return OutArgs;
};

std::string GeoCmdRotNode(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x12: Create rotation scene graph node with optional display list
     *   0x01: u8 params
     *      0b1000_0000: if set, enable displayList field and drawingLayer
     *      0b0000_1111: drawingLayer
     *   0x02: s16 rotationX
     *   0x04: s16 rotationY
     *   0x06: s16 rotationZ
     *   0x08: [u32 displayList: if MSbit of params set, display list segmented address]
     *
    #define GEO_ROTATION_NODE(layer, ux, uy, uz) \
        CMD_BBH(0x12, layer, ux), \
        CMD_HH(uy, uz)
    #define GEO_ROTATION_NODE_WITH_DL(layer, ux, uy, uz, displayList) \
        CMD_BBH(0x12, (layer | 0x80), ux), \
        CMD_HH(uy, uz), \
        CMD_PTR(displayList)
    */

    u8 Layer = Rom.ReadBytes<u8>(Start + 1);
    s16 RotX = Rom.ReadBytes<s16>(Start + 2);
    s16 RotY = Rom.ReadBytes<s16>(Start + 4);
    s16 RotZ = Rom.ReadBytes<s16>(Start + 6);

    std::string OutArgs = "";
    if (Layer & 0x80) {
        u32 DisplayList = Rom.ReadBytes<u32>(Start + 8);
        std::string DisplayListName = "0";
        if (DisplayList) {
            DisplayListName = GetGeoDLName(Script, Area, DisplayList);
            Script.AddDisplayList(DisplayList, Area);
        }

        OutArgs = std::format(
            "{}, {}, {}, {}, {}",
            GetGeoLayerName(Layer & ~0x80), RotX, RotY, RotZ, DisplayListName
        );
    } else {
        OutArgs = std::format(
            "{}, {}, {}, {}",
            GetGeoLayerName(Layer), RotX, RotY, RotZ
        );
    }

    return OutArgs;
};

std::string GeoCmdAnimatedPart(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x13: Create a scene graph node that is rotated by the object's animation.
     *   0x01: u8 drawingLayer
     *   0x02: s16 xTranslation
     *   0x04: s16 yTranslation
     *   0x06: s16 zTranslation
     *   0x08: u32 displayList: dislay list segmented address
     *
    #define GEO_ANIMATED_PART(layer, x, y, z, displayList) \
        CMD_BBH(0x13, layer, x), \
        CMD_HH(y, z), \
        CMD_PTR(displayList)
    */

    u8 Layer = Rom.ReadBytes<u8>(Start + 1);
    s16 TransX = Rom.ReadBytes<s16>(Start + 2);
    s16 TransY = Rom.ReadBytes<s16>(Start + 4);
    s16 TransZ = Rom.ReadBytes<s16>(Start + 6);
    u32 DisplayList = Rom.ReadBytes<u32>(Start + 8);

    std::string DisplayListName = "0";
    if (DisplayList) {
        DisplayListName = GetGeoDLName(Script, Area, DisplayList);
        Script.AddDisplayList(DisplayList, Area);
    }
    std::string OutArgs = std::format(
        "{}, {}, {}, {}, {}",
        GetGeoLayerName(Layer), TransX, TransY, TransZ, DisplayListName
    );

    return OutArgs;
};

std::string GeoCmdBillboard(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x14: Create billboarding node with optional display list
     *   0x01: u8 params
     *      0b1000_0000: if set, enable displayList field and drawingLayer
     *      0b0000_1111: drawingLayer
     *   0x02: s16 xTranslation
     *   0x04: s16 yTranslation
     *   0x06: s16 zTranslation
     *   0x08: [u32 displayList: if MSbit of params is set, display list segmented address]
     *
    #define GEO_BILLBOARD_WITH_PARAMS(layer, tx, ty, tz) \
        CMD_BBH(0x14, layer, tx), \
        CMD_HH(ty, tz)
    #define GEO_BILLBOARD_WITH_PARAMS_AND_DL(layer, tx, ty, tz, displayList) \
        CMD_BBH(0x14, (layer | 0x80), tx), \
        CMD_HH(ty, tz), \
        CMD_PTR(displayList)
    #define GEO_BILLBOARD() \
        GEO_BILLBOARD_WITH_PARAMS(0, 0, 0, 0)
    */

    u8 Layer = Rom.ReadBytes<u8>(Start + 1);
    s16 TransX = Rom.ReadBytes<s16>(Start + 2);
    s16 TransY = Rom.ReadBytes<s16>(Start + 4);
    s16 TransZ = Rom.ReadBytes<s16>(Start + 6);

    std::string OutArgs;
    if (Layer & 0x80) {
        u32 DisplayList = Rom.ReadBytes<u32>(Start + 8);
        std::string DisplayListName = "0";
        if (DisplayList) {
            DisplayListName = GetGeoDLName(Script, Area, DisplayList);
            Script.AddDisplayList(DisplayList, Area);
        }
        OutArgs = std::format(
            "{}, {}, {}, {}, {}",
            GetGeoLayerName(Layer & ~0x80), TransX, TransY, TransZ, DisplayListName
        );
    } else {
        OutArgs = std::format(
            "{}, {}, {}, {}",
            GetGeoLayerName(Layer), TransX, TransY, TransZ
        );
    }

    return OutArgs;
};

std::string GeoCmdDisplayList(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x15: Create plain display list scene graph node
     *   0x01: u8 drawingLayer
     *   0x02-0x03: unused
     *   0x04: u32 displayList: display list segmented address
     *
    #define GEO_DISPLAY_LIST(layer, displayList) \
        CMD_BBH(0x15, layer, 0x0000), \
        CMD_PTR(displayList)
    */

    u8 Layer = Rom.ReadBytes<u8>(Start + 1);
    u32 DisplayList = Rom.ReadBytes<u32>(Start + 4);

    std::string DisplayListName = "0";
    if (DisplayList) {
        DisplayListName = GetGeoDLName(Script, Area, DisplayList);
        Script.AddDisplayList(DisplayList, Area);
    }
    std::string OutArgs = std::format(
        "{}, {}",
        GetGeoLayerName(Layer), DisplayListName
    );

    return OutArgs;
};

std::string GeoCmdShadow(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
    * 0x16: Create shadow scene graph node
    *   0x01: unused
    *   0x02: s16 shadowType (cast to u8)
    *   0x04: s16 shadowSolidity (cast to u8)
    *   0x06: s16 shadowScale
    *
    #define GEO_SHADOW(type, solidity, scale) \
        CMD_BBH(0x16, 0x00, type), \
        CMD_HH(solidity, scale)
    */

    s16 ShadowType = Rom.ReadBytes<s16>(Start + 2) & 0xff;
    s16 ShadowSolidity = Rom.ReadBytes<s16>(Start + 4) & 0xff;
    s16 ShadowScale = Rom.ReadBytes<s16>(Start + 6);

    std::string OutArgs = std::format(
        "{}, {}, {}",
        ShadowType, ShadowSolidity, ShadowScale
    );

    return OutArgs;
};

std::string GeoCmdASM(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x18: Create dynamically generated displaylist scene graph node
     *   0x01: unused
     *   0x02: s16 parameter
     *   0x04: GraphNodeFunc function
     *
    #define GEO_ASM(param, function) \
        CMD_BBH(0x18, 0x00, param), \
        CMD_PTR(function)
    */

    s16 Param = Rom.ReadBytes<s16>(Start + 2);
    u32 Func = Rom.ReadBytes<u32>(Start + 4);

    std::string FuncName = GetLabelFromMap(Func);
    if (FuncName == "geo_movtex_draw_water_regions") {
        Script.AreaDatas[Area].WaterBoxParams.push_back(Param);
    }

    std::string OutArgs = std::format(
        "{}, {}",
        Param, FuncName
    );

    return OutArgs;
};

std::string GeoCmdBackground(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x19: Create background scene graph node
     *   0x02: s16 background: background ID, or RGBA5551 color if backgroundFunc is null
     *   0x04: GraphNodeFunc backgroundFunc
     *
    #define GEO_BACKGROUND(background, function) \
        CMD_BBH(0x19, 0x00, background), \
        CMD_PTR(function)
    #define GEO_BACKGROUND_COLOR(background) \
        GEO_BACKGROUND(background, NULL)
    */

    s16 Background = Rom.ReadBytes<s16>(Start + 2);
    u32 Func = Rom.ReadBytes<u32>(Start + 4);

    std::string FuncName = "geo_skybox_main";
    std::string OutArgs = "";
    if (Func) {
        std::string SkyboxName;

        if (ExportSkybox(Script, SkyboxName)) {
            OutArgs = std::format("{}, geo_skybox_main", SkyboxName);
        } else {
            OutArgs = std::format("{}, geo_skybox_main", Background);
        }
    } else {
        OutArgs = std::format("{}", Background);
    }

    return OutArgs;
};

std::string GeoCmdHeldObject(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x1C: Create a held object scene graph node
     *  cmd+0x01: u8 unused
     *  cmd+0x02: s16 offsetX
     *  cmd+0x04: s16 offsetY
     *  cmd+0x06: s16 offsetZ
     *  cmd+0x08: GraphNodeFunc nodeFunc
     *
    #define GEO_HELD_OBJECT(param, ux, uy, uz, nodeFunc) \
        CMD_BBH(0x1C, param, ux), \
        CMD_HH(uy, uz), \
        CMD_PTR(nodeFunc)
    */

    u8 Unused = Rom.ReadBytes<u8>(Start + 1);
    s16 OffX = Rom.ReadBytes<s16>(Start + 2);
    s16 OffY = Rom.ReadBytes<s16>(Start + 4);
    s16 OffZ = Rom.ReadBytes<s16>(Start + 6);
    u32 Func = Rom.ReadBytes<u32>(Start + 8);

    std::string FuncName = GetLabelFromMap(Func);

    std::string OutArgs = std::format(
        "{}, {}, {}, {}, {}",
        Unused, OffX, OffY, OffZ, FuncName
    );

    return OutArgs;
};

std::string GeoCmdScale(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x1D: Create scale scene graph node with optional display list
     *   0x01: u8 params
     *     0b1000_0000: if set, enable displayList field and drawingLayer
     *     0b0000_1111: drawingLayer
     *   0x02-0x03: unused
     *   0x04: u32 scale (0x10000 = 1.0)
     *   0x08: [u32 displayList: if MSbit of params is set, display list segment address]
     *
    #define GEO_SCALE(layer, scale) \
        CMD_BBH(0x1D, layer, 0x0000), \
        CMD_W(scale)
    #define GEO_SCALE_WITH_DL(layer, scale, displayList) \
        CMD_BBH(0x1D, (layer | 0x80), 0x0000), \
        CMD_W(scale), \
        CMD_PTR(displayList)
    */

    u8 Layer = Rom.ReadBytes<u8>(Start + 1);
    u32 Scale = Rom.ReadBytes<u32>(Start + 4);

    std::string OutArgs = "";
    if (Layer & 0x80) {
        u32 DisplayList = Rom.ReadBytes<u32>(Start + 8);
        std::string DisplayListName = "0";
        if (DisplayList) {
            DisplayListName = GetGeoDLName(Script, Area, DisplayList);
            Script.AddDisplayList(DisplayList, Area);
        }

        OutArgs = std::format(
            "{}, {}, {}",
            GetGeoLayerName(Layer & ~0x80), Scale, DisplayListName
        );
    } else {
        OutArgs = std::format(
            "{}, {}",
            GetGeoLayerName(Layer), Scale
        );
    }

    return OutArgs;
};

std::string GeoCmdCullRadius(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) {
    /**
     * 0x20: Create a scene graph node that specifies for an object the radius that
     * is used for frustum culling.
     *   0x01: unused
     *   0x02: s16 cullingRadius
     *
    #define GEO_CULLING_RADIUS(cullingRadius) \
        CMD_BBH(0x20, 0x00, cullingRadius)
    */

    s16 CullRadius = Rom.ReadBytes<s16>(Start + 2);

    std::string OutArgs = std::format(
        "{}",
        CullRadius
    );

    return OutArgs;
};

std::string (*GeoCommandsFunctions[])(N64Rom &Rom, LevelScript &Script, u32 &Start, u8 Area) = {
    GeoCmdBranchAndLink,  GeoCmdEnd,             GeoCmdBranch,          GeoCmdReturn,
    GeoCmdStub,           GeoCmdStub,            (nullptr),             (nullptr),
    GeoCmdScreenArea,     GeoCmdOrtho,           GeoCmdCamFrustum,      GeoCmdStub,
    GeoCmdZbuffer,        GeoCmdRenderRange,     GeoCmdSwitchCase,      GeoCmdCamera,
    GeoCmdTransRot,       GeoCmdTransNode,       GeoCmdRotNode,         GeoCmdAnimatedPart,
    GeoCmdBillboard,      GeoCmdDisplayList,     GeoCmdShadow,          GeoCmdStub,
    GeoCmdASM,            GeoCmdBackground,      (nullptr),             (nullptr),
    GeoCmdHeldObject,     GeoCmdScale,           (nullptr),             (nullptr),
    GeoCmdCullRadius,     (nullptr),             (nullptr),             (nullptr),
};


u8 GetGeolayoutCmdSize(N64Rom &Rom, u32 Entry) {
    u8 Cmd = Rom.ReadBytes<u8>(Entry);
    switch (Cmd) {
        case 0x13:
        case 0x08:
            return 12;
        case 0x0F:
            return 20;
        case 0x0A: { // GEO_CAMERA_FRUSTUM
            u8 FuncEnable = Rom.ReadBytes<u8>(Entry + 1);
            if (FuncEnable) return 12;
            return 8;
        }
        case 0x11: // GEO_TRANSLATE_NODE
        case 0x12: // GEO_ROTATION_NODE
        case 0x14: // GEO_BILLBOARD
        case 0x1D: { // GEO_SCALE
            u8 Layer = Rom.ReadBytes<u8>(Entry + 1);
            if (Layer & 0x80) return 12;
            return 8;
        }
        case 0x10: { // GEO_TRANSLATE_ROTATE
            u8 Layer = Rom.ReadBytes<u8>(Entry + 1);
            if (Layer & 0x30) {
                if (Layer & 0x80) return 8;
                return 4;
            } else if (Layer & 0x20 || Layer & 0x10) {
                if (Layer & 0x80) return 12;
                return 8;
            } else {
                if (Layer & 0x80) return 20;
                return 16;
            }
        }
        case 0x1C:
            return 12;
        case 0x15:
        case 0x16:
        case 0x19:
        case 0x18:
        case 0x02:
        case 0x00:
        case 0x0E:
        case 0x0D:
            return 8;
    }
    return 4;
}

std::string GetSpecialGeoCmdName(N64Rom &Rom, u8 Cmd, u32 Start) {
    switch (Cmd) {
        case 0x0A: {
            u32 Enable = Rom.ReadBytes<u8>(Start + 1);
            if (Enable) {
                return "GEO_CAMERA_FRUSTUM_WITH_FUNC";
            } else {
                return "GEO_CAMERA_FRUSTUM";
            }
            break;
        }
        case 0x19: {
            u32 Pointer = Rom.ReadBytes<u32>(Start + 4);
            if (!Pointer) {
                return "GEO_BACKGROUND_COLOR";
            } else {
                return "GEO_BACKGROUND";
            }
            break;
        }
        case 0x11: {
            u8 Layer = Rom.ReadBytes<u8>(Start + 1);
            if (Layer & 0x80) {
                return "GEO_TRANSLATE_NODE_WITH_DL";
            } else {
                return "GEO_TRANSLATE_NODE";
            }
            break;
        }
        case 0x12: {
            u8 Layer = Rom.ReadBytes<u8>(Start + 1);
            if (Layer & 0x80) {
                return "GEO_ROTATION_NODE_WITH_DL";
            } else {
                return "GEO_ROTATION_NODE";
            }
            break;
        }
        case 0x14: {
            u8 Layer = Rom.ReadBytes<u8>(Start + 1);
            if (Layer & 0x80) {
                return "GEO_BILLBOARD_WITH_PARAMS_AND_DL";
            } else {
                return "GEO_BILLBOARD_WITH_PARAMS";
            }
            break;
        }
        case 0x1D: {
            u8 Layer = Rom.ReadBytes<u8>(Start + 1);
            if (Layer & 0x80) {
                return "GEO_SCALE_WITH_DL";
            } else {
                return "GEO_SCALE";
            }
            break;
        }
        case 0x10: {
            std::string OutName;
            u8 Layer = Rom.ReadBytes<u8>(Start + 1);
            if (Layer & 0x30) {
                OutName = "GEO_ROTATE_Y";
            } else if (Layer & 0x20) {
                OutName = "GEO_ROTATE";
            } else if (Layer & 0x10) {
                OutName = "GEO_TRANSLATE";
            } else {
                OutName = "GEO_TRANSLATE_ROTATE";
            }

            if (Layer & 0x80) {
                OutName += "_WITH_DL";
            }
            return OutName;
        }
        default: return GeoCommandsName[Cmd];
    }
}

static void WriteGeoLayoutRecursive(FILE *GeoDump, N64Rom &Rom, u8 Area, std::string LvlName, u32 SegAddr, u32 Entry, LevelScript &Script) {
    if (!SegAddr || ProcessedGeos[SegAddr]) return;
    ProcessedGeos[SegAddr] = true;
    u32 ScanEntry = Entry;
    u32 CommandCount = 0; // for validating... kaze's shitty geolayouts

    while (true) {
        u8 Cmd = Rom.ReadBytes<u8>(ScanEntry);
        u8 Len = GetGeolayoutCmdSize(Rom, ScanEntry);
        
        if (Cmd == 0x02 || Cmd == 0x00) {
            u32 NewSegAddr = Rom.ReadBytes<u32>(ScanEntry + 4);
            if (ValidateMemAddr(NewSegAddr)) {
                WriteGeoLayoutRecursive(GeoDump, Rom, Area, LvlName, NewSegAddr, NewSegAddr, Script);
            }
        }

        if (Cmd == 0x01 || Cmd == 0x03) break;
        if (Cmd == 0x02 && Rom.ReadBytes<u8>(ScanEntry + 1) == 0) break;
        if (++CommandCount > 10000 /*|| Rom.ReadBytes<u32>(ScanEntry) == 0x00000000*/) {
            printf("Geolayout 0x%x is broken, ignoring export\n", SegAddr);
            break;
        }
        ScanEntry += Len;
    }

    if (Script.CurrentActor) {
        if (SegAddr == Script.CurrentActor->Addr) {
            fprintf(GeoDump, "const GeoLayout %s[] = {\n", LvlName.c_str());
        } else {
            fprintf(GeoDump, "const GeoLayout %s_geo_%x[] = {\n", LvlName.c_str(), SegAddr);
        }
    } else {
        fprintf(GeoDump, "const GeoLayout %s_area_%u_geo_%x[] = {\n", LvlName.c_str(), Area, SegAddr);
    }

    CommandCount = 0;
    while (true) {
        u8 Cmd = Rom.ReadBytes<u8>(Entry);
        if (Cmd > 35) {
            printf("Unknown GeoLayout Command 0x%x at address 0x%x\n", Cmd, Entry);
            break;
        }
        u8 Len = GetGeolayoutCmdSize(Rom, Entry);

        if (GeoCommandsFunctions[Cmd]) {
            std::string CmdName = GetSpecialGeoCmdName(Rom, Cmd, Entry);
            std::string Args = GeoCommandsFunctions[Cmd](Rom, Script, Entry, Area);
            fprintf(GeoDump, "    %s(%s),\n", CmdName.c_str(), Args.c_str());

            if (Cmd == 0x01 || Cmd == 0x03) break;
            if (Cmd == 0x02 && Rom.ReadBytes<u8>(Entry + 1) == 0) break;
        } else {
            printf("Unimplemented GeoLayout command 0x%x at address 0x%x\n", Cmd, Entry);
            break;
        }
        if (++CommandCount > 10000/* || Rom.ReadBytes<u32>(Entry) == 0x00000000*/) {
            printf("Geolayout 0x%x is broken, ignoring export\n", SegAddr);
            break;
        }
        Entry += Len;
    }

    fprintf(GeoDump, "};\n\n");
}

void ExportGeolayout(N64Rom &Rom, u8 Area, const std::string &LvlName, u32 SegAddr, u32 Entry, LevelScript &Script, const char *FilePath) {
    FILE *GeoDump = fopen(FilePath, "w");

    ProcessedGeos.clear();

    WriteGeoLayoutRecursive(GeoDump, Rom, Area, LvlName, SegAddr, Entry, Script);

    fclose(GeoDump);
}