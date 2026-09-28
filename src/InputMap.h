#pragma once
#include <raylib.h>
#include <unordered_map>
#include <string>
#include <vector>

struct KeyBinding {
    int key = 0;          // raylib key code (KEY_W, KEY_SPACE, etc.)
    bool alt = false;     // alternative key (for double binding)
};

enum class Action {
    // Movement
    MoveUp, MoveDown, MoveLeft, MoveRight,
    Sprint, Jump,
    
    // Combat
    Skill1, Skill2, Skill3, Skill4, Skill5, Skill6,
    Interact,           // E - equip/dialog/portal
    
    // UI Navigation
    NavUp, NavDown, NavLeft, NavRight,
    NavSelect,          // Enter/Space
    NavCancel,          // Escape
    
    // UI
    Inventory, Equipment, QuestLog, SkillTree, Crafting, Shop,
    Map, Pause, Settings,
    
    // RTS
    RTSSelectModifier,  // SHIFT - RTS selection
    
    // Debug
    ToggleBot, CycleZone, InfernoWarp,
    
    COUNT
};

class InputMap {
public:
    InputMap() { loadDefaults(); }

    // Get key for action (checks primary then alt)
    bool isPressed(Action action) const {
        const auto& b = bindings[(int)action];
        if (b.key && IsKeyDown(b.key)) return true;
        if (b.alt && IsKeyDown(b.alt)) return true;
        return false;
    }
    
    bool isPressedOnce(Action action) const {
        const auto& b = bindings[(int)action];
        if (b.key && IsKeyPressed(b.key)) return true;
        if (b.alt && IsKeyPressed(b.alt)) return true;
        return false;
    }

    // Rebinding
    void bind(Action action, int key, bool isAlt = false) {
        if (isAlt) bindings[(int)action].alt = key;
        else bindings[(int)action].key = key;
    }
    
    void unbind(Action action, bool isAlt = false) {
        if (isAlt) bindings[(int)action].alt = 0;
        else bindings[(int)action].key = 0;
    }

    // Get display name for key
    static const char* keyName(int key) {
        if (key == 0) return "—";
        switch (key) {
            case KEY_SPACE: return "SPACE";
            case KEY_LEFT_SHIFT: case KEY_RIGHT_SHIFT: return "SHIFT";
            case KEY_LEFT_CONTROL: case KEY_RIGHT_CONTROL: return "CTRL";
            case KEY_LEFT_ALT: case KEY_RIGHT_ALT: return "ALT";
            case KEY_ENTER: return "ENTER";
            case KEY_ESCAPE: return "ESC";
            case KEY_TAB: return "TAB";
            case KEY_BACKSPACE: return "BACKSPACE";
            case KEY_INSERT: return "INS";
            case KEY_DELETE: return "DEL";
            case KEY_HOME: return "HOME";
            case KEY_END: return "END";
            case KEY_PAGE_UP: return "PGUP";
            case KEY_PAGE_DOWN: return "PGDN";
            case KEY_UP: return "UP";
            case KEY_DOWN: return "DOWN";
            case KEY_LEFT: return "LEFT";
            case KEY_RIGHT: return "RIGHT";
            case KEY_F1: return "F1"; case KEY_F2: return "F2"; case KEY_F3: return "F3";
            case KEY_F4: return "F4"; case KEY_F5: return "F5"; case KEY_F6: return "F6";
            case KEY_F7: return "F7"; case KEY_F8: return "F8"; case KEY_F9: return "F9";
            case KEY_F10: return "F10"; case KEY_F11: return "F11"; case KEY_F12: return "F12";
            case MOUSE_BUTTON_LEFT: return "LMB"; case MOUSE_BUTTON_RIGHT: return "RMB";
            case MOUSE_BUTTON_MIDDLE: return "MMB";
            default:
                if (key >= KEY_A && key <= KEY_Z) {
                    static char buf[2] = {0, 0};
                    buf[0] = 'A' + (key - KEY_A);
                    return buf;
                }
                if (key >= KEY_ZERO && key <= KEY_NINE) {
                    static char buf[2] = {0, 0};
                    buf[0] = '0' + (key - KEY_ZERO);
                    return buf;
                }
                return "?";
        }
    }

    // Get binding display string for UI
    std::string getBindingString(Action action) const {
        const auto& b = bindings[(int)action];
        if (b.key == 0 && b.alt == 0) return "Unbound";
        if (b.alt == 0) return keyName(b.key);
        return std::string(keyName(b.key)) + " / " + keyName(b.alt);
    }

