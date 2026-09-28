#include "Game.h"
#include "SpriteGen.h"
#include "SpriteExtrude.h"
#include "SkillTree.h"
// true SOMENTE durante a captura de sprite para voxelização (Game::ensureVoxel →
// SpriteExtrude::CaptureToImage): as entidades suprimem sombras/textos 2D para não
// virarem "pedestal" na malha voxel. Não seleciona pipeline de render.
bool g_voxelCapture = false;

// ── ARQUITETURA POR BIOMA ────────────────────────────────────────────────────
// Todas as fases usavam os MESMOS 4 modelos (casa/celeiro/castelo/silo): trocava
// o chao e o ceu, mas a cidade era identica em Los Angeles, no cemiterio e no
// inferno. Aqui cada bioma tem seu proprio conjunto de tipos de estrutura.
//   0 casa   1 celeiro  7 castelo/predio  8 silo
//  14 cripta 15 bunker  16 espira infernal 17 monolito 18 cabana 19 torre

// Tinta das construcoes por bioma: o mesmo modelo lido como pedra clara em LA e
// como pedra queimada no inferno ja muda a leitura da cidade inteira.
static Color structureTintFor(ZoneID z) {
    switch (z) {
        case ZoneID::Cemetery:       return { 150, 158, 172, 255 };
        case ZoneID::DarkForest:     return { 148, 156, 132, 255 };
        case ZoneID::CursedFarm:     return { 198, 176, 132, 255 };
        case ZoneID::Bunker:         return { 138, 150, 140, 255 };
        case ZoneID::Catacombs:      return { 152, 140, 126, 255 };
        case ZoneID::AbandonedManor: return { 158, 140, 162, 255 };
        case ZoneID::KronosForge:    return { 186, 142, 110, 255 };
        case ZoneID::InfernoZone:    return { 150,  96,  80, 255 };
        case ZoneID::KronosNexus:    return { 130, 168, 196, 255 };
        case ZoneID::GhostCity:      return { 160, 168, 180, 255 };
        case ZoneID::LARuins:
        default:                     return { 255, 255, 255, 255 };
    }
}

// Zonas onde os modelos MEDIEVAIS (castle.obj / house.obj) sao coerentes: areas
// rurais/goticas. Nas zonas urbanas e sci-fi (LA, cidade fantasma, bunker, forja,
// nexus...) o castelo de torres e a casa de telha quebram a direcao de arte —
// there the BuildingSystem draws modern structures with primitives (audit P1).
static bool isMedievalZone(ZoneID z) {
    return z == ZoneID::CursedFarm || z == ZoneID::DarkForest ||
           z == ZoneID::Cemetery  || z == ZoneID::AbandonedManor;
}

// ── ESCALA DO MUNDO ──────────────────────────────────────────────────────────
// Tudo ancorado no heroi: ~28 unidades de altura = 1,75 m, entao 1 metro ~ 16u.
// Os valores antigos (casa 110u = 7 m na MAIOR dimensao) deixavam predio menor
// que gente: a cidade lia como maquete e o personagem como um poste ao lado dela.
static constexpr float FIT_HOUSE    = 175.0f;   // casa de 2 andares ~11 m
static constexpr float FIT_BARRACKS = 190.0f;   // celeiro/galpao ~12 m
static constexpr float FIT_CASTLE   = 340.0f;   // predio/castelo ~21 m
static constexpr float FIT_TURRET   =  95.0f;
static constexpr float FIT_MARKET   = 200.0f;
static constexpr float FIT_WELL     = 130.0f;   // silo alto
static constexpr float FIT_CAR      =  68.0f;   // carro ~4,2 m de comprimento
#include <raylib.h>
#include <raymath.h>
#include "rlgl.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <string>
#include <filesystem>

static void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    rlSetTexture(texture.id);

    rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // Front Face
        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);

        // Back Face
        rlNormal3f(0.0f, 0.0f, -1.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);

        // Top Face
        rlNormal3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);

        // Bottom Face
        rlNormal3f(0.0f, -1.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);

        // Right face
        rlNormal3f(1.0f, 0.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);

        // Left Face
        rlNormal3f(-1.0f, 0.0f, 0.0f);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);
    rlEnd();

    rlSetTexture(rlGetTextureIdDefault());   // P0: religa branca p/ não vazar textura nas primitivas
}

// ─── Constructor / Destructor ────────────────────────────────────────────────

Game::Game(bool headless_, int startPhaseOverride_) {
    headless = headless_;
    startPhaseOverride = startPhaseOverride_;
    // Headless (CI sem display/GPU): pula TODO o bloco grafico do construtor
    // (janela, render textures, sprites, modelos e shaders) e so monta os dados
    // e a simulacao. O bot corre igual — cenario, inimigos, fases, colisao.
    // Bloco grafico: janela redimensionavel — o conteudo (1280x720) e escalado com
    // letterbox em presentFrame(), entao nunca corta. F11 alterna tela cheia.
    if (!headless) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "KRONOSFALL - ARPG | The Darknet is falling. Make Kronos fall.");
    SetExitKey(KEY_NULL);   // ESC NAO fecha o jogo — abre o menu de pause
    SetTargetFPS(60);

    // Inicia em JANELA que cabe na area util do monitor (evita ficar maior que a
    // tela e cortar). Reduz mantendo proporcao se o monitor for pequeno.
    {
        // Abre no monitor MAIS A DIREITA (onde o Antigravity fica). Acha o monitor
        // com maior X virtual.
        int mc = GetMonitorCount();
        int mon = 0; float bestX = -1e9f;
        for (int i = 0; i < mc; ++i) {
            Vector2 mp = GetMonitorPosition(i);
            if (mp.x > bestX) { bestX = mp.x; mon = i; }
        }
        Vector2 mpos = GetMonitorPosition(mon);
        int mw  = GetMonitorWidth(mon);
        int mh  = GetMonitorHeight(mon);
        if (mw > 0 && mh > 0) {
            float maxW = mw * 0.90f, maxH = mh * 0.90f;
            float s = std::min(maxW / screenWidth, maxH / screenHeight);
            if (s > 1.0f) s = 1.0f;
            int winW = (int)(screenWidth  * s);
            int winH = (int)(screenHeight * s);
            SetWindowSize(winW, winH);
            SetWindowPosition((int)mpos.x + (mw - winW) / 2,
                              (int)mpos.y + std::max(0, (mh - winH) / 2 - 16));
        }
    }
    gameTarget = GfxRenderTexture(LoadRenderTexture(screenWidth, screenHeight));
    // POINT (nearest) deixa o texto NITIDO ao escalar para tela cheia (BILINEAR borrava).
    SetTextureFilter(gameTarget.get().texture, TEXTURE_FILTER_POINT);
    tempEntityTarget = GfxRenderTexture(LoadRenderTexture(128, 128));
    SetTextureFilter(tempEntityTarget.get().texture, TEXTURE_FILTER_POINT);
    initPostFX();       // bloom + tonemap
    initWorldShader();  // luz direcional + rim + nevoa nos modelos 3D
    {   // textura branca 1x1 para materiais cor-por-vertice (voxels de personagens)
        Image white = GenImageColor(1, 1, WHITE);
        m_whiteTex = GfxTexture(LoadTextureFromImage(white));
        UnloadImage(white);
    }
    lightSystem.init(screenWidth, screenHeight);

    SpriteBank::get().init();   // gera os sprites pixel-art (precisa de contexto GL)

    // Carrega modelos 3D para graficos reais
    if (FileExists("resources/models/house.obj")) {
        m_houseModel = GfxModel(LoadModel("resources/models/house.obj"));
        m_houseTex = GfxTexture(LoadTexture("resources/models/house_diffuse.png"));
        m_houseModel.get().materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_houseTex.get();
    }
    if (FileExists("resources/models/turret.obj")) {
        m_turretModel = GfxModel(LoadModel("resources/models/turret.obj"));
        m_turretTex = GfxTexture(LoadTexture("resources/models/turret_diffuse.png"));
        m_turretModel.get().materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_turretTex.get();
    }
    if (FileExists("resources/models/barracks.obj")) {
        m_barracksModel = GfxModel(LoadModel("resources/models/barracks.obj"));
        m_barracksTex = GfxTexture(LoadTexture("resources/models/barracks_diffuse.png"));
        m_barracksModel.get().materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_barracksTex.get();
    }
    if (FileExists("resources/models/castle.obj")) {
        m_castleModel = GfxModel(LoadModel("resources/models/castle.obj"));
        m_castleTex = GfxTexture(LoadTexture("resources/models/castle_diffuse.png"));
        m_castleModel.get().materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_castleTex.get();
    }
    if (FileExists("resources/models/market.obj")) {
        m_marketModel = GfxModel(LoadModel("resources/models/market.obj"));
        m_marketTex = GfxTexture(LoadTexture("resources/models/market_diffuse.png"));
        m_marketModel.get().materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_marketTex.get();
    }
    if (FileExists("resources/models/well.obj")) {
        m_wellModel = GfxModel(LoadModel("resources/models/well.obj"));
        m_wellTex = GfxTexture(LoadTexture("resources/models/well_diffuse.png"));
        m_wellModel.get().materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_wellTex.get();
    }
    if (FileExists("resources/models/old_car_new.glb")) {
        m_carModel = GfxModel(LoadModel("resources/models/old_car_new.glb"));
    }
    {
        auto _fit = [](const GfxModel& m, float target)->float {
            if (!m.valid()) return 1.0f;
            BoundingBox bb = GetModelBoundingBox(m.get());
            float d = fmaxf(bb.max.y - bb.min.y, fmaxf(bb.max.x - bb.min.x, bb.max.z - bb.min.z));
            return (d > 0.001f) ? target / d : 1.0f;
        };
        m_houseScale    = _fit(m_houseModel,    FIT_HOUSE);
        m_barracksScale = _fit(m_barracksModel, FIT_BARRACKS);
        m_castleScale   = _fit(m_castleModel,   FIT_CASTLE);
        m_turretScale   = _fit(m_turretModel,   FIT_TURRET);
        m_marketScale   = _fit(m_marketModel,   FIT_MARKET);
        m_wellScale     = _fit(m_wellModel,     FIT_WELL);
        m_carScale      = _fit(m_carModel,      FIT_CAR);
    }
    // Liga a iluminacao nos modelos de cenario (os voxel recebem ao serem gerados)
    applyWorldShader(m_houseModel);    applyWorldShader(m_barracksModel);
    applyWorldShader(m_castleModel);   applyWorldShader(m_turretModel);
    applyWorldShader(m_marketModel);   applyWorldShader(m_wellModel);
    applyWorldShader(m_carModel);
    m_modelsLoaded = true;
    }   // fim do bloco grafico — headless nao cria janela/GL/models/texturas

    audio.init();
    loadPhaseDefs();   // campanha vem de content/phases.txt (editavel sem recompilar)
    buildQuests();
    buildNPCs();
    craftingSystem.buildRecipes();
    achievements.init();

    // Open world — mundo aberto CENTRADO NA BASE. A fase precisa estar definida
    // ANTES das regioes: setupWorldRegions deriva a grid do owPhaseRadius e do
    // bioma (currentZone) da fase. Com o raio no default (3000) as regioes
    // nasciam menores que a barreira e o anel externo era populado por chunks
    // com outra densidade — a borda da primeira fase lia diferente do resto.
    int startPhase = (startPhaseOverride >= 0) ? startPhaseOverride : 0;
    const PhaseDef& p0 = phaseDef(startPhase);
    owPhase = startPhase; owPhaseKills = 0; owKillsAtStart = 0;
    owPhaseGoal = p0.goal; owPhaseRadius = p0.radius;
    owBossPhase = p0.boss; owBossDown = false; owPortalOpen = false;
    openWorldMode = true;
    tilemap.generateOpenWorld();
    setupWorldRegions();

    // Player starts in center of first region (LARuins) = ZONA SEGURA
    // (definido ANTES de buildOpenWorldScenery: o cenario usa safeZoneCenter e
    //  owPhaseRadius para limitar o mundo da fase — com o centro velho a
    //  construcao saia errada e era refeita depois, dobrando o trabalho)
    float cx = (float)(Tilemap::OW_ZONE_W * Tilemap::tileSize) / 2.0f;
    float cy = (float)(Tilemap::OW_ZONE_H * Tilemap::tileSize) / 2.0f;
    player.position  = {cx, cy};
    safeZoneCenter   = {cx, cy};   // refugio fica no centro da regiao inicial
    {   // fase 1 tambem sai da tabela (antes os valores viviam so no codigo)
        currentZone   = p0.zone;
        currentRegion = p0.zone;
    }
    buildOpenWorldScenery();

    camera.offset   = {screenWidth / 2.0f, screenHeight / 2.0f};
    camera.target   = player.position;
    camera.rotation = 0.0f;
    camera.zoom     = 1.0f;

    // Câmera 3D (2.5D) — valores iniciais válidos antes do primeiro update.
    updateCamera3D();

    spawnInterval = getZoneInfo(currentZone).spawnInterval;
    if (!headless)   // background e textura GPU (so render), nao existe simulacao nela
        background.generate(currentZone, tilemap.width, tilemap.height, Tilemap::tileSize);
}

