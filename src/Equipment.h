#pragma once
#include <string>
#include <raylib.h>
#include "Item.h"

enum class EquipSlot { None, Weapon, Armor, Implant };

struct Equipment {
    std::string name;
    std::string description;
    EquipSlot   slot        = EquipSlot::None;
    float       primary     = 0.0f;   // dmg for weapon, maxHP for armor, speed for implant
    float       secondary   = 0.0f;   // range for weapon, defense% for armor, xpMult for implant
    Color       color       = WHITE;
    int         tier        = 0;

    int upgradeLevel = 0;  // 0-3, each level +30% stats

    // Afixos (bônus de itens raros/epicos/lendarios) — aplicados em applyEquipmentStats
    float       bonusDamage    = 0.0f;
    float       bonusHealth    = 0.0f;
    float       bonusSpeed     = 0.0f;
    float       bonusDefense   = 0.0f;
    float       bonusCrit      = 0.0f;
    float       bonusVampirism = 0.0f;
    std::string affixPrefix;
    std::string affixSuffix;
    std::string baseName;
    int         affixCount     = 0;

    // ID estavel (save/load) — NUNCA muda; name pode ser renomeado. Fica no FIM
    // da struct com default vazio: itens craftados (agregados sem id) continuam
    // compilando e sao salvos pelo nome, como antes.
    std::string id;

    bool  isEmpty()            const { return slot == EquipSlot::None; }
    float getEffectivePrimary()   const { return primary   * (1.0f + upgradeLevel * 0.30f); }
    float getEffectiveSecondary() const { return secondary * (1.0f + upgradeLevel * 0.30f); }
    int   upgradeCost()        const {
        if (upgradeLevel >= 3) return 0;
        int costs[3] = {100, 300, 600};
        return costs[upgradeLevel];
    }
    bool  canUpgrade()         const { return !isEmpty() && upgradeLevel < 3; }
};

namespace EDB {
    inline Equipment make(const std::string& name, const std::string& desc,
                          EquipSlot slot, float primary, float secondary,
                          Color color, int tier, int upgradeLevel, const std::string& id) {
        Equipment e;
        e.name = name; e.description = desc; e.slot = slot;
        e.primary = primary; e.secondary = secondary;
        e.color = color; e.tier = tier; e.upgradeLevel = upgradeLevel; e.id = id;
        return e;
    }

    // ── Armas (Tier 1) ────────────────────────────────────────────────────────
    inline Equipment pistolaPlas()    { return make("Pistola Plasma",     "Dano +15, Alc +20",   EquipSlot::Weapon,  15,  20,  {0,200,255,255},   1, 0, "pistola_plasma"); }
    inline Equipment submetMilitar()  { return make("Submetralhadora Mil","Dano +22, Alc +10",   EquipSlot::Weapon,  22,  10,  {180,200,120,255},  1, 0, "submet_militar"); }
    // ── Armas (Tier 2) ────────────────────────────────────────────────────────
    inline Equipment rifleEnergia()   { return make("Rifle de Energia",   "Dano +35, Alc +40",   EquipSlot::Weapon,  35,  40,  {0,255,150,255},    2, 0, "rifle_energia"); }
    inline Equipment shotgunPlasma()  { return make("Shotgun Plasma",     "Dano +55, Alc -10",   EquipSlot::Weapon,  55, -10,  {255,150,0,255},    2, 0, "shotgun_plasma"); }
    inline Equipment espadaEnergia()  { return make("Espada de Energia",  "Dano +50, Alc +25",   EquipSlot::Weapon,  50,  25,  {255,50,200,255},   2, 0, "espada_energia"); }
    // ── Armas (Tier 3) ────────────────────────────────────────────────────────
    inline Equipment canhaoEMP()      { return make("Canhao EMP",         "Dano +70, Alc +60",   EquipSlot::Weapon,  70,  60,  {255,200,0,255},    3, 0, "canhao_emp"); }
    inline Equipment railgunSkynet()  { return make("Railgun Skynet",     "Dano +100, Alc +80",  EquipSlot::Weapon, 100,  80,  {200,0,255,255},    3, 0, "railgun_skynet"); }
    inline Equipment canhaoAnti()     { return make("Canhao Anti-Maquina","Dano +130, Alc +55",  EquipSlot::Weapon, 130,  55,  {255,80,0,255},     3, 0, "canhao_antimaquina"); }

    // ── Armaduras ─────────────────────────────────────────────────────────────
    inline Equipment coleteMilitar()  { return make("Colete Militar",     "+50 HP, Def 5%",      EquipSlot::Armor,   50,  5,   {120,120,120,255},  1, 0, "colete_militar"); }
    inline Equipment armaduraAvan()   { return make("Armadura Avancada",  "+120 HP, Def 15%",    EquipSlot::Armor,  120,  15,  {100,150,220,255},  2, 0, "armadura_avancada"); }
    inline Equipment exoesqueleto()   { return make("Exoesqueleto Titan", "+250 HP, Def 30%",    EquipSlot::Armor,  250,  30,  {200,200,255,255},  3, 0, "exoesqueleto_titan"); }
    inline Equipment nanoMalha()      { return make("Nano Malha T-1000",  "+180 HP, Def 25%",    EquipSlot::Armor,  180,  25,  {0,220,200,255},    3, 0, "nano_malha"); }

