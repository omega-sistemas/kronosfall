#pragma once

#include "Player.h"
#include "Enemy.h"
#include "Item.h"
#include "Projectile.h"
#include "EnemyProjectile.h"
#include "Tilemap.h"
#include "NPC.h"
#include "Quest.h"
#include "XPOrb.h"
#include "Particle.h"
#include "AudioManager.h"
#include "SaveManager.h"
#include "Zone.h"
#include "Equipment.h"
#include "Background.h"
#include "LightSystem.h"
#include "BotController.h"
#include "EnemyDirector.h"
#include "Companion.h"
#include "BuildingSystem.h"
#include "ShopSystem.h"
#include "AnomalyPortal.h"
#include "CraftingSystem.h"
#include "DarkWorld.h"
#include "InfernoZone.h"
#include "Achievement.h"
#include "TutorialSystem.h"
#include "NetClient.h"
#include "StoreClient.h"
#include "GfxResource.h"
#include "InputMap.h"
#include <vector>
#include <string>
#include <raylib.h>
#include <unordered_map>
#include <set>

// Equipment piece lying on the ground — requires E to pick up
struct GroundEquipment {
    Vector2     position;
    Equipment   equip;
    float       pulseTimer = 0.0f;
    float       lifetime   = 45.0f;
    bool        collected  = false;
};

struct DamageNumber {
    Vector2     pos;           // ancora NO MUNDO (nao se move; a subida vai em `rise`)
    float       value;
    Color       color;
    float       life   = 1.2f;
    // std::string, nao const char*: guardar o retorno de TextFormat() aqui era bug —
    // raylib devolve ponteiro pra um buffer estatico rotativo (4 slots) que e
    // sobrescrito em poucas chamadas, entao o texto virava outro.
    std::string prefix;        // "$" creditos, "+" cura, "Madeira +" recurso, "" dano
    float       rise   = 0.0f; // quanto ja subiu (px no 2D / altura no 3D)
};

// ─── Difficulty System ───────────────────────────────────────────────────────

enum class DifficultyLevel {
    Historia = 0,  // Story Mode — inimigos fracos, mais drops
    Resistente,    // Normal — balanceado (default)
    Guerreiro,     // Hard — mais inimigos fortes
    Infiltrado,    // Nightmare — muito difícil
    Apocalipse     // INSANE — sem misericordia
};

struct DifficultySettings {
    const char* name;
    const char* description;
    Color        labelColor;
    float        enemyHPMult;
    float        enemyDmgMult;
    float        enemySpeedMult;
    float        spawnRateMult;
    float        dropChanceMult;
    float        xpMult;
    float        creditMult;
    float        bossHPMult;
};

// ─────────────────────────────────────────────────────────────────────────────

class Game {
public:
    Game(bool headless = false, int startPhaseOverride = -1);
    ~Game();
    void run();
    unsigned worldSeed    = 0;      // 0 = aleatorio; >0 = mundo reprodutivel
    float autoTestSeconds = 0.0f;   // >0 encerra o autoteste e grava o relatorio
    bool  autoTestPassed  = true;   // resultado do portao de validacao (vira exit code)
    void runAutoTest(bool autoTest);
    void runHeadless();
    bool  headless = false;          // roda SEM janela/GPU (CI)
    int   startPhaseOverride = -1;   // >=0: jump to this phase at start (audit)
    float headlessFps = 0.0f;   // FPS medido do proprio loop headless (GetFPS()=0 sem janela)

private:
    // Update
    void update(float dt);
    void handleInput(float dt);
    void movePlayerWithSlide(Vector2 direction, float dt);
    // Decisoes do bot/autotest (extraido de handleInput): roda no mesmo ponto,
    // sob a guarda interna botController.active. shouldQuit sinaliza via quitRequested.
    void updateBotControl(float dt);
    void spawnEnemy();
    void spawnBoss();
    void checkCollisions();
    void drainLevelUps();   // converte player.unclaimedLevels em pontos/evolucoes pendentes
    // Numeros flutuantes — COMPARTILHADO pelos 2 caminhos de render. No 3D projeta
    // a posicao do mundo pra tela (em coords de mundo ficavam invisiveis no 3D).
    void drawFloatingNumbers(bool project3D) const;
    // ANIMACAO 3D: cada tipo gera VOX_POSES modelos, um por quadro do passo.
    // Antes existia UM modelo congelado por tipo: o personagem so transladava,
    // ou seja, DESLIZAVA pelo cenario em vez de andar.
    static constexpr int VOX_POSES = 4;
    static int  voxKey(int base, int pose) { return base * 8 + pose; }
    // O render 3D depende do SPRITE 2D capturado; o modelo voxel e gerado mas nao usado.
    bool hasVoxelSprite(int key) const {
        auto it = m_voxSprites.find(key);
        return it != m_voxSprites.end() && it->second.valid();
    }
    bool hasVoxelPoses(int base) const { return hasVoxelSprite(voxKey(base, VOX_POSES - 1)); }
    // DEPRECATED: mantido so para compatibilidade de codigo antigo.
    bool hasVoxel(int key) const { return hasVoxelSprite(key); }
    void updateItems(float dt);
    void updateXPOrbs(float dt);
    void updateProjectiles(float dt);
    void updateEnemyProjectiles(float dt);
    void checkPortalTransition();
    void transitionToZone(ZoneID dest);
    void grantQuestRewards(Quest& q);