Game::~Game() {
    if (headless) {
        audio.shutdown();   // sem contexto GL: nao ha GPU/texturas/modelos para liberar
        return;
    }
    // Os wrappers RAII (GfxRenderTexture, GfxModel, GfxTexture, GfxShader) liberam
    // seus recursos automaticamente no destrutor, mesmo que excecoes tenham ocorrido.
    lightSystem.shutdown();
    SpriteBank::get().shutdown();
    audio.shutdown();
    unloadPostFX();

    CloseWindow();
}

// ─── Difficulty System ───────────────────────────────────────────────────────

const DifficultySettings& Game::getDifficulty() const {
    return DIFFICULTY_TABLE[(int)difficulty];
}

void Game::drawDifficultyScreen() const {
    BeginTextureMode(gameTarget.get());  // overlay on top of menu (no ClearBackground)

    float t = (float)GetTime();

    // Dark overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.88f));

    // Title
    const char* title = "SELECIONE A DIFICULDADE";
    int titleW = MeasureText(title, 30);
    DrawText(title, screenWidth/2 - titleW/2 + 3, 101, 30, ColorAlpha({0,140,255,255}, 0.25f));
    DrawText(title, screenWidth/2 - titleW/2, 98, 30, Color{0,235,255,255});
    {
        int ty = 134;
        DrawLine(screenWidth/2 - 340, ty, screenWidth/2 + 340, ty, ColorAlpha({0,235,255,255}, 0.45f));
        DrawLine(screenWidth/2 - 346, ty - 5, screenWidth/2 - 346, ty + 5, ColorAlpha({0,235,255,255}, 0.6f));
        DrawLine(screenWidth/2 - 340, ty, screenWidth/2 - 346, ty + 5, ColorAlpha({0,235,255,255}, 0.8f));
        DrawLine(screenWidth/2 + 340, ty, screenWidth/2 + 346, ty + 5, ColorAlpha({0,235,255,255}, 0.8f));
        DrawRectangle(screenWidth/2 - 3, ty - 4, 6, 8, ColorAlpha({255,180,40,255}, 0.9f));
    }

    // 5 cards layout
    const int cardW   = 196;
    const int cardH   = 268;
    const int cardGap = 10;
    const int totalW  = 5 * cardW + 4 * cardGap;
    const int startX  = (screenWidth - totalW) / 2;
    const int startY  = 150;

    for (int i = 0; i < 5; ++i) {
        const DifficultySettings& ds = DIFFICULTY_TABLE[i];
        bool isHov = (difficultyHovered == i);
        bool isSel = ((int)difficulty == i);
        float alpha = isHov ? 1.0f : 0.75f;
        float pulse = 0.5f + 0.5f * std::sin(t * 3.0f);

        int cx = startX + i * (cardW + cardGap);
        int cy = startY;

        // Glow halo behind hovered card
        if (isHov) {
            DrawRectangle(cx - 5, cy - 5, cardW + 10, cardH + 10,
                         ColorAlpha(ds.labelColor, 0.12f * pulse));
        }

        // Card background
        {
            unsigned char cr = isHov ? 18 : 8;
            unsigned char cg = isHov ? 28 : 12;
            unsigned char cb = isHov ? 50 : 20;
            DrawRectangle(cx, cy, cardW, cardH,
                         ColorAlpha(Color{cr, cg, cb, 255}, 0.96f));
        }

        // Card border
        float bw = isHov ? 2.5f : 1.0f;
        DrawRectangleLinesEx({(float)cx,(float)cy,(float)cardW,(float)cardH}, bw,
                             ColorAlpha(ds.labelColor, isHov ? 1.0f : 0.40f));

        // Cyberpunk corner cuts
        int cc = 9;
        DrawLine(cx, cy+cc, cx+cc, cy, ColorAlpha(ds.labelColor, isHov ? 0.9f : 0.35f));
        DrawLine(cx+cardW-cc, cy, cx+cardW, cy+cc, ColorAlpha(ds.labelColor, isHov ? 0.9f : 0.35f));
        DrawLine(cx, cy+cardH-cc, cx+cc, cy+cardH, ColorAlpha(ds.labelColor, isHov ? 0.7f : 0.25f));
        DrawLine(cx+cardW-cc, cy+cardH, cx+cardW, cy+cardH-cc, ColorAlpha(ds.labelColor, isHov ? 0.7f : 0.25f));

        // Pulsing top glow on hovered
        if (isHov) {
            DrawRectangle(cx, cy, cardW, 3, ColorAlpha(ds.labelColor, 0.75f * pulse));
        }

        // Number badge
        DrawRectangle(cx+cardW-26, cy+4, 22, 18, ColorAlpha(ds.labelColor, 0.20f));
        DrawText(TextFormat("%d", i+1), cx+cardW-21, cy+6, 13, ColorAlpha(ds.labelColor, 0.85f));

        // Difficulty name
        int nameW = MeasureText(ds.name, 15);
        DrawText(ds.name, cx + cardW/2 - nameW/2, cy + 10, 15,
                 ColorAlpha(ds.labelColor, alpha));

        // Separator
        DrawLine(cx+8, cy+32, cx+cardW-8, cy+32,
                 ColorAlpha(ds.labelColor, isHov ? 0.45f : 0.22f));

        // Description (clipped to card)
        BeginScissorMode(cx+4, cy+36, cardW-8, 22);
        DrawText(ds.description, cx+8, cy+36, 10, ColorAlpha(WHITE, alpha * 0.75f));
        EndScissorMode();

        // ── Bars ─────────────────────────────────────────────────────────────
        struct BarDef { const char* label; float value; float maxVal; Color col; };
        BarDef bars[3] = {
            { "INIMIGOS",   ds.enemyHPMult,   3.5f, {220,60, 60,255} },
            { "VELOCIDADE", ds.spawnRateMult,  2.5f, {255,160,0, 255} },
            { "RECOMPENSA", ds.dropChanceMult, 2.5f, {0, 200,100,255} },
        };

        int barX = cx + 8;
        int barInnerW = cardW - 16;
        int barH2 = 9;
        int barSpacing = 38;
        int barsStartY = cy + 64;

        for (int b = 0; b < 3; ++b) {
            int by = barsStartY + b * barSpacing;
            float pct = std::min(bars[b].value / bars[b].maxVal, 1.0f);
            float barAlpha = isHov ? 1.0f : 0.65f;

            // Label
            DrawText(bars[b].label, barX, by, 9, ColorAlpha(WHITE, alpha * 0.65f));

            // Track
            DrawRectangle(barX, by + 13, barInnerW, barH2, ColorAlpha(BLACK, 0.55f));

            // Fill
            int filled = (int)(barInnerW * pct);
            if (filled > 0) {
                DrawRectangle(barX, by+13, filled, barH2, ColorAlpha(bars[b].col, barAlpha));
                DrawRectangle(barX, by+13, filled, barH2/3, ColorAlpha(WHITE, 0.12f * barAlpha));
            }
            DrawRectangleLinesEx({(float)barX,(float)(by+13),(float)barInnerW,(float)barH2},
                                 1.0f, ColorAlpha(bars[b].col, 0.3f * barAlpha));

            // Multiplier value
            DrawText(TextFormat("x%.1f", bars[b].value),
                     cx + cardW - 34, by + 13, 8, ColorAlpha(bars[b].col, barAlpha * 0.9f));
        }

        // ── Info line ─────────────────────────────────────────────────────────
        int infoY = barsStartY + 3 * barSpacing + 2;
        DrawLine(cx+4, infoY, cx+cardW-4, infoY, ColorAlpha(ds.labelColor, 0.20f));
        DrawText(TextFormat("XP:%.1fx  $:%.1fx",  ds.xpMult, ds.creditMult),
                 cx+8, infoY+5, 9, ColorAlpha(Color{160,220,255,255}, alpha * 0.75f));
        DrawText(TextFormat("Boss HP: x%.1f",     ds.bossHPMult),
                 cx+8, infoY+18, 9, ColorAlpha(Color{255,160,100,255}, alpha * 0.65f));

        // ── "SELECIONADO" badge at bottom ──────────────────────────────────
        if (isSel) {
            DrawRectangle(cx+6, cy+cardH-22, cardW-12, 18,
                         ColorAlpha(ds.labelColor, 0.25f));
            const char* selTxt = "SELECIONADO";
            DrawText(selTxt, cx + cardW/2 - MeasureText(selTxt,11)/2,
                     cy+cardH-20, 11, ds.labelColor);
        }
    }

    // ── Instructions ───────────────────────────────────────────────────────────
    int hy = startY + cardH + 18;
    const char* h1 = "< Setas/Mouse: navegar >";
    const char* h2 = "ENTER ou clique: confirmar";
    const char* h3 = "ESC: voltar";
    DrawText(h1, screenWidth/2 - MeasureText(h1,13)/2, hy,    13, ColorAlpha(WHITE, 0.55f));
    DrawText(h2, screenWidth/2 - MeasureText(h2,14)/2, hy+20, 14, ColorAlpha({0,235,255,255}, 0.85f));
    DrawText(h3, screenWidth/2 - MeasureText(h3,12)/2, hy+42, 12, ColorAlpha(WHITE, 0.38f));

    EndTextureMode();
}

