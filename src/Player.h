#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include <cstdint>
#include "Item.h"
#include "Skill.h"
#include "Equipment.h"

struct PlayerSpeech {
    std::string text;
    float       duration  = 3.0f;
    float       timer     = 0.0f;
    bool        active    = false;
    Color       color     = {0, 220, 255, 255};
};

struct KillStreak {
    int   count      = 0;
    float resetTimer = 0.0f;
};

enum class EvolutionPath {
    None = 0,
    CyborgSoldier,
    HackerFantasma,
    ExecutorOmega
};

struct LevelUpChoice {
    std::string title;
    std::string description;
    int         statType;    // 0=maxHP 1=dmg 2=speed 3=defense 4=range 5=cdReduction
    float       bonusAmount;
};

// Classes jogaveis — cada uma tem aparencia, formato de corpo e stats proprios.
enum class CharacterClass {
    Soldado = 0,   // homem soldado — equilibrado, armadura, rifle
    Guerreira,     // mulher guerreira — rapida, agil, pistolas duplas
    Robo,          // robo de combate — muito HP/defesa, lento, pesado
    Mago,          // mago — pouca vida, MUITO dano de habilidade, cajado, tunica
    Bruxa,         // bruxa — equilibrada, chapeu, magia, varinha
    HomemFera,     // homem-fera — alto dano corpo a corpo, garras, pelos, rapido
    COUNT
};

class Player {
public:
    Vector2 position     = {0, 0};
    Vector2 velocity     = {0, 0};
    float   speed        = 250.0f;

    // Corrida e pulo (publicos — Game controla)
    float jumpZ      = 0.0f;   // altura atual do pulo (z)
    float jumpVel    = 0.0f;   // velocidade vertical do pulo
    bool  isJumping  = false;
    bool  sprinting  = false;  // Game seta quando segura correr
    void  startJump() { if (!isJumping) { isJumping = true; jumpVel = 360.0f; } }

    float   health       = 100.0f;
    float   maxHealth    = 100.0f;
    float   attackDamage = 25.0f;
    float   attackRange  = 90.0f;
    float   radius       = 16.0f;
    float   defense      = 0.0f;
    float   xpMultiplier = 1.0f;

    int level        = 1;
    int xp           = 0;
    int xpToNextLevel= 100;
    int credits      = 0;

    std::vector<Item>      inventory;
    std::vector<Skill>     skills;

    Equipment equippedWeapon;
    Equipment equippedArmor;
    Equipment equippedImplant;

    // Mochila de equipamentos guardados (gear nao equipado) — estilo Diablo:
    // o jogador escolhe o que equipar de uma lista em vez de descartar o antigo.
    std::vector<Equipment> equipBag;
    int  selectedBagEquip = 0;
    void equipFromBag(int idx);   // troca o item da bolsa pelo equipado do slot

    // Assinatura da APARENCIA (arma/armadura/implante/tinta). O modelo 3D e gerado
    // a partir do sprite 2D e fica CACHEADO: sem incluir isto na chave do cache, o
    // personagem continuava com o visual antigo depois de trocar de equipamento.
    int visualSignature() const;

    // ── Cosméticos (atualizados pelo Game: loja comum + skins premium de Gems) ──
    bool  hasCosmeticTint = false;          // tinta comprada com creditos
    Color cosmeticTint    = {255,255,255,255};
    bool  skinNeon        = false;          // skin_neon  (brilho neon aditivo)
    bool  skinDragon      = false;          // skin_dragon (tonalidade quente + brasas)
    bool  petDrone        = false;          // pet_drone  (drone flutuante ao lado)

    // ── Poder de cura (estilo Diablo 3: pocao com cooldown + regen buff) ───────
    float healCooldown = 0.0f;   // tempo restante ate poder usar de novo
    float regenTimer   = 0.0f;   // duracao do buff de regeneracao ativo
    void  usePotion();           // cura instantanea + ativa regeneracao (tecla Q)
    bool  potionReady() const { return healCooldown <= 0.0f; }

    // Buff timers
    float overloadTimer = 0.0f;
    float shieldTimer   = 0.0f;
    bool  inSafeRefuge  = false;   // setado pelo Game: invulnerável dentro da zona segura
    bool  reviveReady   = true;    // Protocolo Imortal (perk) — 1 uso por 60s
    float reviveTimer   = 0.0f;

    bool isOverloaded() const { return overloadTimer > 0.0f; }
    bool isShielded()   const { return shieldTimer   > 0.0f; }

    // ── Sistema de classes de personagem ─────────────────────────────────────
    CharacterClass charClass = CharacterClass::Soldado;
    float          skillPower = 1.0f;   // multiplicador de dano de habilidade da classe
    // Paleta visual da classe (definida em applyClass, usada no render)
    Color classPrimary   = {35, 55, 120, 255};
    Color classSecondary = {55, 90, 180, 255};
    Color classAccent    = {0, 210, 255, 255};
    Color classSkin      = {180, 135, 105, 255};
    Color classTrim      = {160, 170, 185, 255};

    void applyClass(CharacterClass c);                      // aplica stats/cores base
    static const char* className(CharacterClass c);        // nome p/ a tela de selecao
    static const char* classDescription(CharacterClass c); // descricao curta
    static const char* classFantasy(CharacterClass c);     // 1 frase de "sabor"

