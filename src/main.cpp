// Main code files
#include "main.hpp"
#include "ModConfig.hpp"

// Hook files
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "System/Action.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "UnityEngine/Vector3.hpp"

// Dependency files
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"
#include "bs-utils/shared/utils.hpp"
#include "bs-utils/shared/AssetBundle.hpp"


using namespace GlobalNamespace;
using namespace QuestUI;

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup
DEFINE_CONFIG(ModConfig);

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

#define INFO_LOG(value...)  getLogger().WithContext("INFO").info(value)

float newWorldRotation = 0.0f;
UnityEngine::Vector3 newMoveStartPos;  //   What moveStartPos is changed to when toggle is enabled
UnityEngine::Vector3 newMoveEndPos;    //   What moveEndPos is changed to
bool worldRotationOn = false;
bool changeStartPos = false;
bool modEnabled = false;
UnityEngine::Vector3 moveStartPos;
UnityEngine::Vector3 originalNoteMoveStartPos;
UnityEngine::Vector3 originalWallMoveStartPos;
UnityEngine::Vector3 newJumpEndPos;

void _UPDATE_SCORE_SUBMISSION(){                    // Toggle score submission if modEnabled (in ModConfig) is enabled
    if(getModConfig().modEnabled.GetValue())
        {bs_utils::Submission::disable(modInfo);}
    else
        {bs_utils::Submission::enable(modInfo);}}

MAKE_HOOK_MATCH(NoteController_Init, &GlobalNamespace::NoteController::Init, void, GlobalNamespace::NoteController* self, GlobalNamespace::NoteData* NoteData, float worldRotation, UnityEngine::Vector3 moveStartPos, UnityEngine::Vector3 moveEndPos, UnityEngine::Vector3 jumpEndPos, float moveDuration, float jumpDuration, float jumpGravity, float endRotation, float uniformScale)
{
    originalNoteMoveStartPos = moveStartPos;
    if(getModConfig().modEnabled.GetValue())
    {
        if(getModConfig().worldRotationOn.GetValue())
        {newWorldRotation = newWorldRotation + 1.0f;}   // Rotate world by 1 degree

        if(getModConfig().changeStartPos.GetValue())
        {newMoveStartPos = UnityEngine::Vector3(0, -1, -1);}    // Cause notes to spawn beneath you

        else
        {newMoveStartPos = UnityEngine::Vector3(originalNoteMoveStartPos);}  // Set note spawn point back to original value
        moveStartPos = newMoveStartPos;
        worldRotation = newWorldRotation;
        uniformScale = getModConfig().newUniformScale.GetValue();
    }
    

    NoteController_Init(self, NoteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration, jumpGravity, endRotation, uniformScale);
}


MAKE_HOOK_MATCH(ObstacleController_Init, &GlobalNamespace::ObstacleController::Init, void, GlobalNamespace::ObstacleController* self, GlobalNamespace::ObstacleData* ObstacleData, float worldRotation, UnityEngine::Vector3 startPos, UnityEngine::Vector3 midPos, UnityEngine::Vector3 endPos, float move1Duration, float move2Duration, float singleLineWidth, float height)
{
    originalWallMoveStartPos = moveStartPos;

    if(getModConfig().modEnabled.GetValue())
    {
        if(getModConfig().worldRotationOn.GetValue())
        {newWorldRotation = newWorldRotation + 1.0f;}   // Rotate world by 1 degree

        if(getModConfig().changeStartPos.GetValue())
        {newMoveStartPos = UnityEngine::Vector3(0, -3, 5);}    // Cause walls to spawn beneath you

        else
        {newMoveStartPos = UnityEngine::Vector3(originalNoteMoveStartPos);}     // Set wall spawn point back to original value
        moveStartPos = newMoveStartPos;
        worldRotation = newWorldRotation;
    }


    ObstacleController_Init(self, ObstacleData, worldRotation, startPos, midPos, endPos, move1Duration, move2Duration, singleLineWidth, height);
}


MAKE_HOOK_MATCH(BeatmapObjectSpawnMovementData_Init, &GlobalNamespace::BeatmapObjectSpawnMovementData::Init, void, GlobalNamespace::BeatmapObjectSpawnMovementData* self, int noteLinesCount, float startNoteJumpMovementSpeed, float startBpm, float noteJumpStartBeatOffset, float jumpOffsetY, UnityEngine::Vector3 rightVec, UnityEngine::Vector3 forwardVec)
{
    newWorldRotation = 0.0f;
    newMoveStartPos = originalNoteMoveStartPos;
    getLogger().info("BOSMD Called!");
    if (getModConfig().modEnabled.GetValue())
    {
    
        getLogger().info("Disabling score submission because mod is enabled");
        bs_utils::Submission::disable(modInfo);
    } 
    else
    {
        getLogger().info("Enabling score submission because mod is disabled");
        bs_utils::Submission::enable(modInfo);
    }
    

    BeatmapObjectSpawnMovementData_Init(self, noteLinesCount, startNoteJumpMovementSpeed, startBpm, noteJumpStartBeatOffset, jumpOffsetY, rightVec, forwardVec);
}

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
    if(firstActivation){
        UnityEngine::GameObject* container = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        BeatSaberUI::CreateText(container->get_transform(), "NoteController v1 - AllyPallyUK");

        AddConfigValueToggle(container->get_transform(), getModConfig().modEnabled);

        AddConfigValueToggle(container->get_transform(), getModConfig().worldRotationOn);
        
        AddConfigValueToggle(container->get_transform(), getModConfig().changeStartPos);
        
        //std::u16string_view text, int decimals, float increment, float currentValue, float minValue, float maxValue, std::function<void(float)> onValueChange = nullptr);

        AddConfigValueIncrementFloat(container->get_transform(), getModConfig().newUniformScale, 1, 0.1f, 1.0f, 999.0f);
    }
    else{
        // If not first time activated
    }
};


// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
    
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}



// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();
    getModConfig().Init(modInfo);

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();                                                                    
    QuestUI::Register::RegisterModSettingsViewController(modInfo, DidActivate);         // <----- This Adds a button in the settings menu
    QuestUI::Register::RegisterMainMenuModSettingsViewController(modInfo, DidActivate); // <----- This Adds a button in the main menu
    getLogger().info("Successfully installed Settings UI!");

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(getLogger(), NoteController_Init);
    INSTALL_HOOK(getLogger(), ObstacleController_Init);
    INSTALL_HOOK(getLogger(), BeatmapObjectSpawnMovementData_Init);
    getLogger().info("Installed all hooks!");

};