    // Render
    void render();
    void drawUI() const;
    void drawHudAndOverlays();   // HUD de recursos/ameaça + pause/levelup/loja/etc. (2D E 3D)
    void drawMinimap() const;
    void drawMainMenu() const;
    void drawPauseMenu() const;
    void drawZoneInfo() const;
    void drawQuestLog() const;
    void drawQuestHUD() const;

    // Save
    void autoSave();
    // Bloqueia auto-save em momentos instaveis (morte, dialogo, transicao de
    // fase, telas de escolha, chefe final, pause) — auditoria 09-06 P1.
    bool canAutoSave() const;

    static constexpr int screenWidth  = 1280;
    static constexpr int screenHeight = 720;

    Player                        player;
    std::vector<Enemy>            enemies;
    std::vector<Item>             items;
    std::vector<Projectile>       projectiles;
    std::vector<EnemyProjectile>  enemyProjectiles;
    std::vector<XPOrb>            xpOrbs;
    std::vector<GroundEquipment>  groundEquips;
    Tilemap                       tilemap;
    std::vector<NPC>              npcs;
    std::vector<Quest>            quests;
    ParticleSystem                particles;
    AudioManager                  audio;
    Background                    background;
    InputMap                      input;      // key rebinding system

    Camera2D camera;

    // ── Migração 2.5D isométrico (Incremento 1 & 2) ───────────────────────────
    // Câmera 3D para o mundo (chão/paredes). 2D continua sendo a base estável; o
    // modo 3D é alternável por F10 enquanto a migração avança incremento a incremento.
    Camera3D camera3D{};
    float    cameraHeight = 740.0f;   // altura da câmera acima do plano
    float    cameraDistY  = 580.0f;   // recuo no eixo Z (profundidade isométrica)
    float    cameraZoom   = 1.0f;
    float    camPunch     = 0.0f;   // "camera kick" de zoom: sobe no cast/impacto e decai
    float    worldClock   = 0.32f;    // ciclo dia/noite (0=meia-noite, 0.5=meio-dia)
    int      bossPowersAbsorbed = 0;  // poderes de boss absorvidos (estilo V Rising)
    bool     victoryReported = false; // reset por partida (era static de funcao = bug)
    float    worldSun    = 1.0f;      // 0=noite, 1=dia (deriva do worldClock)     // roda do mouse: <1 aproxima, >1 afasta (olhar de cima)
    GfxRenderTexture tempEntityTarget; // alvo temporário p/ desenhar entidades procedurais
    void     updateCamera3D();
    Vector2  mouseGround3D() const;   // raycast do mouse no plano Y=0 -> mundo 2D
    void     renderWorld3D();         // caminho de render 2.5D completo (mundo 3D + outdoors procedurais)
    // Sprites 2D capturados por tipo — usados como textura nos billboards do mundo 3D.
    std::unordered_map<int, GfxTexture> m_voxSprites;
    int      m_voxGenBudget = 0;   // limite de geracoes de sprite por frame (anti-engasgo)
    template<typename Fn>
    void     ensureVoxel(int key, Vector2 capPos, Fn&& drawFn);
    void     drawVoxel(int base, Vector2 pos, float rotDeg, float walkPhase = 0.0f,
                       bool moving = false);

    struct ChatBubble {
        std::string text;
        float       timer = 0.0f;
    };
    std::unordered_map<uint32_t, ChatBubble> activeChats;
    bool        chatActive = false;
    std::string chatInput;
    std::vector<const Enemy*> netKilledEnemies;