// ─── Juice de combate: decalques de chão (sangue/queimado) ───────────────────
void Game::addDecal(Vector2 p, Color c, int type, float size) {
    if (decals.size() > 120) decals.erase(decals.begin());  // teto p/ perf
    decals.push_back({ p, c, 10.0f, 10.0f, size, type });
}

void Game::renderDecals() const {
    Vector2 cam = camera.target;
    for (const auto& d : decals) {
        if (std::fabs(d.pos.x - cam.x) > 1000 || std::fabs(d.pos.y - cam.y) > 650) continue;
        float a = (d.life / d.maxLife);   // some aos poucos
        if (d.type == 0) { // mancha de sangue — manchas irregulares
            DrawEllipse((int)d.pos.x, (int)d.pos.y, d.size, d.size*0.6f, ColorAlpha(d.color, 0.45f*a));
            DrawCircleV({d.pos.x - d.size*0.4f, d.pos.y + 2}, d.size*0.35f, ColorAlpha(d.color, 0.4f*a));
            DrawCircleV({d.pos.x + d.size*0.5f, d.pos.y - 1}, d.size*0.3f,  ColorAlpha(d.color, 0.35f*a));
        } else {           // marca de queimado/faísca — escuro com brasa
            DrawCircleV(d.pos, d.size*0.7f, ColorAlpha(Color{20,18,16,255}, 0.5f*a));
            DrawCircleLines((int)d.pos.x, (int)d.pos.y, d.size*0.7f, ColorAlpha(Color{255,120,30,255}, 0.3f*a));
        }
    }
}

// ─── Run / Update ────────────────────────────────────────────────────────────

void Game::runAutoTest(bool autoTest) {
    if (autoTest) {
        // Limpa screenshots de runs anteriores: shot_NN.png antigo misturado com
        // o do run atual vira evidencia falsa (runs indistinguiveis no mesmo dir).
        try {
            for (const auto& e : std::filesystem::directory_iterator(".")) {
                const std::string fn = e.path().filename().string();
                if (fn.rfind("shot_", 0) == 0 && e.path().extension() == ".png")
                    std::filesystem::remove(e.path());
            }
        } catch (...) { /* sem permissao/dir estranho: segue o jogo */ }
        // Skip menu, start game immediately with bot active
        buildQuests();
        // O mundo ja foi construido UMA vez no construtor (com safeZoneCenter
        // correto) — regenerar aqui era a 2a construcao descartavel do log SCENERY.
        setupZoneNPCs(currentZone);
        // Sem isto o --autotest parava no MENU esperando um ENTER humano: o bot
        // so roda depois que a partida comeca. "Skip menu" era so o comentario.
        inMainMenu = false;
        audio.stopMenuMusic();
        botController.active   = true;
        botController.autoTest = true;
        botController.testDuration = (autoTestSeconds > 0.0f) ? autoTestSeconds : 7200.0f;
        botController.addLog("=== AUTO-BOT TEST MODE ATIVADO ===");
        botController.addLog("Duracao maxima: 7200s (2h)");
        startNetwork();   // testa o cliente WebSocket (multiplayer)
        startStore();     // testa login + catalogo da loja premium
    }
    run();
    // After run() exits, write report if bot was active
    if (botController.active || autoTest) {
        botController.writeReport("bot_report.txt");   // relativo ao CWD: funciona em qualquer maquina/CI
        // PORTAO DE VALIDACAO: veredito no console; o exit code sai por main.cpp.
        std::vector<std::string> why;
        autoTestPassed = botController.passed(&why);
        TraceLog(LOG_INFO, "VALIDACAO: %s", autoTestPassed ? "PASSOU" : "FALHOU");
        for (const auto& w : why) TraceLog(LOG_WARNING, "VALIDACAO: %s", w.c_str());
    }
}

void Game::runHeadless() {
    // CI/validacao sem display nem GPU: roda a MESMA update() do jogo em tempo
    // real (dt real do relogio), sem documento/menu/render/screenshot. O FPS
    // reportado ao bot e medido deste proprio loop (GetFPS() fica em 0 sem janela).
    auto tLast = std::chrono::steady_clock::now();
    while (!quitRequested) {
        auto tNow = std::chrono::steady_clock::now();
        float dt  = std::chrono::duration<float>(tNow - tLast).count();
        tLast     = tNow;
        if (dt <= 0.0f) {
            std::this_thread::yield();
            continue;
        }
        if (dt > 0.25f) dt = 0.25f;   // mesma quarentena da janela (frame de carga)
        headlessFps = 1.0f / dt;
        update(dt);
        // update() pode marcar quitRequested (autoteste concluido/fim do jogo)
    }
}