    // ── Implants ──────────────────────────────────────────────────────────────
    inline Equipment chipVel()        { return make("Chip de Velocidade", "Vel +60",             EquipSlot::Implant, 60,  0,   {255,100,255,255},  1, 0, "chip_velocidade"); }
    inline Equipment neuralLink()     { return make("Neural Link",        "Vel +40, XP x1.5",    EquipSlot::Implant, 40,  1.5f,{150,255,200,255},  2, 0, "neural_link"); }
    inline Equipment quantumCore()    { return make("Quantum Core",       "Vel +80, XP x2.0",    EquipSlot::Implant, 80,  2.0f,{255,255,100,255},  3, 0, "quantum_core"); }
    inline Equipment adrenChip()      { return make("Adrenal Override",   "Vel +100, XP x1.8",   EquipSlot::Implant,100,  1.8f,{255,50,100,255},   3, 0, "adrenal_override"); }

    // Resolve pelo ID estavel (save/load). Retorna Equipment vazio se desconhecido.
    inline Equipment byId(const std::string& id) {
        static const Equipment all[] = {
            pistolaPlas(), submetMilitar(), rifleEnergia(), shotgunPlasma(),
            espadaEnergia(), canhaoEMP(), railgunSkynet(), canhaoAnti(),
            coleteMilitar(), armaduraAvan(), exoesqueleto(), nanoMalha(),
            chipVel(), neuralLink(), quantumCore(), adrenChip()
        };
        for (const auto& e : all) if (e.id == id) return e;
        return {};
    }

    // Sorteio por TIER — usado pela recompensa de fim de fase. Mantido aqui pra
    // ficar junto do catalogo: quem adicionar item novo ve este sorteio na hora.
    inline Equipment randomForTier(int tier) {
        if (tier < 1) tier = 1;
        if (tier > 3) tier = 3;
        Equipment t1[] = { pistolaPlas(), submetMilitar(), coleteMilitar(), chipVel() };
        Equipment t2[] = { rifleEnergia(), armaduraAvan(), neuralLink() };
        Equipment t3[] = { exoesqueleto(), nanoMalha(), quantumCore(), adrenChip() };
        if (tier == 1) return t1[GetRandomValue(0, 3)];
        if (tier == 2) return t2[GetRandomValue(0, 2)];
        return t3[GetRandomValue(0, 3)];
    }

    // Gera equipamento com afixos baseados na raridade (para drops de elite/boss)
    inline Equipment withRandomAffixes(Equipment eq, ItemRarity rarity) {
        int affixRolls = 0;
        switch (rarity) {
            case ItemRarity::Common:     affixRolls = 0; break;
            case ItemRarity::Uncommon:   affixRolls = 1; break;
            case ItemRarity::Rare:       affixRolls = 2; break;
            case ItemRarity::Epic:       affixRolls = 3; break;
            case ItemRarity::Legendary:  affixRolls = 4; break;
            case ItemRarity::Omega:      affixRolls = 5; break;
        }
        for (int i = 0; i < affixRolls; ++i) {
            int roll = GetRandomValue(0, 5);
            float value = 0.0f;
            switch (rarity) {
                case ItemRarity::Common:     value = 0.0f; break;
                case ItemRarity::Uncommon:   value = 3.0f + GetRandomValue(0, 2); break;
                case ItemRarity::Rare:       value = 6.0f + GetRandomValue(0, 4); break;
                case ItemRarity::Epic:       value = 10.0f + GetRandomValue(0, 6); break;
                case ItemRarity::Legendary:  value = 16.0f + GetRandomValue(0, 8); break;
                case ItemRarity::Omega:      value = 24.0f + GetRandomValue(0, 12); break;
            }
            switch (roll) {
                case 0: eq.bonusDamage    += value; break;
                case 1: eq.bonusHealth    += value; break;
                case 2: eq.bonusSpeed     += value; break;
                case 3: eq.bonusDefense   += value; break;
                case 4: eq.bonusCrit      += value / 100.0f; break;
                case 5: eq.bonusVampirism += value / 100.0f; break;
            }
        }
        if (affixRolls > 0) {
            static const char* prefixes[] = { "Flaming", "Frozen", "Venomous", "Arcane", "Void", "Primal", "Celestial", "Infernal" };
            static const char* suffixes[] = { "of Power", "of Speed", "of Life", "of the Hunt", "of Ages", "of the Void", "of Titans", "of Legends" };
            eq.affixPrefix  = prefixes[GetRandomValue(0, 7)];
            eq.affixSuffix  = suffixes[GetRandomValue(0, 7)];
            eq.affixCount   = affixRolls;
        }
        return eq;
    }

};