    ZoneID  currentZone       = ZoneID::LARuins;
    float   zoneNameTimer     = 0.0f;  // show zone name on transition

    float   spawnTimer        = 0.0f;
    float   spawnInterval     = 3.0f;
    int     enemiesKilled     = 0;
    int     bossSpawnThreshold= 20;
    bool    bossSpawned       = false;

    // Screen shake
    float   shakeIntensity    = 0.0f;
    float   shakeTimer        = 0.0f;
    void    triggerShake(float intensity, float dur) {
        shakeIntensity = intensity; shakeTimer = dur;
    }

    // Eventos de guerra ambiente (ruinas/cidade fantasma): impacto distante com
    // flash, estrondo abafado e micro-tremor — o mundo "continua em guerra".
    float   owWarTimer  = 0.0f;   // regressiva ate o proximo impacto ambiente
    float   owWarFlash  = 0.0f;   // >0 = impacto ativo (render usa pra desenhar o flash)
    Vector2 owWarPos    = { 0, 0 };
    float   owWarSeed   = 0.0f;   // estabiliza angulo/escala do impacto corrente

    // Indicador direcional de dano — aponta para QUEM feriu o jogador (shooter juice)
    Vector2 hurtDir      = {0, 0};   // vetor unitario (mundo) fonte -> player
    float   hurtDirTimer = 0.0f;     // tempo restante do indicador na borda da tela
    void    noteHurtDir(Vector2 src);

    // Click-to-move target
    Vector2 moveTarget        = {0, 0};
    bool    hasTarget         = false;

    // Combat timers & FX
    std::vector<DamageNumber> damageNumbers;
    float companionHealAccum = 0.0f;
    // Ambiente BASE do bioma atual, interpolado (a troca de regiao nao pisca)
    Color m_ambBaseCol  = {206,212,226,255};
    float m_ambBaseDark = 0.27f;   // cura do Healer acumulada p/ 1 numero a cada ~5 HP
    float meleeCooldown  = 0.0f;
    float hitFlashTimer  = 0.0f;
    float eliteFlashTimer = 0.0f;   // flash BRANCO (dano pesado: elite/boss)
    float slowMoTimer    = 0.0f;
    float footstepTimer  = 0.0f;

    // Story mode
    int   storyChapter   = 1;
    float storyBannerTimer = 0.0f;
    std::string storyBannerText;
    std::string storyBannerSub;
    void  showStoryBanner(const std::string& title, const std::string& sub, float dur = 4.0f);
    void  drawStoryBanner() const;

    // Passive unlock announce
    std::string passiveMsg;
    float       passiveMsgTimer = 0.0f;

    // Combo system
    int   comboCount     = 0;
    float comboTimer     = 0.0f;

    // Runtime telemetry
    int   totalKills     = 0;
    int   totalDamage    = 0;
    float sessionTime    = 0.0f;

    // ── Juice de combate: hit-stop + decalques de chão (sangue/faíscas) ───────
    float hitStopTimer = 0.0f;   // congela o mundo por alguns frames no impacto
    // Alvo do aim assist: posição do inimigo "grudado" no cursor para o retículo
    // do HUD pintar o lock. {-1,-1} = sem alvo.
    Vector2 hudAimLock = {-1.0f, -1.0f};
    struct GroundDecal {
        Vector2 pos; Color color; float life; float maxLife; float size; int type; // 0=sangue 1=queimado
    };
    std::vector<GroundDecal> decals;
    void  addDecal(Vector2 p, Color c, int type, float size);
    void  renderDecals() const;