void Game::run() {
    if (headless) { runHeadless(); return; }
    bool menuMusicStarted = false;
    while (!WindowShouldClose() && !quitRequested) {
        float dt = GetFrameTime();

        if (inMainMenu) {
            // Start menu music once
            if (!menuMusicStarted) { audio.playMenuMusic(); menuMusicStarted = true; }
            audio.updateMusic();

            bool hasSave = SaveManager::exists();
            Vector2 mouse = virtualizeMousePos(GetMousePosition());
            int cx = screenWidth/2;
            int cy = screenHeight/2;
            int bw = 360;
            // Button Y positions matching drawMainMenu
            int btn0y = hasSave ? cy - 2  : cy + 20;
            int btn1y = hasSave ? cy + 44 : cy + 66;
            int btn2y = hasSave ? cy + 90 : -999;

            auto hitBtn = [&](int y) {
                return mouse.x >= cx-bw/2 && mouse.x <= cx+bw/2 &&
                       mouse.y >= y-3     && mouse.y <= y+33;
            };

            // Hover tracking
            menuHoveredBtn = -1;
            if (hitBtn(btn0y)) menuHoveredBtn = 0;
            else if (hitBtn(btn1y)) menuHoveredBtn = 1;
            else if (btn2y > 0 && hitBtn(btn2y)) menuHoveredBtn = 2;

            bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

            // ── Difficulty overlay ────────────────────────────────────────────
            if (selectingDifficulty) {
                const int dCW = 196, dCG = 10;
                const int dSX = (screenWidth - (5*dCW + 4*dCG)) / 2;
                const int dSY = 150, dCH = 268;
                for (int i = 0; i < 5; ++i) {
                    int bx = dSX + i*(dCW+dCG);
                    if (mouse.x >= bx && mouse.x <= bx+dCW &&
                        mouse.y >= dSY && mouse.y <= dSY+dCH)
                        difficultyHovered = i;
                }
                if (IsKeyPressed(KEY_LEFT)  && difficultyHovered > 0) difficultyHovered--;
                if (IsKeyPressed(KEY_RIGHT) && difficultyHovered < 4) difficultyHovered++;
                bool dConf = false;
                if (mouseClicked) {
                    for (int i = 0; i < 5; ++i) {
                        int bx = dSX + i*(dCW+dCG);
                        if (mouse.x >= bx && mouse.x <= bx+dCW &&
                            mouse.y >= dSY && mouse.y <= dSY+dCH)
                        { difficultyHovered = i; dConf = true; break; }
                    }
                }
                if (IsKeyPressed(KEY_ENTER)) dConf = true;
                if (dConf) {
                    difficulty = (DifficultyLevel)difficultyHovered;
                    selectingDifficulty = false;
                    if (pendingNewGame) {
                        // Novo jogo: escolher PERSONAGEM antes de comecar
                        selectingCharacter = true;
                        characterHovered   = 0;
                        drawMainMenu(); drawCharacterSelectScreen(); presentFrame(); continue;
                    } else {
                        if (hasSave) SaveManager::load(player, quests, currentZone, 0, &totalKills, nullptr, &newGamePlus);
                        if (openWorldMode) {
                            // Reconstroi a fase a partir da zona salva (ver startLoadedGame).
                            owPhase = 0; owPhaseRadius = 3000.0f; owPhaseGoal = 20; owBossPhase = false;
                            for (int i = 0; i < (int)phaseDefs.size(); ++i)
                                if (phaseDefs[i].zone == currentZone) {
                                    owPhase = i; owPhaseGoal = phaseDefs[i].goal;
                                    owPhaseRadius = phaseDefs[i].radius; owBossPhase = phaseDefs[i].boss; break;
                                }
                            owPhaseKills = 0; owKillsAtStart = 0; owBossDown = false; owPortalOpen = false;
                            tilemap.generateOpenWorld();
                            setupWorldRegions();
                            buildOpenWorldScenery();
                            currentRegion = currentZone;
                        } else {
                            tilemap.generate(currentZone);
                        }
                        setupZoneNPCs(currentZone);
                        spawnInterval = getZoneInfo(currentZone).spawnInterval / getDifficulty().spawnRateMult;
                        inMainMenu = false;
                        audio.stopMenuMusic(); audio.setZone(currentZone);
                        triggerPlayerSpeech("Missao iniciada. Eliminando ameacas KRONOS.", 4.0f);
                    }
                    drawMainMenu(); presentFrame(); continue;
                }
                if (IsKeyPressed(KEY_ESCAPE)) selectingDifficulty = false;
                drawMainMenu(); drawDifficultyScreen(); presentFrame(); continue;
            }

            // ── Selecao de PERSONAGEM (apos a dificuldade, em novo jogo) ──────
            if (selectingCharacter) {
                const int total = (int)CharacterClass::COUNT; // 6
                const int cardW = 188, cardG = 10;
                const int totalW = total*cardW + (total-1)*cardG;
                const int csx = (screenWidth - totalW) / 2;
                const int csy = 150, cardH = 300;
                for (int i = 0; i < total; ++i) {
                    int bx = csx + i*(cardW+cardG);
                    if (mouse.x >= bx && mouse.x <= bx+cardW &&
                        mouse.y >= csy && mouse.y <= csy+cardH)
                        characterHovered = i;
                }
                if (IsKeyPressed(KEY_LEFT)  && characterHovered > 0)         characterHovered--;
                if (IsKeyPressed(KEY_RIGHT) && characterHovered < total-1)   characterHovered++;
                bool cConf = false;
                if (mouseClicked) {
                    for (int i = 0; i < total; ++i) {
                        int bx = csx + i*(cardW+cardG);
                        if (mouse.x >= bx && mouse.x <= bx+cardW &&
                            mouse.y >= csy && mouse.y <= csy+cardH)
                        { characterHovered = i; cConf = true; break; }
                    }
                }
                if (IsKeyPressed(KEY_ENTER)) cConf = true;
                if (IsKeyPressed(KEY_ESCAPE)) { selectingCharacter = false; selectingDifficulty = true; }
                if (cConf) {
                    player.applyClass((CharacterClass)characterHovered);
                    selectingCharacter = false;
                    startNewGame();
                    drawMainMenu(); presentFrame(); continue;
                }
                drawMainMenu(); drawCharacterSelectScreen(); presentFrame(); continue;
            }

            // Botao 0 / ENTER:
            //  - com save  = CONTINUAR (carrega direto, SEM tela de dificuldade)
            //  - sem save  = NOVO JOGO (mostra dificuldade)
            if (IsKeyPressed(KEY_ENTER) || (mouseClicked && menuHoveredBtn == 0)) {
                if (hasSave) {
                    startLoadedGame();
                    presentFrame();
                    continue;
                } else {
                    pendingNewGame = true; selectingDifficulty = true;
                }
            }
            // Novo jogo (tecla N ou botao 1, so existe quando ha save) — mostra dificuldade
            if (IsKeyPressed(KEY_N) || (mouseClicked && hasSave && menuHoveredBtn == 1)) {
                pendingNewGame = true; selectingDifficulty = true;
            }
            if (IsKeyPressed(KEY_ESCAPE) || (mouseClicked && menuHoveredBtn == (hasSave ? 2 : 1))) {
                break;
            }
            drawMainMenu();
            if (selectingDifficulty) drawDifficultyScreen();
            presentFrame();
            continue;
        }

        // F11 — toggle fullscreen (RenderTexture handles scaling)
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (shopSystem.open) { shopSystem.close(); continue; }
            if (craftingSystem.open) { craftingSystem.open = false; continue; }
            if (dialogOpen) { dialogOpen = false; continue; }
            paused = !paused;
            // Renderiza e PULA o processamento de input deste frame, senao o
            // mesmo ESC seria lido pelo menu de pause e fecharia na hora.
            render();
            presentFrame();
            continue;
        }

        if (paused) {
            // Menu de pause (9 opcoes): Continuar/Salvar/Dificuldade/Trilha/Efeitos/
            //                            Vozes/Reiniciar/Menu/Sair
            const int PAUSE_OPTS = 9;
            Vector2 pm = virtualizeMousePos(GetMousePosition());
            int pcx = screenWidth / 2;
            int pby = screenHeight / 2 - 150;  // mesma base do drawPauseMenu
            int pbw = 340, pbh = 32, pgap = 6;
            pauseHovered = -1;
            for (int i = 0; i < PAUSE_OPTS; ++i) {
                int y = pby + i * (pbh + pgap);
                if (pm.x >= pcx - pbw/2 && pm.x <= pcx + pbw/2 &&
                    pm.y >= y && pm.y <= y + pbh) pauseHovered = i;
            }
            // Navegacao por teclado
            if (IsKeyPressed(KEY_DOWN)) pauseHovered = (pauseHovered + 1 + PAUSE_OPTS) % PAUSE_OPTS;
            if (IsKeyPressed(KEY_UP))   pauseHovered = (pauseHovered - 1 + PAUSE_OPTS) % PAUSE_OPTS;

            int chosen = -1;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && pauseHovered >= 0) chosen = pauseHovered;
            if (IsKeyPressed(KEY_ENTER) && pauseHovered >= 0) chosen = pauseHovered;
            // Atalhos diretos
            if (IsKeyPressed(KEY_ESCAPE)) chosen = 0;   // ESC continua
            if (IsKeyPressed(KEY_F5))     chosen = 1;   // F5 salva

            if (chosen == 0) {            // Continuar
                paused = false;
            } else if (chosen == 1) {     // Salvar
                autoSave();
                showStoryBanner("JOGO SALVO", "Progresso gravado com sucesso.", 2.0f);
            } else if (chosen == 2) {     // Dificuldade — cicla e aplica (continua pausado)
                difficulty = (DifficultyLevel)(((int)difficulty + 1) % 5);
                spawnInterval = getZoneInfo(currentZone).spawnInterval / getDifficulty().spawnRateMult;
            } else if (chosen == 3) {     // Trilha sonora ON/OFF
                audio.setMusicEnabled(!audio.musicEnabled);
            } else if (chosen == 4) {     // Todos os sons ON/OFF (master)
                audio.setAllSoundOn(!audio.allSoundOn);
            } else if (chosen == 5) {     // Vozes/personagens ON/OFF
                audio.setVoiceEnabled(!audio.voiceEnabled);
            } else if (chosen == 6) {     // Reiniciar partida
                paused = false;
                restartRun();
            } else if (chosen == 7) {     // Voltar ao menu principal
                paused = false;
                inMainMenu = true;
            } else if (chosen == 8) {     // Sair do jogo
                quitRequested = true;
            }

            render();       // drawPauseMenu() is called inside render() when paused
            presentFrame();
            continue;
        }

        auto _t0 = std::chrono::high_resolution_clock::now();
        update(dt);
        if (WindowShouldClose()) break;  // bot may have called CloseWindow()
        auto _t1 = std::chrono::high_resolution_clock::now();
        render();
        auto _t2 = std::chrono::high_resolution_clock::now();
        if (botController.active) {
            float um = std::chrono::duration<float, std::milli>(_t1 - _t0).count();
            float rm = std::chrono::duration<float, std::milli>(_t2 - _t1).count();
            if (um > botController.peakUpdateMs) botController.peakUpdateMs = um;
            if (rm > botController.peakRenderMs) botController.peakRenderMs = rm;
        }
        presentFrame();
    }
}