    // Save/Load from JSON-like format (simple string map)
    void save(std::unordered_map<std::string, std::string>& out) const {
        static const std::string actionNames[] = {
            "MoveUp", "MoveDown", "MoveLeft", "MoveRight", "Sprint", "Jump",
            "Skill1", "Skill2", "Skill3", "Skill4", "Skill5", "Skill6",
            "Interact",
            "NavUp", "NavDown", "NavLeft", "NavRight", "NavSelect", "NavCancel",
            "Inventory", "Equipment", "QuestLog", "SkillTree", "Crafting", "Shop",
            "Map", "Pause", "Settings", "RTSSelectModifier",
            "ToggleBot", "CycleZone", "InfernoWarp"
        };
        for (int i = 0; i < (int)Action::COUNT; ++i) {
            const auto& b = bindings[i];
            out[actionNames[i] + "_primary"] = std::to_string(b.key);
            out[actionNames[i] + "_alt"] = std::to_string(b.alt);
        }
    }

    void load(const std::unordered_map<std::string, std::string>& in) {
        static const std::string actionNames[] = {
            "MoveUp", "MoveDown", "MoveLeft", "MoveRight", "Sprint", "Jump",
            "Skill1", "Skill2", "Skill3", "Skill4", "Skill5", "Skill6",
            "Interact",
            "NavUp", "NavDown", "NavLeft", "NavRight", "NavSelect", "NavCancel",
            "Inventory", "Equipment", "QuestLog", "SkillTree", "Crafting", "Shop",
            "Map", "Pause", "Settings", "RTSSelectModifier",
            "ToggleBot", "CycleZone", "InfernoWarp"
        };
        for (int i = 0; i < (int)Action::COUNT; ++i) {
            auto itP = in.find(actionNames[i] + "_primary");
            if (itP != in.end()) bindings[i].key = std::stoi(itP->second);
            auto itA = in.find(actionNames[i] + "_alt");
            if (itA != in.end()) bindings[i].alt = std::stoi(itA->second);
        }
    }

private:
    KeyBinding bindings[(int)Action::COUNT]{};

    void loadDefaults() {
        // Movement
        bind(Action::MoveUp,    KEY_W);
        bind(Action::MoveDown,  KEY_S);
        bind(Action::MoveLeft,  KEY_A);
        bind(Action::MoveRight, KEY_D);
        bind(Action::Sprint,    KEY_LEFT_SHIFT);
        bind(Action::Jump,      KEY_SPACE);
        
        // Combat skills
        bind(Action::Skill1, KEY_ONE);
        bind(Action::Skill2, KEY_TWO);
        bind(Action::Skill3, KEY_THREE);
        bind(Action::Skill4, KEY_FOUR);
        bind(Action::Skill5, KEY_FIVE);
        bind(Action::Skill6, KEY_SIX);
        
        // Interaction (E) - now configurable
        bind(Action::Interact, KEY_E);
        
        // Navigation
        bind(Action::NavUp,    KEY_UP);
        bind(Action::NavDown,  KEY_DOWN);
        bind(Action::NavLeft,  KEY_LEFT);
        bind(Action::NavRight, KEY_RIGHT);
        bind(Action::NavSelect, KEY_ENTER);
        bind(Action::NavSelect, KEY_SPACE, true);  // Alt: Space
        bind(Action::NavCancel, KEY_ESCAPE);
        
        // UI
        bind(Action::Inventory, KEY_I);
        bind(Action::Equipment, KEY_E, true);  // Alt: E
        bind(Action::QuestLog, KEY_Q);
        bind(Action::SkillTree, KEY_X);
        bind(Action::Crafting, KEY_C);
        bind(Action::Shop, KEY_P);
        bind(Action::Map, KEY_M);
        bind(Action::Pause, KEY_ESCAPE);
        bind(Action::Settings, KEY_F1);
        
        // RTS
        bind(Action::RTSSelectModifier, KEY_LEFT_SHIFT);
        bind(Action::RTSSelectModifier, KEY_RIGHT_SHIFT, true);
        
        // Debug
        bind(Action::ToggleBot, KEY_F12);
        bind(Action::CycleZone, KEY_P);
        bind(Action::InfernoWarp, KEY_F9);
    }
};