    // ── Motor de Evolucao Infinita — novidade constante, nunca estagna ────────
    // Nivel de Ameaca: sobe com o tempo/kills, escala inimigos e recompensas.
    int   threatLevel    = 1;
    // IA que EVOLUI durante a partida: observa como o jogador luta e responde
    // na composicao do spawn e na tatica de grupo. Ver EnemyDirector.h.
    EnemyDirector director;
    float threatTimer    = 0.0f;
    int   threatKillMark = 0;     // kills no inicio do nivel atual
    // Mutadores rotativos do mundo — efeitos globais que mudam a cada ciclo.
    enum class WorldMutator {
        None, SwiftEnemies, ArmoredEnemies, BloodMoon, LootRain, Frenzy, Berserk, COUNT
    };
    WorldMutator activeMutator = WorldMutator::None;
    float        mutatorTimer  = 0.0f;
    float        mutatorDuration = 75.0f;
    void         rollNewMutator();
    const char*  mutatorName(WorldMutator m) const;
    const char*  mutatorDesc(WorldMutator m) const;
    float        threatStatMult() const { return 1.0f + (threatLevel - 1) * 0.12f; }
    float        mutatorSpeedMult() const { return activeMutator == WorldMutator::SwiftEnemies ? 1.35f : 1.0f; }
    float        mutatorHPMult()    const { return activeMutator == WorldMutator::ArmoredEnemies ? 1.6f : 1.0f; }
    float        mutatorDropMult()  const { return activeMutator == WorldMutator::LootRain ? 2.5f : 1.0f; }
    float        mutatorSpawnMult() const { return activeMutator == WorldMutator::Frenzy ? 0.55f : 1.0f; }
    float        mutatorDmgMult()   const { return activeMutator == WorldMutator::Berserk ? 1.4f : 1.0f; }
    bool         mutatorBloodMoon() const { return activeMutator == WorldMutator::BloodMoon; }
    void         updateEvolutionEngine(float dt);
    void         playEnemyDeathSound(const class Enemy& e); // som por facção/tipo

    // Render helpers
    void drawObjectivesPanel() const;
    void drawCharacterPanel()  const;
    void drawSkillsPanel()     const;

    // Hack Tree (skill tree de perks)
    void updateSkillTreePanel();
    void drawSkillTreePanel()  const;
    void autoSpendSkillPoints();
    bool buyPerk(int idx);
    static void DrawPanel(int x, int y, int w, int h, Color border, float alpha = 0.82f);
    static void DrawBarH(int x, int y, int w, int h, float pct, Color fill, Color bg);

    std::vector<Companion> companions;

    BuildingSystem buildingSystem;

    // Crafting materials (used by BuildingSystem costs + crafting UI)
    int     materialMetal    = 0;
    int     materialCarapace = 0;

    BotController botController;
    bool    botMeleeRequest   = false;
    bool    botWantsPortal    = false;  // bot pediu o avanco de fase (consumido em updatePhasePortal)
    Vector2 botAimTarget      = {0, 0};

    // ── Timers do bot/autotest (eram static de funcao: nao resetavam entre
    // partidas — restartRun zera todos) ──
    float   botReportSaveTimer = 0.0f;  // auto-save parcial do relatorio (5min)
    float   botAllyTimer       = 2.0f;
    float   botBuildTimer      = 4.0f;
    float   botProduceTimer    = 8.0f;
    float   botUpgradeTimer    = 12.0f;
    float   botStipendTimer    = 0.0f;
    int     botBuildCycle      = 0;
    mutable double lastShot      = 0.0;   // TEMP-SHOT: ultimo screenshot do autotest
    mutable int    shotN         = 0;

    bool    showInventory     = false;
    bool    showEquipment     = false;
    bool    showQuestLog      = false;
    bool    showSkillTree     = false;
    int     perkCursor        = 0;
    bool    paused            = false;
    mutable int pauseHovered  = -1;   // opcao destacada no menu de pause
    float   dyingCryCooldown  = 0.0f; // evita spam do grito de morte
    bool    inMainMenu        = true;

    void    restartRun();             // reinicia a partida do zero
    void    startLoadedGame();        // carrega o save e entra (sem tela de dificuldade)
    bool    quitRequested     = false;  // bot autotest quit signal
    float   saveTimer         = 0.0f;

    // Player speech bubble
    std::string playerSpeechText;
    float       playerSpeechTimer = 0.0f;
    void        triggerPlayerSpeech(const std::string& text, float dur = 3.5f);
    void        drawPlayerSpeech() const;

    // Menu hover tracking (not const — mouse is stateful)
    mutable int menuHoveredBtn = -1;

    // Difficulty system
    DifficultyLevel difficulty          = DifficultyLevel::Resistente;
    bool            selectingDifficulty = false;
    bool            pendingNewGame      = false;
    mutable int     difficultyHovered   = 1;

