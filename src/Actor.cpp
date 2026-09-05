#include "Actor.h"
#include "LevelScript.h"
#include "GeoLayout.h"
#include "Model.h"
#include "Memory.h"

std::map<u32, bool> ExportedActors;

void ExportActors(N64Rom &Rom, LevelScript &Script) {
    std::string ActorsPath = "output/actors";
    fs::create_directories(ActorsPath);

    for (auto &Act : Script.Actors) {
        if (!ValidateMemAddr(Act.Addr)) {
            printf("Actor 0x%08x has an invalid address, Skipping\n", Act.Addr);
            continue;
        }
        if (ExportedActors[Act.Addr] && (Act.Addr >> 24) == 0) continue;
        
        std::string ActorFolder = (ActorsPath + "/" + Act.Name);
        fs::create_directories(ActorFolder);

        Script.CurrentActor = &Act;

        printf("Exporting actor %s\n", Act.Name.c_str());
        if (!Act.IsDL) {
            std::string GeoDumpPath = ActorFolder + "/geo.inc.c";
            ExportGeolayout(Rom, 0, Act.Name, Act.Addr, Act.Addr, Script, GeoDumpPath.c_str());
        } else {
            Act.DisplayLists.push_back(Act.Addr);
        }

        std::string ModelDumpPath = ActorFolder + "/model.inc.c";
        ExportModels(Rom, Script, "", 0, ModelDumpPath.c_str(), true, &Act);

        Script.CurrentActor = nullptr;
        ExportedActors[Act.Addr] = true;
    }
}