void Game::startNewGame() {
    // Inicia a partida do zero apos escolher dificuldade e personagem.
    victoryReported = false;
    tutorial.init();
    tutorialRewardGiven = false;
    achievements.playerPtr = &player;
    achievements.audioPtr = &audio;
    buildQuests();
    currentZone   = ZoneID::LARuins;
    currentRegion = ZoneID::LARuins;
    if (openWorldMode) {
        // Reset completo do estado de fase: o mundo novo comeca na fase 1 do
        // zero (raio/bioma/c1da fase). Antes owPhaseRadius ou owPhaseGoal podiam
        // sobrar da partida anterior — o "Novo Jogo" herdava fase advanced.
        const PhaseDef& pd0 = phaseDef(0);
        owPhase = 0; owPhaseKills = 0; owKillsAtStart = 0;
        owPhaseGoal = pd0.goal; owPhaseRadius = pd0.radius;
        owBossPhase = pd0.boss; owBossDown = false; owPortalOpen = false;
        tilemap.generateOpenWorld();
        setupWorldRegions();
        float ox = (float)(Tilemap::OW_ZONE_W * Tilemap::tileSize) / 2.0f;
        float oy = (float)(Tilemap::OW_ZONE_H * Tilemap::tileSize) / 2.0f;
        player.position = {ox, oy};
        safeZoneCenter  = {ox, oy};   // ANTES do cenario (ele limita pela barreira da fase)
        buildOpenWorldScenery();
    } else {
        tilemap.generate(currentZone);
    }
    setupZoneNPCs(currentZone);
    spawnInterval = getZoneInfo(currentZone).spawnInterval / getDifficulty().spawnRateMult;
    inMainMenu = false; storyChapter = 1;
    // Reset do motor de evolucao para a nova partida
    threatLevel = 1; threatTimer = 0.0f; threatKillMark = 0;
    activeMutator = WorldMutator::None; mutatorTimer = 0.0f;
    totalKills = 0; enemiesKilled = 0; sessionTime = 0.0f;
    audio.stopMenuMusic(); audio.setZone(currentZone);
    showStoryBanner("CAPITULO 1: O JULGAMENTO",
        "2047 - KRONOS domina. O NEXUS e a ultima esperanca da humanidade.", 5.0f);
    triggerPlayerSpeech(TextFormat("%s pronto para o combate.",
                        Player::className(player.charClass)), 4.0f);
    startNetwork();   // multiplayer em tempo real (mostra outros jogadores)
    startStore();     // loja premium (login + catalogo de gems)
}

void Game::drawCharacterSelectScreen() const {
    BeginTextureMode(gameTarget.get());  // overlay sobre o menu
    float t = (float)GetTime();
    int cx = screenWidth / 2;

    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.86f));

    float tp = 0.9f + 0.1f * std::sin(t * 1.4f);
    const char* title = "ESCOLHA SEU PERSONAGEM";
    int tFont = 36;
    int tw = MeasureText(title, tFont);
    int txc = cx - tw/2;
    DrawText(title, txc + 3, 83, tFont, ColorAlpha({0,140,255,255}, 0.25f));
    DrawText(title, txc,     80, tFont, ColorAlpha({0,235,255,255}, tp));
    int bl = tw / 2 + 20;
    DrawLine(cx - bl, 128, cx + bl, 128, ColorAlpha({0,235,255,255}, 0.35f));
    DrawLine(cx - bl - 8, 121, cx - bl, 129, ColorAlpha({0,235,255,255}, 0.6f));
    DrawLine(cx + bl + 8, 121, cx + bl, 129, ColorAlpha({0,235,255,255}, 0.6f));
    DrawRectangle(cx + bl - 2, 121, 2, 8, ColorAlpha({255,180,40,255}, 0.9f));
    const char* sub = "Cada classe tem visual, stats e estilo proprios.";
    int sw = MeasureText(sub, 16);
    DrawText(sub, cx - sw/2, 136, 16, ColorAlpha({175,195,220,255}, 0.65f));

    const int total = (int)CharacterClass::COUNT;
    const int cardW = 188, cardG = 10;
    const int totalW = total*cardW + (total-1)*cardG;
    const int csx = (screenWidth - totalW) / 2;
    const int csy = 150, cardH = 300;

    // Cores por classe (combinam com o visual do Player)
    const Color cardCols[6] = {
        {60,90,150,255},   // Soldado
        {180,70,120,255},  // Guerreira
        {120,130,150,255}, // Robo
        {90,60,160,255},   // Mago
        {140,60,170,255},  // Bruxa
        {150,90,40,255},   // HomemFera
    };

    for (int i = 0; i < total; ++i) {
        int bx = csx + i*(cardW+cardG);
        bool sel = (i == characterHovered);
        Color col   = cardCols[i];
        Color cardBg = sel ? Color{10,22,38,255} : Color{8,14,26,255};
        // Card (painel chanfrado, trilho de energia da cor da classe)
        DrawRectangle(bx, csy, cardW, cardH, ColorAlpha(cardBg, sel ? 0.92f : 0.84f));
        DrawRectangle(bx, csy, 3, cardH, ColorAlpha(col, sel ? 1.0f : 0.45f));
        if (sel) { // topo pulsante
            float p = 0.5f + 0.5f * std::sin(t*4.0f);
            DrawRectangle(bx, csy, cardW, 3, ColorAlpha(Color{0,235,255,255}, p));
        }
        int cut = 8;
        Color brd = sel ? Color{0,235,255,255} : ColorAlpha(col, 0.65f);
        DrawLine(bx+cut, csy,    bx+cardW-cut, csy,    brd);
        DrawLine(bx,     csy+cut, bx,    csy+cardH-cut, brd);
        DrawLine(bx+cut, csy+cardH, bx+cardW-cut, csy+cardH, brd);
        DrawLine(bx+cardW, csy+cut, bx+cardW, csy+cardH-cut, brd);
        DrawLine(bx,     csy+cut,  bx+cut, csy,     brd);
        DrawLine(bx+cardW-cut, csy, bx+cardW, csy+cut, brd);
        DrawLine(bx,     csy+cardH-cut, bx+cut, csy+cardH, brd);
        DrawLine(bx+cardW-cut, csy+cardH, bx+cardW, csy+cardH-cut, brd);
        if (sel) { // pulso externo
            float p2 = 0.45f + 0.35f * std::sin(t*3.0f);
            DrawLine(bx-2, csy-2, bx+cardW+2, csy-2, ColorAlpha(Color{0,235,255,255}, p2));
            DrawLine(bx-2, csy+cardH+2, bx+cardW+2, csy+cardH+2, ColorAlpha(Color{0,235,255,255}, p2));
        }
        // Nome
        CharacterClass cc = (CharacterClass)i;
        const char* nm = Player::className(cc);
        int nw = MeasureText(nm, 20);
        DrawText(nm, bx + cardW/2 - nw/2, csy + 12, 20, sel ? WHITE : col);

        // Avatar (retrato pixel-art da classe)
        int ax = bx + cardW/2, ay = csy + 108;
        SpriteBank& sb = SpriteBank::get();
        if (sb.ready && i < SpriteBank::NUM_CHAR_AVATARS) {
            Texture2D av = sb.charAvatar[i];
            float scale = 1.0f;
            float aw = av.width * scale, ah = av.height * scale;
            // halo da cor da classe atras
            DrawCircle(ax, ay, 50, ColorAlpha(col, 0.18f));
            DrawTexturePro(av, {0,0,(float)av.width,(float)av.height},
                           {ax - aw/2, ay - ah/2, aw, ah}, {0,0}, 0.0f, WHITE);
        } else {
            DrawCircle(ax, ay, 42, ColorAlpha(col, 0.25f));
            DrawCircleLines(ax, ay, 42.0f, ColorAlpha(col, 0.8f));
        }

        // Descricao + sabor
        const char* desc = Player::classDescription(cc);
        std::string d = desc ? desc : "";
        // quebra simples
        int ty = csy + 170, lineMax = 22;
        std::string word, line;
        std::istringstream iss(d);
        while (iss >> word) {
            std::string test = line.empty() ? word : line + " " + word;
            if ((int)test.size() > lineMax) {
                DrawText(line.c_str(), bx + 10, ty, 11, ColorAlpha(WHITE, 0.8f));
                ty += 15; line = word;
            } else line = test;
        }
        if (!line.empty()) DrawText(line.c_str(), bx + 10, ty, 11, ColorAlpha(WHITE, 0.8f));

        const char* fant = Player::classFantasy(cc);
        if (fant) {
            std::string f = fant; std::string l2, w2;
            int fy = csy + cardH - 64; std::istringstream iss2(f);
            while (iss2 >> w2) {
                std::string test = l2.empty() ? w2 : l2 + " " + w2;
                if ((int)test.size() > 24) {
                    DrawText(l2.c_str(), bx + 10, fy, 10, ColorAlpha(col, 0.9f));
                    fy += 13; l2 = w2;
                } else l2 = test;
            }
            if (!l2.empty()) DrawText(l2.c_str(), bx + 10, fy, 10, ColorAlpha(col, 0.9f));
        }
    }

    {
        const char* hk = "Setas/Mouse para escolher  -  ENTER/Clique para confirmar  -  ESC volta";
        int hw = MeasureText(hk, 14);
        int hy = csy + cardH + 22;
        DrawRectangle(cx - hw/2 - 16, hy - 8, hw + 32, 26, ColorAlpha({8,16,34,255}, 0.85f));
        int hc = 6;
        DrawLine(cx-hw/2 - 16 + hc, hy - 8, cx+hw/2 + 16 - hc, hy - 8, ColorAlpha({0,235,255,255}, 0.5f));
        DrawLine(cx-hw/2 - 16, hy - 8 + hc, cx-hw/2 - 16, hy + 18 - hc, ColorAlpha({0,235,255,255}, 0.5f));
        DrawLine(cx-hw/2 - 16 + hc, hy + 18, cx+hw/2 + 16 - hc, hy + 18, ColorAlpha({0,235,255,255}, 0.5f));
        DrawLine(cx+hw/2 + 16, hy - 8 + hc, cx+hw/2 + 16, hy + 18 - hc, ColorAlpha({0,235,255,255}, 0.5f));
        DrawText(hk, cx - hw/2, hy - 3, 14, ColorAlpha({200,215,235,255}, 0.75f));
    }
    EndTextureMode();
}