    // Selecao de personagem (apos a dificuldade, em novo jogo)
    bool            selectingCharacter  = false;
    mutable int     characterHovered    = 0;   // indice em CharacterClass
    void            startNewGame();            // inicia a partida apos as escolhas
    void            drawCharacterSelectScreen() const;
    const DifficultySettings& getDifficulty() const;
    void            drawDifficultyScreen() const;
    static constexpr DifficultySettings DIFFICULTY_TABLE[5] = {
        { "HISTORIA",   "Apenas a historia",          {100,200,255,255}, 0.55f,0.50f,0.80f,0.70f,1.60f,0.80f,1.20f,0.50f },
        { "RESISTENTE", "Experiencia balanceada",      {0,220,100,255},   1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f },
        { "GUERREIRO",  "Hard - mais recompensas",     {255,180,0,255},   1.45f,1.35f,1.15f,1.30f,1.35f,1.25f,1.30f,1.45f },
        { "INFILTRADO", "So os melhores sobrevivem",   {255,80,0,255},    2.10f,1.85f,1.35f,1.70f,1.70f,1.60f,1.75f,2.00f },
        { "APOCALIPSE", "Sem misericordia",            {200,0,255,255},   3.20f,2.60f,1.60f,2.20f,2.20f,2.00f,2.40f,3.00f }
    };

    // Shop system
    ShopSystem shopSystem;

    // Crafting system
    CraftingSystem craftingSystem;

    // Dialog state
    int     nearNpcIndex      = -1;
    int     dialogLine        = 0;
    bool    dialogOpen        = false;

    // Fullscreen render target (virtual 1280x720 always)
    GfxRenderTexture gameTarget;

    // ── POS-PROCESSAMENTO (bloom + tonemap) ──
    // O frame inteiro (mundo + HUD) sai do gameTarget e passa por: brilho ->
    // blur H -> blur V -> composicao com tonemap filmico. Se algum shader falhar
    // em compilar, m_postFX fica false e a apresentacao volta ao caminho antigo
    // (DrawTexturePro puro) — nunca tela preta.
    GfxShader          m_shBright, m_shBlur, m_shGrade;
    GfxRenderTexture   m_bloomA, m_bloomB;
    bool            m_postFX = false;
    int m_locThreshold = -1, m_locKnee = -1, m_locBlurDir = -1;
    int m_locBloomTex = -1, m_locBloomStr = -1, m_locExposure = -1,
        m_locSaturation = -1, m_locContrast = -1;
    // Shader de ILUMINACAO do mundo 3D (direcional + ambiente + rim + nevoa).
    // Sem ele todo poligono saia com a cor escrita, sem volume: papelao colorido.
    GfxShader m_shWorld;
    bool      m_worldLit = false;
    GfxTexture m_whiteTex;   // textura 1x1 branca: voxels usam cor por vertice
    int    m_locLightDir = -1, m_locLightCol = -1, m_locAmbCol = -1, m_locCamPos = -1,
           m_locFogCol = -1, m_locFogStart = -1, m_locFogEnd = -1, m_locRim = -1,
           m_locSpecK = -1, m_locWorldPer = -1;
    void   initWorldShader();
    void   applyWorldShader(Model& m) const;   // liga o shader no material do modelo
    void   applyWorldShader(GfxModel& m) const;
    void   updateWorldShaderUniforms();

    void    drawGenericStructure(Vector2 pos, float sc) const;
    void    drawArkStructure(Vector2 pos) const;   // Arca sci-fi (bunker) p/ zonas urbanas
    void    initPostFX();
    void    unloadPostFX();

    void    presentFrame() const;
    Vector2 virtualizeMousePos(Vector2 m) const;

    // Omega boss tracking
    int     omegaKillThreshold = 50;
    void    spawnOmegaBoss();

    void    buildNPCs();
    void    buildQuests();
    void    setupZoneNPCs(ZoneID zone);
    void    setupBaseNPCs();          // NPCs de servico dentro da zona segura

    // Open world
    struct WorldRegion {
        Rectangle   bounds;
        ZoneID      zoneType;
        std::string name;
        bool        discovered = false;
        Color       mapColor;
    };
    std::vector<WorldRegion> worldRegions;

    // Cenario persistente do mundo aberto — TODAS as regioes populadas de uma vez,
    // espalhadas por toda a area e sempre renderizadas (casas, lapides, lava, etc.)
    DarkWorld owDecor;
    bool      owDecorBuilt = false;

    // E.1 — geração amortizada do cenário fixo: objetos são enfileirados durante
    // buildOpenWorldScenery e transferidos para owDecor.scenery em lotes durante
    // o update, em vez de realocar tudo num único frame.
    std::vector<SceneryObject> m_sceneryBuildQueue;
    bool      m_sceneryPostProcessNeeded = false;
    static constexpr int SCENERY_BUILD_BUDGET = 300; // objetos por frame
    void      streamSceneryBuild();