    // Evolution system
    EvolutionPath evolutionPath  = EvolutionPath::None;
    int           evolutionTier  = 0;
    float         evolutionPulse = 0.0f;

    float walkAnimTimer = 0.0f;
    bool  isMoving      = false;
    bool  moveRequested = false;  // input recebido neste frame (game feel)
    int   facing        = 1;
    bool  leveledUp     = false;   // SO efeito visual (fica true por levelUpTimer)
    float levelUpTimer  = 0.0f;
    // Niveis ganhos que o Game ainda nao contabilizou. Contador (nao flag): o
    // Game drena 1x por frame. Antes o Game usava `leveledUp` como gatilho de
    // borda — mas ela fica true por 2.5s, entao cada orbe de XP colhido nessa
    // janela dava um level-up/evolucao DE GRACA. E subir 2 niveis de uma vez
    // (addXP grande) so dava 1 ponto.
    int   unclaimedLevels = 0;
    std::string lastPassive;

    // ── Hack Tree (skill tree de perks) ──
    int      skillPoints = 0;   // 1 por level-up; gasta na arvore (tecla X)
    uint32_t perkMask    = 0;   // bits dos perks comprados (SkillTree::bit)

    // Player speech / dialogue
    PlayerSpeech speech;
    KillStreak   killStreak;
    bool         wasLowHP     = false;  // track for speech trigger
    int          totalKills   = 0;

    void say(const std::string& text, float duration = 3.0f, Color col = {0,220,255,255});
    void onKill();
    void onBossFound();
    void onPortalClosed();
    void onEnterZone(int zoneId);

    Player();

    void  move(Vector2 direction, float dt);
    void  update(float dt);
    void  updateSpeech(float dt);
    void  renderSpeech() const;
    void  render() const;
    void  addItem(const Item& item);
    void  useSkill(int index, Vector2 target);
    void  drawInventory() const;
    void  absorbBossEssence(int kind);   // buff PERMANENTE de boss (mexe no stat BASE)

    // ── Save/Load de progresso (classe + stats BASE — efetivos sao recalculados) ──
    CharacterClass getCharClass() const { return charClass; }
    float getBaseMaxHealth()    const { return baseMaxHealth; }
    float getBaseAttackDamage() const { return baseAttackDamage; }
    float getBaseSpeed()        const { return baseSpeed; }
    float getBaseAttackRange()  const { return baseAttackRange; }
    float getBaseDefense()      const { return baseDefense; }
    void  loadSavedProgress(CharacterClass c, float bMax, float bDmg, float bSpd, float bRng, float bDef) {
        applyClass(c);
        baseMaxHealth = bMax; baseAttackDamage = bDmg; baseSpeed = bSpd;
        baseAttackRange = bRng; baseDefense = bDef;
        applyEquipmentStats();
    }
    void  handleInventoryInput();
    // Mouse no inventário (vmouse já virtualizado p/ 1280x720). Retorna true se
    // o clique foi no botão FECHAR (Game deve então fechar o inventário).
    bool  handleInventoryMouse(Vector2 vmouse, bool leftClick, bool rightClick);
    bool  tryUpgradeEquip(int slot);
    void  heal(float amount);
    void  addXP(int amount);   // acumula em unclaimedLevels
    void  takeDamage(float amount);
    void  increaseBaseMaxHP(float amount);
    void  increaseBaseAttackDamage(float amount);
    void  increaseBaseSpeed(float amount);
    void  increaseBaseAttackRange(float amount);
    void  increaseBaseDefense(float amount);
    void  refreshSkillVectors();   // recalcula dano/alcance/cooldown das skills (idempotente)
    void  equipItem(const Equipment& equip);
    void  drawEquipment() const;
    float getEffectiveDamage() const;
    bool  useInventoryItem(int index);
    bool  tryFuseItems();

    int   selectedEquipSlot = 0;
    int   selectedInvItem   = 0;

private:
    void  levelUp();
    void  applyEquipmentStats();
    float baseSpeed        = 250.0f;
    float baseMaxHealth    = 100.0f;
    float baseAttackDamage = 25.0f;
    float baseAttackRange  = 90.0f;
    float baseDefense      = 8.0f;   // defesa base da classe (armadura soma por cima)

    std::vector<float> baseSkillDamage;  // dano base das skills (p/ aplicar skillPower idempotente)
    std::vector<float> baseSkillRange;    // alcance base das skills (perks recomputam dos originais)
    std::vector<float> baseSkillCool;     // cooldown base das skills (idem)
    float cdEvoMult = 1.0f;               // reducoes de cooldown por evolucao (lv 10/25/40/60)
    float cdFusionMult = 1.0f;            // reducoes de cooldown por fusao PlasmaCell (persistente)

    // Render de corpo por classe (formatos distintos)
    Color accentNow() const;  // accent considerando overload/escudo
    void  renderSoldado  (float px, float py, float f, float legL, float legR) const;
    void  renderGuerreira(float px, float py, float f, float legL, float legR) const;
    void  renderRobo     (float px, float py, float f, float legL, float legR) const;
    void  renderMago     (float px, float py, float f, float legL, float legR) const;
    void  renderBruxa    (float px, float py, float f, float legL, float legR) const;
    void  renderHomemFera(float px, float py, float f, float legL, float legR) const;
};