void Game::startLoadedGame() {
    // Carrega o save e entra direto no jogo — SEM tela de dificuldade.
    // A dificuldade salva e mantida (so muda em Novo Jogo ou pelo menu de pause).
    std::vector<std::string> buildingLines;
    if (SaveManager::exists()) SaveManager::load(player, quests, currentZone, 0, &totalKills, &buildingLines, &newGamePlus);

    // Limpa estado residual da sessao anterior antes de reconstruir o mundo.
    enemies.clear();
    items.clear();
    projectiles.clear();
    enemyProjectiles.clear();
    xpOrbs.clear();
    groundEquips.clear();
    companions.clear();
    damageNumbers.clear();
    particles.particles.clear();
    anomalySystem.portals.clear();
    anomalySystem.waveActive = false;

    // Ow phase state nao fica no .json: reconstruo owPhase/raio/meta/boss a partir
    // da ZONA salva, senao as regioes nascem para a fase 1 (raio 5200) num save de
    // phase 10 (raio 7800) — grid menor que a barreira, anel externo sem cenario.
    {
        owPhase = 0; owPhaseRadius = 3000.0f; owPhaseGoal = 20; owBossPhase = false;
        for (int i = 0; i < (int)phaseDefs.size(); ++i) {
            if (phaseDefs[i].zone == currentZone) {
                owPhase = i; owPhaseGoal = phaseDefs[i].goal;
                owPhaseRadius = phaseDefs[i].radius; owBossPhase = phaseDefs[i].boss;
                break;
            }
        }
        owPhaseKills = 0; owKillsAtStart = 0; owBossDown = false; owPortalOpen = false;
    }
    player.unclaimedLevels = 0;   // nivel veio do arquivo; nao e level-up novo
    if (openWorldMode) {
        tilemap.generateOpenWorld();
        setupWorldRegions();
        buildOpenWorldScenery();
        currentRegion = currentZone;
    } else {
        tilemap.generate(currentZone);
    }
    setupZoneNPCs(currentZone);

    // Restaura construcoes e unidades aliadas do save.
    if (!buildingLines.empty()) buildingSystem.load(buildingLines);

    spawnInterval = getZoneInfo(currentZone).spawnInterval / getDifficulty().spawnRateMult;
    inMainMenu = false;
    selectingDifficulty = false;
    audio.stopMenuMusic(); audio.setZone(currentZone);
    triggerPlayerSpeech("Partida carregada. Retomando a missao.", 4.0f);
    startNetwork();   // multiplayer em tempo real
    startStore();     // loja premium (login + catalogo de gems)
}

void Game::restartRun() {
    // Reset do jogador (o construtor reconfigura skills e stats base)
    victoryReported = false;
    player = Player();
    tutorial.init();
    tutorialRewardGiven = false;
    achievements.playerPtr = &player;
    achievements.audioPtr = &audio;

    // Limpa todas as entidades em jogo
    enemies.clear();
    items.clear();
    projectiles.clear();
    enemyProjectiles.clear();
    xpOrbs.clear();
    groundEquips.clear();
    companions.clear();
    damageNumbers.clear();
    particles.particles.clear();
    buildingSystem.buildings.clear();
    buildingSystem.tanks.clear();
    buildingSystem.soldiers.clear();
    buildingSystem.buildModeActive = false;

    // Reset de progressao e flags
    enemiesKilled       = 0;
    totalKills          = 0;
    sessionTime         = 0.0f;
    storyChapter        = 1;
    omegaKillThreshold  = 50;
    threatLevel         = 1;
    threatTimer         = 0.0f;
    threatKillMark      = 0;
    activeMutator       = WorldMutator::None;
    mutatorTimer        = 0.0f;
    gameWon             = false;
    victoryTimer        = 0.0f;
    finalBossSpawned    = false;
    finalBossAlive      = false;
    pendingLevelUps     = 0;
    pendingEvolutions   = 0;
    showLevelUpScreen   = false;
    showEvolutionScreen = false;
    showSkillTree   = false;
    perkCursor      = 0;
    showInventory = showEquipment = showQuestLog = false;
    shopSystem.close();
    craftingSystem.open = false;
    hasTarget   = false;
    rtsDragging = false;
    rtsHasUnits = false;

    // ── Reset do bot/autotest (eram static de funcao — vazavam entre partidas) ─
    botReportSaveTimer = 0.0f;
    botAllyTimer       = 2.0f;
    botBuildTimer      = 4.0f;
    botProduceTimer    = 8.0f;
    botUpgradeTimer    = 12.0f;
    botStipendTimer    = 0.0f;
    botBuildCycle      = 0;
    lastShot           = 0.0;
    shotN              = 0;
    botController.reset();       // telemetria, estado, timers e rota cacheada
    Companion::resetSpawnIndex(); // vagas de formacao voltam ao inicio

    // Reconstroi quests e mundo
    quests.clear();
    buildQuests();
    currentZone   = ZoneID::LARuins;
    currentRegion = ZoneID::LARuins;
    if (openWorldMode) {
        // Mesmo reset de fase do Novo Jogo: retorno "partida reiniciada" tambem
        // volta para a fase 1 (raio/bioma corretos antes de gerar o cenario).
        const PhaseDef& pd0 = phaseDef(0);
        owPhase = 0; owPhaseKills = 0; owKillsAtStart = 0;
        owPhaseGoal = pd0.goal; owPhaseRadius = pd0.radius;
        owBossPhase = pd0.boss; owBossDown = false; owPortalOpen = false;
        tilemap.generateOpenWorld();
        setupWorldRegions();
        buildOpenWorldScenery();
        float ox = (float)(Tilemap::OW_ZONE_W * Tilemap::tileSize) / 2.0f;
        float oy = (float)(Tilemap::OW_ZONE_H * Tilemap::tileSize) / 2.0f;
        player.position = {ox, oy};
    } else {
        tilemap.generate(currentZone);
    }
    setupZoneNPCs(currentZone);
    infernoZone.active = false;
    darkWorld.active   = false;
    darkZoneActive     = false;
    lightSystem.setEnabled(false);
    anomalySystem.storm.stop();
    audio.setZone(currentZone);

    showStoryBanner("PARTIDA REINICIADA", "Uma nova tentativa contra o KRONOS.", 3.0f);
    triggerPlayerSpeech("Reiniciando sistemas de combate.", 3.0f);
}


void Game::grantQuestRewards(Quest& q) {
    q.complete();
    if (q.rewardHP  > 0.0f) player.heal(q.rewardHP);
    audio.playHeal();
    if (q.rewardXP  > 0)    player.addXP(q.rewardXP);
    if (!q.rewardEquip.isEmpty()) player.equipItem(q.rewardEquip);
    particles.spawnLevelUp(player.position);
    audio.playLevelUp();
}

// ─── Input ───────────────────────────────────────────────────────────────────


// ─── Collisions ──────────────────────────────────────────────────────────────

// Unico ponto que converte niveis ganhos em pontos/evolucoes pendentes. Vale para
// QUALQUER fonte de XP (orbe, quest, TechChip, item usado do inventario), inclusive
// as que ficam dentro de Player e o Game nao enxerga.
void Game::drawFloatingNumbers(bool project3D) const {
    for (const auto& dn : damageNumbers) {
        float alpha = std::min(dn.life / 0.45f, 1.0f);
        Color c = ColorAlpha(dn.color, alpha);
        // Fonte menor para nao poluir a tela perto do personagem
        int fontSize = (dn.value >= 100.0f) ? 15 :
                       (dn.value >= 50.0f)  ? 13 : 11;
        const char* txt = TextFormat("%s%.0f", dn.prefix.c_str(), dn.value);
        int tw = MeasureText(txt, fontSize);
        Vector2 p;
        if (project3D) {
            // Sobe de verdade no eixo Y do mundo 3D e so entao vira coord de tela.
            p = GetWorldToScreenEx({ dn.pos.x, 30.0f + dn.rise, dn.pos.y },
                                   camera3D, screenWidth, screenHeight);
        } else {
            p = { dn.pos.x, dn.pos.y - dn.rise - 14.0f };
        }
        // Shadow for readability
        DrawText(txt, (int)p.x - tw/2 + 1, (int)p.y + 1, fontSize, ColorAlpha(BLACK, 0.6f * alpha));
        DrawText(txt, (int)p.x - tw/2,     (int)p.y,     fontSize, c);
    }
}

void Game::drainLevelUps() {
    int gained = player.unclaimedLevels;
    if (gained <= 0) return;
    player.unclaimedLevels = 0;

    tutorial.onLeveledUp();
    achievements.onLevelUp(player.level);

    particles.spawnLevelUp(player.position);
    audio.playLevelUp();
    static const char* lvlLines[] = {
        "Estou ficando mais forte.",
        "Experiencia e a melhor arma.",
        "KRONOS nao sabe o que vem ai.",
        "Modulo de combate expandido.",
        "Capacidade elevada. Missao continua."
    };
    triggerPlayerSpeech(lvlLines[player.level % 5], 3.0f);
    // NAO trava o jogo — apenas acumula pontos e avisa o jogador.
    // Ele escolhe quando quiser: tecla L (level up) / tecla K (evolucao).
    levelUpAnimTimer   = 0.0f;
    pendingNotifyPulse = 1.0f;

    // Quantos dos niveis CRUZADOS sao de evolucao (conta cada um; subir 2 de uma
    // vez passando por 10 e 11 da 1 evolucao + 1 ponto).
    static const int EVO_LEVELS[] = {10, 25, 40, 60};
    int evo = 0;
    for (int l = player.level - gained + 1; l <= player.level; ++l)
        for (int el : EVO_LEVELS) if (l == el) { ++evo; break; }

    pendingEvolutions += evo;
    pendingLevelUps   += (gained - evo);
    if (evo > 0) triggerPlayerSpeech("EVOLUCAO disponivel! Pressione K para escolher.", 4.0f);
}