    // ── FASES (mundo aberto) ─────────────────────────────────────────────────
    // Cada fase e um MUNDO inteiro de um bioma so. Some o numero de abates da
    // fase; ao bater a meta o PORTAL abre e leva ao proximo mundo com tela de
    // transicao. E o que da a sensacao de "passei de fase" que andar nao dava.
    int       owPhase        = 0;
    int       owPhaseKills   = 0;
    int       owPhaseGoal    = 20;
    int       owKillsAtStart = 0;
    bool      owPortalOpen   = false;
    Vector2   owPortalPos    = { 0.0f, 0.0f };
    float     owFadeTimer    = 0.0f;   // >0 = tela de transicao de fase
    std::string owFadeText;
    // A fase e um LUGAR com limite, nao um tapete infinito: barreira de energia
    // no raio abaixo. Sem borda, "passar de fase" nao existe - o jogador so anda.
    float     owPhaseRadius  = 3000.0f;
    float     borderWarnTimer = 0.0f;   // anti-spam do aviso de barreira
    bool      owBossPhase    = false;   // a cada 3 fases o portal so abre com o boss morto
    bool      owBossDown     = false;
    void      updatePhasePortal(float dt);
    void      advanceOpenWorldPhase();
    void      drawPhaseFade() const;
    // Campanha vinda de content/phases.txt (dado, nao codigo). Se o arquivo nao
    // existir cai numa tabela embutida, entao o jogo NUNCA deixa de abrir por
    // causa de conteudo faltando.
    struct PhaseDef {
        ZoneID      zone   = ZoneID::LARuins;
        int         goal   = 20;
        bool        boss   = false;
        float       radius = 3000.0f;
        std::string title;
    };
    std::vector<PhaseDef> phaseDefs;
    void            loadPhaseDefs();
    const PhaseDef& phaseDef(int phase) const;
    static ZoneID phaseZone(int phase);
    // Mundo infinito: cenário gerado por CHUNKS ao redor do player (auto-construção)
    std::set<long long> m_sceneryChunks;
    int  m_lastChunkX = -999999, m_lastChunkY = -999999;
    void updateSceneryChunks(Vector2 playerPos);
    std::vector<Vector3> m_chunkSolids;  // colisao de estruturas dos chunks (x,y=pos, z=raio)
    bool isBlocked(Vector2 pos) const;   // parede do grid OU estrutura de chunk no infinito
    void clearBlockingAt(Vector2 pos, float radius);  // fallback do portal: remove colisao de cenario num raio
    void      buildOpenWorldScenery();
    void      placeBaseShops();   // barracas/lojas dos NPCs da zona segura (tipos 23-27)

    // Zona Segura / Base — refugio sem inimigos para preparar e construir.
    // Voce nasce e renasce aqui; inimigos nao spawnam nem perseguem dentro dela.
    Vector2   safeZoneCenter = {1280.0f, 1280.0f}; // centro da regiao inicial
    float     safeZoneRadius = 1050.0f;
    bool      wasInSafeZone   = true;   // p/ detectar quando o jogador sai da base
    bool      inSafeZone(Vector2 pos) const {
        float dx = pos.x - safeZoneCenter.x, dy = pos.y - safeZoneCenter.y;
        return (dx*dx + dy*dy) <= (safeZoneRadius * safeZoneRadius);
    }

    // Retorna true se a posicao estiver fora do disco da fase no mundo aberto.
    bool      isOutsideOpenWorldBounds(Vector2 pos) const {
        if (!openWorldMode) return false;
        float dx = pos.x - safeZoneCenter.x, dy = pos.y - safeZoneCenter.y;
        return (dx*dx + dy*dy) > (owPhaseRadius * owPhaseRadius);
    }

    // Projeta a posicao de volta para dentro do limite da fase, se necessario.
    void      clampInsideOpenWorldBounds(Vector2& pos, float margin) const {
        if (!openWorldMode) return;
        float dx = pos.x - safeZoneCenter.x, dy = pos.y - safeZoneCenter.y;
        float l2 = dx*dx + dy*dy;
        float limit = owPhaseRadius - margin;
        if (limit < 0.0f) limit = 0.0f;
        if (l2 > limit * limit) {
            float l = sqrtf(l2); if (l < 1.0f) { dx = 1.0f; dy = 0.0f; l = 1.0f; }
            pos.x = safeZoneCenter.x + dx / l * limit;
            pos.y = safeZoneCenter.y + dy / l * limit;
        }
    }

    // ── Coleta de recursos naturais (madeira/pedra/ferro/prata/ouro) ──────────
    enum class ResourceType { Wood = 0, Stone, Iron, Silver, Gold, COUNT };
    struct ResourceNode {
        Vector2      position;
        ResourceType type;
        int          amount;       // unidades restantes no nó
        int          maxAmount;
        float        harvestProg;  // 0..1 progresso da coleta atual
        float        respawnTimer; // >0 = depletado, contando para voltar
        bool         depleted;
        float        shake;        // tremor visual ao coletar
    };
    std::vector<ResourceNode> resourceNodes;
    int   playerResources[(int)ResourceType::COUNT] = {0,0,0,0,0};

    // ── Animais / vida selvagem (NPCs de animais) ─────────────────────────────
    enum class AnimalType { Deer = 0, Rabbit, Boar, Wolf, Bird, COUNT };
    struct Animal {
        Vector2    position;
        Vector2    velocity;
        AnimalType type;
        float      health, maxHealth;
        float      wanderTimer;
        Vector2    wanderDir;
        bool       fleeing;
        bool       hostile;     // lobo persegue/ataca
        float      animTimer;
        float      attackCD;
        bool       dead;
    };
    std::vector<Animal> animals;
    void  setupAnimals();
    void  updateAnimals(float dt);
    void  renderAnimals() const;

    // ── Civis da cidade (vida ambiente: cada um TEM UMA TAREFA, não vaga à toa) ──
    enum class FolkJob { Guard, Worker, Chatter, Vendor };
    struct CityFolk {
        Vector2 position, target, home, anchor;  // anchor = posto de trabalho / ponto B da ronda / roda de conversa
        float   speed       = 55.0f;
        float   timer       = 0.0f;   // genérico (ronda/troca de alvo)
        float   work        = 0.0f;   // animação/progresso da tarefa
        float   pauseTimer  = 0.0f;   // parado executando a tarefa
        int     role        = 0;      // NPCRole → modelo voxel + cor
        int     facing      = 1;
        FolkJob job         = FolkJob::Worker;
        float   walkPhase   = 0.0f;   // fase do passo (avanca com o deslocamento)
        bool    atStation   = false;  // chegou no posto e está trabalhando/conversando
    };
    std::vector<CityFolk> cityFolk;
    void  spawnCityFolk();
    void  updateCityFolk(float dt);

    // ── Multiplayer em tempo real (NetClient — WebSocket) ─────────────────────
    NetClient net;
    uint32_t  netId       = 0;
    bool      netActive   = false;
    bool      netPending_ = false;   // aguardando o login (JWT) p/ iniciar o WS
    void      startNetwork();
    void      pollStartNetwork();    // inicia o WS assim que o JWT chegar
    void      renderRemotePlayers() const;

    // ── Grupo / Aliança (party multiplayer) ───────────────────────────────────
    bool        partyPanel = false;       // painel de grupo aberto (tecla O)
    std::string partyInput;               // codigo sendo digitado
    void        updateParty();
    void        drawPartyPanel() const;

    // ── Loja premium (Gems / Stripe via backend Node) ─────────────────────────
    StoreClient store;
    bool        storeStarted   = false;  // login + fetch disparados
    bool        premiumView    = false;  // aba premium aberta dentro da loja
    int         premiumSel     = 0;      // item selecionado na aba premium
    float       storeRefreshT  = 0.0f;   // timer p/ atualizar saldo de gems
    void        startStore();            // login + catalogo (uma vez)
    void        updatePremiumStore(float dt);
    void        drawPremiumStore() const;

    // ── Pathfinding do bot: unificado no BotController (BFS em computePathDir) ──
    int   nearResourceIdx = -1;    // nó mais próximo coletável
    float mineSwingCD     = 0.0f;  // cadência entre golpes da picareta
    float mineSwingAnim   = 0.0f;  // 0..1 animação do golpe atual (1=acabou de bater)
    int   mineFxIdx       = -1;    // nó sendo golpeado (p/ desenhar picareta/impacto)
    void  setupResourceNodes();
    void  updateResourceGathering(float dt);
    void  renderResourceNodes() const;   // world-space (dentro de BeginMode2D)
    void  drawResourceHUD() const;       // screen-space
    static const char* resourceName(ResourceType t);
    static Color       resourceColor(ResourceType t);