void Game::checkCollisions() {
    // Player projectiles vs enemies
    for (auto& proj : projectiles) {
        if (!proj.active) continue;

        for (auto& enemy : enemies) {
            if (enemy.isDead()) continue;
            // ao quadrado: evita um sqrt por par projetil x inimigo (loop O(n*m) quente)
            float ddx = proj.position.x - enemy.position.x;
            float ddy = proj.position.y - enemy.position.y;
            float rsum = enemy.radius + proj.radius;
            if (ddx*ddx + ddy*ddy <= rsum*rsum) {
                // Grenade: explode on impact
                if (proj.isGrenade) {
                    if (!proj.exploded) {
                        proj.exploded = true;
                        for (auto& e : enemies) {
                            if (Vector2Distance(proj.position, e.position) <= proj.explodeRadius) {
                                e.takeDamage(proj.damage);
                                particles.spawnHit(e.position, Color{255,120,0,255}, 12);
                                audio.playHit();
                            }
                        }
                        particles.spawnExplosion(proj.position, Color{255,140,0,255}, 35);
                        audio.playExplosion();
                        triggerShake(6.0f, 0.25f);
                        hitStopTimer = 0.07f;
                        camPunch = std::max(camPunch, 0.07f);
                    }
                    proj.active = false;
                } else {
                    enemy.takeDamage(proj.damage);
                    particles.spawnHit(enemy.position, Color{0,255,255,255}, 6);
                    proj.active = false;
                    audio.playHit();
                    comboCount++;
                    comboTimer = 2.5f;
                    Color projDmgCol = comboCount >= 5 ? Color{0,255,200,255} : Color{0,255,255,255};
                    damageNumbers.push_back({enemy.position, proj.damage, projDmgCol, 1.0f});
                }
                break;
            }
        }
    }

    // Items pickup — RAIO DE COLETA AUTOMATICA + magnetismo
    // Itens dentro do raio de coleta sao pegos automaticamente; itens dentro do
    // raio de atracao voam em direcao ao jogador.
    const float ftime     = GetFrameTime();
    const float COLLECT_R = player.radius + 52.0f;   // coleta automatica
    const float MAGNET_R  = 230.0f;                   // atracao magnetica
    for (auto it = items.begin(); it != items.end();) {
        float d = Vector2Distance(player.position, it->position);
        // Magnetismo: puxa o item para o jogador quando dentro do raio de atracao
        if (it->pickupDelay <= 0.0f && d > COLLECT_R && d < MAGNET_R) {
            Vector2 dir = Vector2Normalize(Vector2Subtract(player.position, it->position));
            float pull  = (1.0f - d / MAGNET_R) * 560.0f + 140.0f;
            it->position.x += dir.x * pull * ftime;
            it->position.y += dir.y * pull * ftime;
        }
        if (it->pickupDelay <= 0.0f && d <= COLLECT_R) {
            // Coleta REAL (item saindo do vetor) — o bot contava por proximidade
            // (<20px) e o magnetismo/recolha automatica removia o item antes.
            if (botController.active) botController.itemsCollected++;
            tutorial.onItemPickedUp();
            achievements.onItemFound((int)it->rarity);
            switch (it->type) {
                case ItemType::HealthPack:
    audio.playHeal();
                    player.heal(30.0f);
                    damageNumbers.push_back({it->position, 30.0f, {0,210,80,255}, 1.2f, "+"});
                    break;
                case ItemType::Credits:
                    player.credits += it->value;
                    totalCreditsEarned += it->value;
                    achievements.onCreditsEarned(totalCreditsEarned);
                    damageNumbers.push_back({it->position, (float)it->value, {255,210,0,255}, 1.4f, "$"});
                    break;
                case ItemType::TechChip:
                    player.addXP(50);
                    damageNumbers.push_back({it->position, 50.0f, {0,200,255,255}, 1.4f, "XP+"});
                    break;
                case ItemType::NanoCore:
    audio.playHeal();
                    player.increaseBaseMaxHP(25.0f);
                    player.heal(25.0f);
                    damageNumbers.push_back({it->position, 25.0f, {255,80,200,255}, 1.6f, "HP+"});
                    particles.spawnLevelUp(it->position);
                    break;
                case ItemType::PlasmaCell:
                    for (auto& s : player.skills) s.currentCooldown *= 0.3f;
                    damageNumbers.push_back({it->position, 0.0f, {180,0,255,255}, 1.2f, "CD-"});
    audio.playHeal();
                    break;
                case ItemType::ScrapMetal:
                    player.heal(8.0f);
                    player.credits += 8;
                    totalCreditsEarned += 8;
                    achievements.onCreditsEarned(totalCreditsEarned);
                    damageNumbers.push_back({it->position, 8.0f, {160,160,170,255}, 0.9f, "+"});
                    break;
                case ItemType::WeaponPart:
                    player.credits += 20;
                    totalCreditsEarned += 20;
                    achievements.onCreditsEarned(totalCreditsEarned);
                    damageNumbers.push_back({it->position, 20.0f, {255,130,0,255}, 1.1f, "$"});
                    break;
                case ItemType::EnergyCore:
                    // Grants a shield burst
                    player.shieldTimer = std::max(player.shieldTimer, 2.5f);
                    damageNumbers.push_back({it->position, 0.0f, {0,255,255,255}, 1.2f, "SHD"});
                    break;
                // ── Crafting materials — go into inventory for use in crafting ──
                case ItemType::MetalScrap:
                    player.inventory.push_back(*it);
                    damageNumbers.push_back({it->position, 0.f, {180,180,180,255}, 0.9f, "MAT+"});
                    break;
                case ItemType::AlienCarapace:
                    player.inventory.push_back(*it);
                    damageNumbers.push_back({it->position, 0.f, {60,255,80,255}, 0.9f, "MAT+"});
                    break;
                case ItemType::PlasmaCore:
                    player.inventory.push_back(*it);
                    damageNumbers.push_back({it->position, 0.f, {0,180,255,255}, 0.9f, "MAT+"});
                    break;
                case ItemType::NanoFiber:
                    player.inventory.push_back(*it);
                    damageNumbers.push_back({it->position, 0.f, {0,220,200,255}, 0.9f, "MAT+"});
                    break;
                case ItemType::OmegaEssence:
                    player.inventory.push_back(*it);
                    damageNumbers.push_back({it->position, 0.f, {255,215,0,255}, 1.4f, "OMEGA+"});
                    particles.spawnLevelUp(it->position);
                    break;
                default:
                    player.addItem(*it);
                    break;
            }

            // Quest tracking — quests de Collect avancam ao pegar QUALQUER item
            // (exceto creditos). CollectRare avanca apenas em raros ou superiores.
            if (it->type != ItemType::Credits) {
                bool isRareOrBetter = static_cast<int>(it->rarity) >= static_cast<int>(ItemRarity::Rare);
                for (auto& q : quests) {
                    if (q.completed || !q.active) continue;
                    if (q.type == QuestType::Collect) {
                        q.updateProgress(1);
                    } else if (q.type == QuestType::CollectRare && isRareOrBetter) {
                        q.updateProgress(1);
                    }
                    if (q.isComplete() && !q.rewardGiven) grantQuestRewards(q);
                }
            }

            if (it->type != ItemType::Credits && static_cast<int>(it->rarity) >= static_cast<int>(ItemRarity::Rare))
                audio.playItemPickup(static_cast<int>(it->rarity));
            else
                audio.playPickup();
            it = items.erase(it);
        } else {
            ++it;
        }
    }

    // XP orbs — mesmo raio de coleta + magnetismo (atracao mais forte)
    for (auto it = xpOrbs.begin(); it != xpOrbs.end();) {
        float dxp = Vector2Distance(player.position, it->position);
        if (dxp > COLLECT_R && dxp < MAGNET_R + 60.0f) {
            Vector2 dir = Vector2Normalize(Vector2Subtract(player.position, it->position));
            float pull  = (1.0f - dxp / (MAGNET_R + 60.0f)) * 640.0f + 160.0f;
            it->position.x += dir.x * pull * ftime;
            it->position.y += dir.y * pull * ftime;
        }
        if (dxp <= COLLECT_R) {
            player.addXP(it->amount);   // o credito de level-up sai em drainLevelUps()
            // Juice de coleta: faísca ciano + pop flutuante de XP
            if (it->amount >= 3) {
                particles.spawnHit(it->position, Color{120, 240, 255, 255}, 5);
                particles.spawnExplosion(it->position, Color{90, 190, 255, 255}, 3);
            }
            damageNumbers.push_back({it->position, (float)it->amount,
                                     Color{120, 240, 255, 255}, 0.8f, "XP "});
            it = xpOrbs.erase(it);
        } else {
            ++it;
        }
    }
}

// ─── Update Helpers ──────────────────────────────────────────────────────────

void Game::updateItems(float dt) {
    for (auto it = items.begin(); it != items.end();) {
        it->update(dt);
        if (it->lifetime <= 0.0f) it = items.erase(it);
        else ++it;
    }
    // Teto rígido: impede acúmulo ilimitado de drops pelo mundo (causa de FPS=1).
    const size_t MAX_ITEMS = 220;
    if (items.size() > MAX_ITEMS)
        items.erase(items.begin(), items.begin() + (items.size() - MAX_ITEMS));
}

void Game::updateXPOrbs(float dt) {
    for (auto it = xpOrbs.begin(); it != xpOrbs.end();) {
        it->update(dt, player.position, player.radius);
        if (it->isExpired()) it = xpOrbs.erase(it);
        else ++it;
    }
    const size_t MAX_ORBS = 280;
    if (xpOrbs.size() > MAX_ORBS)
        xpOrbs.erase(xpOrbs.begin(), xpOrbs.begin() + (xpOrbs.size() - MAX_ORBS));
}