    // Controle RTS — selecao por arrasto do mouse
    bool      rtsDragging   = false;   // arrasto ativo (passou do limiar)
    Vector2   rtsDragStart  = {0, 0};  // ponto inicial do arrasto (mundo)
    Vector2   rtsDragCur    = {0, 0};  // ponto atual (mundo)
    bool      rtsHasUnits   = false;   // ha unidades selecionadas
    ZoneID                   currentRegion = ZoneID::LARuins;
    bool                     openWorldMode = false;
    void   setupWorldRegions();
    ZoneID getRegionAt(Vector2 pos) const;

    // Companion system
    void    spawnCompanion(CompanionType t);
    void    updateCompanions(float dt);
    void    drawCompanions() const;

    // Level Up / Evolution screen
    bool          showLevelUpScreen   = false;
    bool          showEvolutionScreen = false;
    int           levelUpChoice       = 1;
    int           evolutionChoice     = 1;
    LevelUpChoice levelUpOptions[3];
    float         levelUpAnimTimer    = 0.0f;

    // Pontos acumulados — NAO travam o jogo; jogador escolhe quando quiser
    int           pendingLevelUps     = 0;   // pontos de level up nao gastos (tecla L)
    int           pendingEvolutions   = 0;   // escolhas de evolucao nao gastas (tecla K)
    float         pendingNotifyPulse  = 0.0f; // pulso visual do aviso no HUD

    // Condicao de vitoria — boss final NUCLEO KRONOS no KronosNexus
    bool          gameWon             = false;
    float         victoryTimer        = 0.0f;
    // New Game+ — reinicia o mundo mantendo o personagem; 0 = jogo normal.
    // No NG+ o boss final vira o Leviathan (conteudo morto ativado).
    int           newGamePlus         = 0;
    bool          finalBossSpawned    = false; // ja foi invocado nesta partida
    bool          finalBossAlive      = false; // esta vivo agora
    void          spawnFinalBoss();
    void          startNewGamePlus();
    void          drawVictoryScreen() const;

    void generateLevelUpChoices();
    void applyLevelUpChoice(int idx);
    void drawLevelUpScreen() const;
    void drawEvolutionScreen() const;
    void applyEvolutionPath(int pathIdx);

    // Contextual dialogue state
    bool  firstCombatTriggered  = false;
    bool  firstKillTriggered    = false;
    float surroundedCooldown    = 0.0f;

    // Anomaly portal system
    AnomalySystem anomalySystem;
    float         anomalyWaveTimer    = 0.0f;
    static constexpr float anomalyWaveCooldown = 45.0f;

    // 2D lighting system (active in dark zones: Cemetery..AbandonedManor)
    LightSystem   lightSystem;
    bool          darkZoneActive = false;

    // Dark world scenery system
    DarkWorldSystem darkWorld;

    // Inferno zone — volcanic lava + geysers
    InfernoZoneSystem infernoZone;

    // Achievement system
    AchievementSystem achievements;

    // Tutorial system
    TutorialSystem tutorial;
    bool tutorialRewardGiven = false;

    // Telemetry for achievements
    int   totalKillsEver   = 0;
    int   totalBossesKilled= 0;
    int   totalPortalsClosed=0;
    int   totalCreditsEarned=0;
    int   totalDeaths      = 0;
    float totalPlaytime    = 0.0f;
    int   zonesVisitedSet  = 0;  // bitmask of visited zone ids

    // 3D Models and textures for realistic 3D graphics
    GfxModel m_houseModel;
    GfxModel m_turretModel;
    GfxModel m_barracksModel;
    GfxModel m_castleModel; // used for Town Hall / Arca
    GfxModel m_marketModel; // used for Tank Factory
    GfxModel m_wellModel;   // used for MedBay
    GfxModel m_carModel;    // old_car_new.glb — carros do cenário 3D
    float m_houseScale=1,m_turretScale=1,m_barracksScale=1,m_castleScale=1,m_marketScale=1,m_wellScale=1,m_carScale=1;
    GfxTexture m_houseTex;
    GfxTexture m_turretTex;
    GfxTexture m_barracksTex;
    GfxTexture m_castleTex;
    GfxTexture m_marketTex;
    GfxTexture m_wellTex;
    bool m_modelsLoaded = false;
};