void Game::updateProjectiles(float dt) {
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        it->update(dt);

        bool hitWall = tilemap.isWallAtPosition(it->position, it->radius) || isOutsideOpenWorldBounds(it->position);

        if (it->isGrenade && (hitWall || it->isOutOfRange())) {
            // Grenade explosion
            if (!it->exploded) {
                it->exploded = true;
                for (auto& enemy : enemies) {
                    if (Vector2Distance(it->position, enemy.position) <= it->explodeRadius) {
                        enemy.takeDamage(it->damage);
                        particles.spawnHit(enemy.position, Color{255,120,0,255}, 12);
                        audio.playHit();
                    }
                }
                particles.spawnExplosion(it->position, Color{255,140,0,255}, 35);
                audio.playExplosion();
                triggerShake(6.0f, 0.25f);
                hitStopTimer = 0.07f;
                camPunch = std::max(camPunch, 0.07f);
            }
            it = projectiles.erase(it);
        } else if (hitWall || !it->active || it->isOutOfRange()) {
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::updateEnemyProjectiles(float dt) {
    for (auto it = enemyProjectiles.begin(); it != enemyProjectiles.end();) {
        it->update(dt);

        bool hitWall = tilemap.isWallAtPosition(it->position, it->radius) || isOutsideOpenWorldBounds(it->position);

        if (it->hitsPlayer(player.position, player.radius)) {
            if (!player.isShielded()) {
                player.takeDamage(it->damage);
                audio.playPlayerHurt();
                noteHurtDir(it->position);
                hitFlashTimer = 0.30f;
                particles.spawnHit(player.position, RED, 6);
                triggerShake(6.0f, 0.22f);
                audio.playHit();
            }
            it->active = false;
        }

        if (!it->active || hitWall || it->isOutOfRange()) {
            it = enemyProjectiles.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::noteHurtDir(Vector2 src) {
    Vector2 from = Vector2Subtract(player.position, src);
    hurtDir = Vector2LengthSqr(from) > 1.0f ? Vector2Normalize(from) : Vector2{0, 1};
    hurtDirTimer = 1.15f;
}

void Game::autoSave() {
    std::vector<std::string> buildingLines;
    buildingSystem.save(buildingLines);
    SaveManager::save(player, quests, currentZone, 0,
                      sessionTime / 60.0f, player.totalKills,
                      totalDeaths, totalBossesKilled, totalPortalsClosed,
                      (int)difficulty, totalKills, &buildingLines, newGamePlus);
}

bool Game::canAutoSave() const {
    if (paused) return false;
    if (player.health <= 0.0f) return false;                    // jogador morto
    if (dialogOpen) return false;                               // dialogo aberto
    if (owFadeTimer > 0.0f) return false;                       // transicao de fase
    if (showLevelUpScreen || showEvolutionScreen) return false; // escolha pendente
    if (finalBossAlive) return false;                           // combate de chefe final
    return true;
}

void Game::showStoryBanner(const std::string& title, const std::string& sub, float dur) {
    storyBannerText  = title;
    storyBannerSub   = sub;
    storyBannerTimer = dur;
}

void Game::triggerPlayerSpeech(const std::string& text, float dur) {
    playerSpeechText  = text;
    playerSpeechTimer = dur;
}

void Game::drawPlayerSpeech() const {
    auto drawBubble = [&](Vector2 worldPos, float height3D, float offset3D, const std::string& text, float timer) {
        if (timer <= 0.0f) return;
        float alpha = timer < 0.8f ? timer / 0.8f : 1.0f;

        const char* txt = text.c_str();
        int fontSize = 14;
        int tw = MeasureText(txt, fontSize);
        int bw = tw + 24, bh = 28;

        Vector2 sp = GetWorldToScreenEx({ worldPos.x, height3D, worldPos.y }, camera3D, screenWidth, screenHeight);
        int by = (int)(sp.y) - (int)offset3D;
        int bx = (int)(sp.x) - bw / 2;

        // Clamp to screen bounds
        if (bx < 5) bx = 5;
        if (bx + bw > screenWidth - 5) bx = screenWidth - bw - 5;
        if (by < 5) by = 5;

        // Bubble Panel
        DrawRectangleRounded({(float)bx,(float)by,(float)bw,(float)bh}, 0.35f, 6,
                             ColorAlpha(BLACK, 0.88f * alpha));
        DrawRectangleLinesEx({(float)bx,(float)by,(float)bw,(float)bh}, 1.5f,
                             ColorAlpha({0,200,255,255}, 0.9f * alpha));

        // Tail pointing down to character
        int tx = (int)sp.x;
        if (tx < bx + 8) tx = bx + 8;
        if (tx > bx + bw - 8) tx = bx + bw - 8;
        int tailY = by + bh;
        DrawTriangle({(float)(tx-7),(float)tailY},{(float)(tx+7),(float)tailY},
                     {(float)tx,(float)(tailY+12)}, ColorAlpha(BLACK, 0.88f * alpha));
        DrawLineEx({(float)(tx-6),(float)tailY},{(float)tx,(float)(tailY+11)},
                   1.5f, ColorAlpha({0,200,255,255}, 0.8f * alpha));
        DrawLineEx({(float)(tx+6),(float)tailY},{(float)tx,(float)(tailY+11)},
                   1.5f, ColorAlpha({0,200,255,255}, 0.8f * alpha));

        DrawText(txt, bx + 12, by + 7, fontSize, ColorAlpha({0,240,255,255}, alpha));
    };

    // 1. Local Player
    drawBubble(player.position, 60.0f, 40.0f, playerSpeechText, playerSpeechTimer);

    // 2. Remote Players (activeChats)
    if (netActive) {
        for (const auto& pair : activeChats) {
            uint32_t peerId = pair.first;
            const ChatBubble& cb = pair.second;
            
            // Find the peer's position
            for (const auto& p : net.peers()) {
                if (p.id == peerId) {
                    drawBubble({ p.x, p.y }, 42.0f, 32.0f, cb.text, cb.timer);
                    break;
                }
            }
        }
    }
}

void Game::drawStoryBanner() const {
    if (storyBannerTimer <= 0.0f) return;
    float alpha = std::min(storyBannerTimer, 1.0f);
    // Fade out last 1s
    if (storyBannerTimer < 1.0f) alpha = storyBannerTimer;

    // Painel COMPACTO, do tamanho do texto. A versao antiga pintava uma barra
    // preta de LARGURA TOTAL da tela: tapava o jogo inteiro numa faixa so pra
    // mostrar duas linhas de texto.
    int tw = MeasureText(storyBannerText.c_str(), 20);
    int sw = MeasureText(storyBannerSub.c_str(), 12);
    int panW = (tw > sw ? tw : sw) + 40;
    int panH = storyBannerSub.empty() ? 34 : 52;
    int panX = screenWidth / 2 - panW / 2;
    int panY = screenHeight / 6;

    DrawRectangle(panX, panY, panW, panH, ColorAlpha(BLACK, 0.84f * alpha));
    DrawRectangleLinesEx({ (float)panX, (float)panY, (float)panW, (float)panH },
                         1.5f, ColorAlpha(Color{0,200,255,255}, 0.65f * alpha));

    DrawText(storyBannerText.c_str(), screenWidth/2 - tw/2 + 2, panY + 9, 20,
             ColorAlpha(BLACK, alpha * 0.8f));
    DrawText(storyBannerText.c_str(), screenWidth/2 - tw/2, panY + 7, 20,
             ColorAlpha({0,220,255,255}, alpha));
    if (!storyBannerSub.empty())
        DrawText(storyBannerSub.c_str(), screenWidth/2 - sw/2 + 1, panY + 34, 12,
                 ColorAlpha(BLACK, alpha * 0.8f));
    if (!storyBannerSub.empty())
        DrawText(storyBannerSub.c_str(), screenWidth/2 - sw/2, panY + 33, 12,
                 ColorAlpha({235,245,255,255}, alpha));
}

// ─── Render ──────────────────────────────────────────────────────────────────

// ─── 2.5D isométrico (Incremento 1: câmera + tilemap 3D + raycast) ───────────

void Game::updateCamera3D() {
    float z = cameraZoom * (1.0f + camPunch);
    camera3D.position   = { camera.target.x, cameraHeight * z, camera.target.y + cameraDistY * z };
    camera3D.target     = { camera.target.x, 0.0f, camera.target.y };
    camera3D.up         = { 0.0f, 1.0f, 0.0f };
    camera3D.fovy       = 30.0f;
    camera3D.projection = CAMERA_PERSPECTIVE;
}

// Lança um raio do mouse (virtualizado p/ a render-texture 1280x720) e intersecta
// o plano do chão Y=0, devolvendo a posição em coordenadas de mundo 2D (x, z).
Vector2 Game::mouseGround3D() const {
    Ray ray = GetScreenToWorldRayEx(virtualizeMousePos(GetMousePosition()),
                                    camera3D, screenWidth, screenHeight);
    // Blindagem: garante o raio apontando para BAIXO e limita o alcance, para que
    // cliques perto do horizonte NÃO gerem alvo no infinito (player disparava pra
    // longe e o mundo infinito colapsava — causa do "travou").
    float dy = ray.direction.y;
    if (dy > -0.08f) dy = -0.08f;
    float t = -ray.position.y / dy;
    if (t < 0.0f)    t = 0.0f;
    if (t > 5000.0f) t = 5000.0f;
    return { ray.position.x + ray.direction.x * t,
             ray.position.z + ray.direction.z * t };
}







void Game::render() {
    // Pipeline único: 2.5D isométrico (mundo 3D voxelizado). O menu principal é
    // desenhado por drawMainMenu() diretamente no loop de run() e nunca passa por aqui.
    renderWorld3D();
}

