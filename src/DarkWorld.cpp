#include "DarkWorld.h"
#include "SpriteGen.h"
#include <cmath>
#include <raylib.h>
#include <raymath.h>

// ─────────────────────────────────────────────────────────────────────────────
// Individual structure renderers (world-space coordinates)
// ─────────────────────────────────────────────────────────────────────────────

void DarkWorld::drawHouse(Vector2 pos, float scale, bool lights, float time) {
    float s = scale;
    int   x = (int)pos.x, y = (int)pos.y;

    Color wall    = {95,  88,  80, 255};
    Color wallDk  = {65,  60,  54, 255};
    Color roof    = {40,  35,  30, 255};
    Color roofDk  = {28,  24,  20, 255};
    Color door    = {55,  38,  22, 255};
    Color winDk   = {20,  18,  25, 255};
    Color chimney = {70,  65,  58, 255};

    // Main walls
    DrawRectangle(x - (int)(40*s), y - (int)(50*s), (int)(80*s), (int)(50*s), wall);
    // Side shading
    DrawRectangle(x + (int)(28*s), y - (int)(50*s), (int)(12*s), (int)(50*s), wallDk);

    // Cracks
    DrawLineEx({pos.x - 20*s, pos.y - 40*s}, {pos.x - 18*s, pos.y - 28*s}, 0.8f,
               ColorAlpha({40,38,34,255}, 0.6f));
    DrawLineEx({pos.x + 10*s, pos.y - 20*s}, {pos.x + 13*s, pos.y - 10*s}, 0.8f,
               ColorAlpha({40,38,34,255}, 0.5f));

    // Roof — triangle
    DrawTriangle({pos.x - 46*s, pos.y - 50*s},
                 {pos.x + 46*s, pos.y - 50*s},
                 {pos.x,        pos.y - 82*s},
                 roof);
    DrawTriangle({pos.x - 40*s, pos.y - 50*s},
                 {pos.x,        pos.y - 76*s},
                 {pos.x + 40*s, pos.y - 50*s},
                 roofDk);

    // Chimney
    DrawRectangle(x + (int)(12*s), y - (int)(82*s), (int)(10*s), (int)(22*s), chimney);
    DrawRectangle(x + (int)(10*s), y - (int)(84*s), (int)(14*s), (int)(5*s), chimney);

    // Windows (2)
    float winBright = lights ? (0.5f + 0.3f * std::sin(time*1.5f)) : 0.0f;
    DrawRectangle(x - (int)(30*s), y - (int)(44*s), (int)(16*s), (int)(14*s), winDk);
    DrawRectangle(x + (int)(14*s), y - (int)(44*s), (int)(16*s), (int)(14*s), winDk);
    if (lights) {
        Color winLight = ColorAlpha({255, 200, 100, 255}, winBright * 0.7f);
        DrawRectangle(x - (int)(28*s), y - (int)(42*s), (int)(12*s), (int)(10*s), winLight);
        DrawRectangle(x + (int)(16*s), y - (int)(42*s), (int)(12*s), (int)(10*s), winLight);
    }

    // Door
    DrawRectangle(x - (int)(8*s), y - (int)(30*s), (int)(16*s), (int)(30*s), door);
    DrawCircleV({pos.x + 6*s, pos.y - 16*s}, 2.0f*s, {180, 150, 80, 255});

    // Porch steps
    DrawRectangle(x - (int)(12*s), y, (int)(24*s), (int)(4*s), wallDk);
    DrawRectangle(x - (int)(10*s), y - (int)(3*s), (int)(20*s), (int)(3*s), wall);
}

void DarkWorld::drawBarn(Vector2 pos, float scale) {
    float s = scale;
    int   x = (int)pos.x, y = (int)pos.y;

    Color barnRed = {100, 35, 20, 255};
    Color barnDk  = {70,  22, 12, 255};
    Color wood    = {80,  55, 35, 255};
    Color roofB   = {45,  35, 25, 255};

    // Main body
    DrawRectangle(x - (int)(55*s), y - (int)(70*s), (int)(110*s), (int)(70*s), barnRed);
    // Wood planks (horizontal lines)
    for (int r = 0; r < 7; ++r)
        DrawLineEx({pos.x - 55*s, pos.y - 60*s + r*10*s},
                   {pos.x + 55*s, pos.y - 60*s + r*10*s},
                   1.0f, ColorAlpha(barnDk, 0.5f));

    // Roof peak (two triangles forming gable)
    DrawTriangle({pos.x - 60*s, pos.y - 70*s},
                 {pos.x + 60*s, pos.y - 70*s},
                 {pos.x,        pos.y - 105*s},
                 roofB);
    DrawRectangle(x - (int)(62*s), y - (int)(73*s), (int)(124*s), (int)(5*s), barnDk);

    // Round window in gable
    DrawCircleV({pos.x, pos.y - 82*s}, 8*s, barnDk);
    DrawCircleLines((int)pos.x, (int)(pos.y - 82*s), 8*s, wood);

    // Large barn doors
    DrawRectangle(x - (int)(30*s), y - (int)(50*s), (int)(28*s), (int)(50*s), barnDk);
    DrawRectangle(x + (int)(2*s),  y - (int)(50*s), (int)(28*s), (int)(50*s), barnDk);
    DrawLineEx({pos.x - 16*s, pos.y - 50*s}, {pos.x - 16*s, pos.y}, 1.0f, wood);
    DrawLineEx({pos.x + 16*s, pos.y - 50*s}, {pos.x + 16*s, pos.y}, 1.0f, wood);
    // Door diagonal braces
    DrawLineEx({pos.x - 28*s, pos.y - 48*s}, {pos.x - 4*s, pos.y - 4*s}, 1.0f, ColorAlpha(wood, 0.5f));
    DrawLineEx({pos.x + 4*s, pos.y - 48*s}, {pos.x + 28*s, pos.y - 4*s}, 1.0f, ColorAlpha(wood, 0.5f));
}

void DarkWorld::drawDeadTree(Vector2 pos, float scale) {
    float s = scale;
    Color bark = {45, 35, 25, 255};
    Color barkL = {60, 48, 35, 255};

    // Trunk
    DrawRectangle((int)(pos.x - 4*s), (int)(pos.y - 55*s), (int)(8*s), (int)(55*s), bark);
    DrawRectangle((int)(pos.x - 2*s), (int)(pos.y - 52*s), (int)(3*s), (int)(50*s), barkL);

    // Roots
    DrawLineEx({pos.x, pos.y}, {pos.x - 12*s, pos.y + 10*s}, 2.0f*s, bark);
    DrawLineEx({pos.x, pos.y}, {pos.x + 10*s, pos.y + 8*s},  2.0f*s, bark);

    // Main branches
    DrawLineEx({pos.x, pos.y - 40*s}, {pos.x - 28*s, pos.y - 65*s}, 2.5f*s, bark);
    DrawLineEx({pos.x, pos.y - 35*s}, {pos.x + 25*s, pos.y - 58*s}, 2.0f*s, bark);
    DrawLineEx({pos.x, pos.y - 50*s}, {pos.x - 14*s, pos.y - 72*s}, 1.8f*s, bark);

    // Sub-branches
    DrawLineEx({pos.x - 20*s, pos.y - 58*s}, {pos.x - 32*s, pos.y - 75*s}, 1.2f*s, bark);
    DrawLineEx({pos.x - 20*s, pos.y - 58*s}, {pos.x - 10*s, pos.y - 76*s}, 1.2f*s, bark);
    DrawLineEx({pos.x + 18*s, pos.y - 52*s}, {pos.x + 30*s, pos.y - 68*s}, 1.2f*s, bark);
    DrawLineEx({pos.x + 18*s, pos.y - 52*s}, {pos.x + 22*s, pos.y - 72*s}, 1.0f*s, bark);
    DrawLineEx({pos.x - 10*s, pos.y - 66*s}, {pos.x - 18*s, pos.y - 80*s}, 1.0f*s, bark);
    DrawLineEx({pos.x - 10*s, pos.y - 66*s}, {pos.x - 2*s,  pos.y - 80*s}, 1.0f*s, bark);
}

void DarkWorld::drawGravestone(Vector2 pos, float rot) {
    float s = 1.0f;
    Color stone  = {120, 120, 115, 255};
    Color stoneD = {85,  85,  80,  255};
    Color moss   = {40,  70,  35,  180};
    Color text_c = {70,  70,  65,  255};

    // Shadow
    DrawEllipse((int)pos.x, (int)(pos.y + 4), 12.0f, 3.0f, ColorAlpha(BLACK, 0.25f));

    // Base slab
    DrawRectangle((int)(pos.x - 10), (int)(pos.y - 3), 20, 5, stoneD);
    // Main body
    DrawRectangle((int)(pos.x - 7), (int)(pos.y - 25), 14, 22, stone);
    // Arched top — approximate with circle
    DrawCircleV({pos.x, pos.y - 25}, 7.0f, stone);

    // Moss patches
    DrawCircleV({pos.x - 4, pos.y - 18}, 3.5f, ColorAlpha(moss, 0.6f));
    DrawCircleV({pos.x + 3, pos.y - 10}, 2.5f, ColorAlpha(moss, 0.5f));

    // "RIP" text
    DrawText("RIP", (int)(pos.x - 9), (int)(pos.y - 22), 7, text_c);
    // Cross groove
    DrawLineEx({pos.x, pos.y - 14}, {pos.x, pos.y - 8}, 1.0f, stoneD);
    DrawLineEx({pos.x - 4, pos.y - 12}, {pos.x + 4, pos.y - 12}, 1.0f, stoneD);

    (void)rot;
}

void DarkWorld::drawFence(Vector2 pos, float rot, float scale) {
    float s = scale;
    Color wood = {60, 45, 30, 255};
    Color woodL = {75, 58, 40, 255};

    // 4 fence posts + 2 rails
    float flen = 60.0f * s;
    float dx = std::cos(rot) * flen;
    float dy = std::sin(rot) * flen;

    // Rails
    DrawLineEx({pos.x - dx*0.5f, pos.y - dy*0.5f - 10*s},
               {pos.x + dx*0.5f, pos.y + dy*0.5f - 10*s}, 2.0f*s, wood);
    DrawLineEx({pos.x - dx*0.5f, pos.y - dy*0.5f - 3*s},
               {pos.x + dx*0.5f, pos.y + dy*0.5f - 3*s}, 2.0f*s, wood);

    // Posts
    for (int p = 0; p < 4; ++p) {
        float t = (float)p / 3.0f - 0.5f;
        float px2 = pos.x + dx * t;
        float py2 = pos.y + dy * t;
        DrawRectangle((int)(px2 - 2*s), (int)(py2 - 18*s), (int)(4*s), (int)(18*s), woodL);
        DrawRectangle((int)(px2 - 3*s), (int)(py2 - 20*s), (int)(6*s), (int)(3*s), wood);
    }
}

void DarkWorld::drawStreetLight(Vector2 pos, bool on, float time) {
    Color pole  = {70, 70, 75, 255};
    Color head  = {55, 55, 60, 255};

    // Pole
    DrawRectangle((int)(pos.x - 2), (int)(pos.y - 60), 4, 60, pole);
    // Arm
    DrawRectangle((int)(pos.x),     (int)(pos.y - 60), 18, 3, pole);
    // Lamp housing
    DrawRectangle((int)(pos.x + 12), (int)(pos.y - 65), 10, 8, head);

    if (on) {
        float pulse = 0.8f + 0.2f * std::sin(time * 1.2f);
        Color lampC = ColorAlpha({255, 210, 130, 255}, pulse);
        DrawCircleV({pos.x + 17, pos.y - 61}, 5.0f, lampC);
        DrawCircleV({pos.x + 17, pos.y - 61}, 18.0f, ColorAlpha({255, 210, 100, 255}, 0.06f * pulse));
        DrawCircleV({pos.x + 17, pos.y - 61}, 30.0f, ColorAlpha({255, 200, 80,  255}, 0.02f * pulse));
    } else {
        DrawCircleV({pos.x + 17, pos.y - 61}, 4.0f, {35, 33, 28, 255});
        DrawLineEx({pos.x + 14, pos.y - 65}, {pos.x + 22, pos.y - 58}, 0.8f,
                   ColorAlpha({60,55,45,255}, 0.5f));
    }
}

void DarkWorld::drawAbandonedCar(Vector2 pos, float rot) {
    Color bodyC  = {75,  68,  60, 255};
    Color rust   = {110, 55,  20, 200};
    Color glass  = {25,  28,  35, 255};
    Color tire   = {22,  20,  18, 255};

    // Shadow
    DrawEllipse((int)pos.x, (int)(pos.y + 8), 28.0f, 6.0f, ColorAlpha(BLACK, 0.3f));

    // Body
    DrawRectangle((int)(pos.x - 30), (int)(pos.y - 10), 60, 18, bodyC);
    // Hood triangle
    DrawTriangle({pos.x + 30, pos.y - 10}, {pos.x + 30, pos.y + 8}, {pos.x + 50, pos.y}, bodyC);
    // Roof
    DrawRectangle((int)(pos.x - 20), (int)(pos.y - 22), 38, 12, {65, 58, 50, 255});

    // Windows (broken)
    DrawRectangle((int)(pos.x - 18), (int)(pos.y - 20), 14, 10, glass);
    DrawRectangle((int)(pos.x +  6), (int)(pos.y - 20), 10, 10, glass);
    // Crack lines in glass
    DrawLineEx({pos.x - 14, pos.y - 20}, {pos.x - 10, pos.y - 11}, 0.7f,
               ColorAlpha({150,155,160,255}, 0.6f));
    DrawLineEx({pos.x + 8, pos.y - 19}, {pos.x + 14, pos.y - 12}, 0.7f,
               ColorAlpha({150,155,160,255}, 0.5f));

    // Rust patches
    DrawCircleV({pos.x - 22, pos.y - 2}, 6.0f, ColorAlpha(rust, 0.5f));
    DrawCircleV({pos.x + 15, pos.y + 4}, 4.5f, ColorAlpha(rust, 0.4f));

    // Tires
    DrawEllipse((int)(pos.x - 20), (int)(pos.y + 10), 8.0f, 5.0f, tire);
    DrawEllipse((int)(pos.x + 18), (int)(pos.y + 10), 8.0f, 5.0f, tire);
    DrawEllipse((int)(pos.x - 20), (int)(pos.y + 10), 4.0f, 2.5f, {35,33,30,255});
    DrawEllipse((int)(pos.x + 18), (int)(pos.y + 10), 4.0f, 2.5f, {35,33,30,255});

    (void)rot;
}

void DarkWorld::drawCityBuilding(Vector2 pos, float w, float h, float time) {
    Color wall    = {50,  50,  58, 255};
    Color wallDk  = {35,  35,  42, 255};
    Color winOn   = {200, 185, 100, 200};
    Color winOff  = {22,  22,  28, 255};
    Color winBrk  = {18,  18,  22, 255};

    int x = (int)(pos.x - w/2);
    int y = (int)(pos.y - h);

    // Facade
    DrawRectangle(x, y, (int)w, (int)h, wall);
    DrawRectangle(x + (int)(w*0.8f), y, (int)(w*0.2f), (int)h, wallDk);
    // Parapet
    DrawRectangle(x - 3, y - 6, (int)w + 6, 6, wallDk);
    // Antenna
    DrawLineEx({pos.x + w*0.1f, (float)y - 6}, {pos.x + w*0.1f, (float)y - 22}, 1.5f, {80,80,85,255});

    // Fire escape zigzag
    for (int f = 0; f < 3; ++f) {
        float fy = pos.y - (float)(f+1) * h / 4.0f;
        DrawLineEx({pos.x - w/2, fy}, {pos.x - w/2 + 10, fy - 12}, 1.0f, {70,70,75,255});
        DrawLineEx({pos.x - w/2 + 10, fy - 12}, {pos.x - w/2, fy - 24}, 1.0f, {70,70,75,255});
    }

    // Windows grid
    int cols = (int)(w / 14);
    int rows = (int)(h / 16);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int wx = x + 4 + c * 14;
            int wy = y + 6 + r * 16;
            // Pseudo-random on/off using position
            int hash = (r*13 + c*7 + (int)(pos.x/10) + (int)(pos.y/10)) % 17;
            if (hash < 6) {
                float pulse = 0.85f + 0.15f * std::sin(time * 0.8f + hash);
                DrawRectangle(wx, wy, 9, 10, ColorAlpha(winOn, pulse));
            } else if (hash < 8) {
                DrawRectangle(wx, wy, 9, 10, winBrk);
                DrawLineEx({(float)wx, (float)wy}, {(float)(wx+9), (float)(wy+10)}, 0.6f,
                           ColorAlpha({80,85,90,255}, 0.4f));
            } else {
                DrawRectangle(wx, wy, 9, 10, winOff);
            }
        }
    }

    // Graffiti at base
    DrawText("KRONOSFALL", x + 4, (int)(pos.y - 14), 7, ColorAlpha({200, 0, 255, 255}, 0.5f));
}

void DarkWorld::drawSilo(Vector2 pos, float scale) {
    float s = scale;
    Color metalC = {85, 80, 70, 255};
    Color metalD = {60, 56, 48, 255};
    Color rust   = {100, 50, 20, 180};

    // Cylinder (approximate with rectangle + dome)
    DrawRectangle((int)(pos.x - 14*s), (int)(pos.y - 70*s), (int)(28*s), (int)(70*s), metalC);
    DrawEllipse((int)pos.x, (int)(pos.y - 70*s), 14.0f*s, 7.0f*s, metalD);
    DrawEllipse((int)pos.x, (int)(pos.y - 70*s), 10.0f*s, 5.0f*s, ColorAlpha(metalC, 0.6f));

    // Horizontal bands
    for (int b = 0; b < 5; ++b)
        DrawLineEx({pos.x - 14*s, pos.y - 15*s - b*13*s},
                   {pos.x + 14*s, pos.y - 15*s - b*13*s}, 1.5f, metalD);

    // Rust patches
    DrawCircleV({pos.x - 6*s, pos.y - 30*s}, 5.0f*s, ColorAlpha(rust, 0.4f));
    DrawCircleV({pos.x + 5*s, pos.y - 50*s}, 3.5f*s, ColorAlpha(rust, 0.35f));

    // Ladder on side
    DrawLineEx({pos.x + 12*s, pos.y}, {pos.x + 12*s, pos.y - 68*s}, 1.0f, metalD);
    for (int r = 0; r < 8; ++r)
        DrawLineEx({pos.x + 10*s, pos.y - r*9*s - 4*s},
                   {pos.x + 14*s, pos.y - r*9*s - 4*s}, 0.8f, metalD);
}

void DarkWorld::drawStatue(Vector2 pos, float scale) {
    float s = scale;
    Color stone = {80, 78, 85, 255};
    Color stoneD = {55, 52, 58, 255};

    // Pedestal
    DrawRectangle((int)(pos.x - 12*s), (int)(pos.y - 10*s), (int)(24*s), (int)(10*s), stoneD);
    DrawRectangle((int)(pos.x - 10*s), (int)(pos.y - 40*s), (int)(20*s), (int)(30*s), stone);

    // Body figure (silhouette)
    DrawRectangle((int)(pos.x - 7*s), (int)(pos.y - 65*s), (int)(14*s), (int)(25*s), stone);
    // Arms outstretched
    DrawRectangle((int)(pos.x - 24*s), (int)(pos.y - 60*s), (int)(18*s), (int)(5*s), stone);
    DrawRectangle((int)(pos.x + 6*s),  (int)(pos.y - 60*s), (int)(18*s), (int)(5*s), stone);
    // Head
    DrawCircleV({pos.x, pos.y - 72*s}, 7.0f*s, stone);
    // Crack
    DrawLineEx({pos.x - 3*s, pos.y - 56*s}, {pos.x + 2*s, pos.y - 44*s}, 0.8f,
               ColorAlpha(stoneD, 0.7f));
}

void DarkWorld::drawCatacombArch(Vector2 pos) {
    Color stone  = {55, 50, 52, 255};
    Color stoneD = {38, 34, 36, 255};
    Color bone   = {190, 180, 155, 255};

    // Left pillar
    DrawRectangle((int)(pos.x - 32), (int)(pos.y - 70), 12, 70, stone);
    // Right pillar
    DrawRectangle((int)(pos.x + 20), (int)(pos.y - 70), 12, 70, stone);
    // Arch keystone
    DrawRectangle((int)(pos.x - 32), (int)(pos.y - 72), 64, 10, stoneD);
    DrawCircleV({pos.x, pos.y - 72}, 20.0f, stoneD);
    DrawCircleV({pos.x, pos.y - 72}, 16.0f, {18, 14, 16, 255});

    // Skull decoration at keystone
    DrawCircleV({pos.x, pos.y - 76}, 5.0f, bone);
    DrawCircleV({pos.x - 2, pos.y - 77}, 1.5f, stoneD);
    DrawCircleV({pos.x + 2, pos.y - 77}, 1.5f, stoneD);

    // Bone pile at base
    for (int b = 0; b < 4; ++b) {
        float bx = pos.x - 28 + b * 14;
        DrawEllipse((int)bx, (int)(pos.y + 3), 6.0f, 2.5f, ColorAlpha(bone, 0.7f));
        DrawCircleV({bx, pos.y + 1}, 3.0f, ColorAlpha(bone, 0.5f));
    }

    // Torch on each pillar
    DrawRectangle((int)(pos.x - 25), (int)(pos.y - 52), 4, 10, {60, 42, 28, 255});
    DrawCircleV({pos.x - 23, pos.y - 55}, 5.0f, ColorAlpha({255, 160, 40, 255}, 0.6f));
    DrawRectangle((int)(pos.x + 21), (int)(pos.y - 52), 4, 10, {60, 42, 28, 255});
    DrawCircleV({pos.x + 23, pos.y - 55}, 5.0f, ColorAlpha({255, 160, 40, 255}, 0.6f));
}

// ─────────────────────────────────────────────────────────────────────────────
// Zone generation
// ─────────────────────────────────────────────────────────────────────────────

static float pseudoRandF(unsigned int& seed, float lo, float hi) {
    seed = seed * 1664525u + 1013904223u;
    float t = (float)(seed & 0xFFFF) / 65535.0f;
    return lo + t * (hi - lo);
}
static int pseudoRandI(unsigned int& seed, int lo, int hi) {
    seed = seed * 1664525u + 1013904223u;
    return lo + (int)(seed % (unsigned int)(hi - lo + 1));
}

void DarkWorld::generate(int zoneId, unsigned int seed) {
    scenery.clear();

    // zoneId mapping: 4=Cemetery, 5=CursedFarm, 6=GhostCity, 7=DarkForest, 8=Catacombs, 9=AbandonedManor
    unsigned int s = seed;

    switch (zoneId) {
        case 4: { // Cemetery
            name     = "Cemiterio Abandonado";
            subtitle = "Os mortos nao descansam aqui...";
            fogColor   = {180, 200, 180, 255};
            fogDensity = 0.75f;
            skyTop    = {8, 10, 12, 255};
            skyBottom = {18, 22, 18, 255};

            int stones = pseudoRandI(s, 15, 25);
            for (int i = 0; i < stones; ++i) {
                SceneryObject o;
                o.type     = 3;
                o.position = {pseudoRandF(s, -400, 400), pseudoRandF(s, -400, 400)};
                o.rotation = pseudoRandF(s, -0.18f, 0.18f);
                o.scale    = pseudoRandF(s, 0.8f, 1.2f);
                o.tint     = WHITE;
                scenery.push_back(o);
            }
            int trees = pseudoRandI(s, 8, 12);
            for (int i = 0; i < trees; ++i) {
                SceneryObject o;
                o.type     = 2;
                o.position = {pseudoRandF(s, -500, 500), pseudoRandF(s, -500, 500)};
                o.rotation = 0; o.scale = pseudoRandF(s, 0.7f, 1.3f); o.tint = WHITE;
                scenery.push_back(o);
            }
            for (int i = 0; i < 4; ++i) {
                SceneryObject o;
                o.type = 4; o.rotation = pseudoRandF(s, 0, 3.14f);
                o.position = {pseudoRandF(s, -350, 350), pseudoRandF(s, -350, 350)};
                o.scale = 1.0f; o.tint = WHITE;
                scenery.push_back(o);
            }
            // One house
            scenery.push_back({pseudoRandF(s, 280, 380), pseudoRandF(s, 280, 380), 0, 0, 0.9f, WHITE});
            break;
        }
        case 5: { // CursedFarm
            name     = "Fazenda Maldita";
            subtitle = "A terra esta podre. As colheitas, corrompidas.";
            fogColor   = {160, 150, 120, 255};
            fogDensity = 0.50f;
            skyTop    = {12, 10, 8, 255};
            skyBottom = {28, 22, 14, 255};

            // 2-3 barns
            int barns = pseudoRandI(s, 2, 3);
            for (int i = 0; i < barns; ++i) {
                SceneryObject o;
                o.type = 1; o.position = {pseudoRandF(s, -300, 300), pseudoRandF(s, -300, 300)};
                o.rotation = 0; o.scale = pseudoRandF(s, 0.85f, 1.1f); o.tint = WHITE;
                scenery.push_back(o);
            }
            // 1 house
            scenery.push_back({{pseudoRandF(s,-100,100), pseudoRandF(s,-100,100)}, 0, 0.0f, 1.0f, WHITE});
            // 2 silos
            for (int i = 0; i < 2; ++i) {
                SceneryObject o;
                o.type = 8; o.position = {pseudoRandF(s,-400,400), pseudoRandF(s,-400,400)};
                o.rotation = 0; o.scale = pseudoRandF(s, 0.9f, 1.2f); o.tint = WHITE;
                scenery.push_back(o);
            }
            // Many fences
            int fences = pseudoRandI(s, 20, 30);
            for (int i = 0; i < fences; ++i) {
                SceneryObject o;
                o.type = 4; o.position = {pseudoRandF(s,-450,450), pseudoRandF(s,-450,450)};
                o.rotation = pseudoRandF(s, 0, 3.14f);
                o.scale = pseudoRandF(s, 0.8f, 1.2f); o.tint = WHITE;
                scenery.push_back(o);
            }
            // Dead trees on edges
            for (int i = 0; i < 8; ++i) {
                SceneryObject o;
                o.type = 2; o.position = {pseudoRandF(s,-500,500), pseudoRandF(s,-500,500)};
                o.rotation = 0; o.scale = pseudoRandF(s,0.7f,1.1f); o.tint = WHITE;
                scenery.push_back(o);
            }
            // Abandoned cars
            for (int i = 0; i < 4; ++i) {
                SceneryObject o;
                o.type = 6; o.position = {pseudoRandF(s,-380,380), pseudoRandF(s,-380,380)};
                o.rotation = pseudoRandF(s,0,3.14f); o.scale = 1.0f; o.tint = WHITE;
                scenery.push_back(o);
            }
            break;
        }
        case 6: { // GhostCity
            name     = "Cidade Fantasma";
            subtitle = "Ruas vazias. Mas nao desertas.";
            fogColor   = {160, 170, 190, 255};
            fogDensity = 0.60f;
            skyTop    = {5, 6, 14, 255};
            skyBottom = {14, 16, 28, 255};

            // Buildings
            int bldgs = pseudoRandI(s, 8, 14);
            for (int i = 0; i < bldgs; ++i) {
                SceneryObject o;
                o.type = 7;
                o.position = {pseudoRandF(s,-500,500), pseudoRandF(s,-500,500)};
                o.rotation = pseudoRandF(s, 40, 80);  // width stored in rotation
                o.scale    = pseudoRandF(s, 80, 200); // height stored in scale
                o.tint     = WHITE;
                scenery.push_back(o);
            }
            // Street lights
            int lights = pseudoRandI(s, 10, 20);
            for (int i = 0; i < lights; ++i) {
                SceneryObject o;
                o.type = 5;
                o.position = {pseudoRandF(s,-480,480), pseudoRandF(s,-480,480)};
                o.rotation = 0; o.scale = 1.0f;
                // tint.r encodes on/off (255=on, 0=off)
                o.tint = (pseudoRandI(s,0,1) == 1) ? WHITE : Color{0,0,0,255};
                scenery.push_back(o);
            }
            // Cars
            int cars = pseudoRandI(s, 6, 10);
            for (int i = 0; i < cars; ++i) {
                SceneryObject o;
                o.type = 6; o.position = {pseudoRandF(s,-460,460), pseudoRandF(s,-460,460)};
                o.rotation = pseudoRandF(s,0,3.14f); o.scale = 1.0f; o.tint = WHITE;
                scenery.push_back(o);
            }
            // Some houses
            for (int i = 0; i < 5; ++i) {
                SceneryObject o;
                o.type = 0; o.position = {pseudoRandF(s,-400,400), pseudoRandF(s,-400,400)};
                o.rotation = 0; o.scale = pseudoRandF(s,0.7f,0.9f); o.tint = WHITE;
                scenery.push_back(o);
            }
            break;
        }
        case 7: { // DarkForest
            name     = "Floresta Negra";
            subtitle = "A nevoa esconde o que mora entre as arvores.";
            fogColor   = {140, 160, 130, 255};
            fogDensity = 0.85f;
            skyTop    = {4, 7, 4, 255};
            skyBottom = {10, 16, 8, 255};

            int trees = pseudoRandI(s, 40, 60);
            for (int i = 0; i < trees; ++i) {
                SceneryObject o;
                o.type = 2; o.position = {pseudoRandF(s,-550,550), pseudoRandF(s,-550,550)};
                o.rotation = 0; o.scale = pseudoRandF(s,0.6f,1.5f); o.tint = WHITE;
                scenery.push_back(o);
            }
            // Ruined walls
            for (int i = 0; i < 3; ++i) {
                SceneryObject o;
                o.type = 4; o.position = {pseudoRandF(s,-350,350), pseudoRandF(s,-350,350)};
                o.rotation = pseudoRandF(s,0,3.14f); o.scale = 1.5f; o.tint = {80,75,70,255};
                scenery.push_back(o);
            }
            break;
        }
        case 8: { // Catacombs
            name     = "Catacumbas";
            subtitle = "Passagens de pedra. Cheiro de morte antiga.";
            fogColor   = {120, 100, 130, 255};
            fogDensity = 0.70f;
            skyTop    = {6, 4, 8, 255};
            skyBottom = {14, 10, 18, 255};

            // Arches in a grid pattern
            for (int i = 0; i < 12; ++i) {
                SceneryObject o;
                o.type = 9;
                o.position = {pseudoRandF(s, -400, 400), pseudoRandF(s, -400, 400)};
                o.rotation = 0; o.scale = 1.0f; o.tint = WHITE;
                scenery.push_back(o);
            }
            // Gravestones as bone piles
            for (int i = 0; i < 20; ++i) {
                SceneryObject o;
                o.type = 3; o.position = {pseudoRandF(s,-450,450), pseudoRandF(s,-450,450)};
                o.rotation = pseudoRandF(s,-0.3f,0.3f); o.scale = 0.7f; o.tint = WHITE;
                scenery.push_back(o);
            }
            break;
        }
        case 9: { // AbandonedManor
            name     = "Mansao Abandonada";
            subtitle = "O boss aguarda nas profundezas.";
            fogColor   = {140, 110, 150, 255};
            fogDensity = 0.65f;
            skyTop    = {8, 4, 12, 255};
            skyBottom = {20, 10, 28, 255};

            // Central manor (large house at origin)
            scenery.push_back({{0, -50}, 0, 0.0f, 2.0f, WHITE});
            // Trees around
            for (int i = 0; i < 5; ++i) {
                SceneryObject o;
                o.type = 2; o.position = {pseudoRandF(s,-350,350), pseudoRandF(s,-350,350)};
                o.rotation = 0; o.scale = pseudoRandF(s, 1.0f, 1.5f); o.tint = WHITE;
                scenery.push_back(o);
            }
            // Statues lining approach
            for (int i = 0; i < 4; ++i) {
                SceneryObject o;
                o.type = 10;
                o.position = {(float)(i < 2 ? -80 : 80), (float)(-80 + (i%2)*120)};
                o.rotation = 0; o.scale = 1.0f; o.tint = WHITE;
                scenery.push_back(o);
            }
            // Fences forming a perimeter
            for (int i = 0; i < 12; ++i) {
                SceneryObject o;
                o.type = 4; o.position = {pseudoRandF(s,-400,400), pseudoRandF(s,-400,400)};
                o.rotation = pseudoRandF(s, 0, 3.14f); o.scale = 1.2f; o.tint = WHITE;
                scenery.push_back(o);
            }
            break;
        }
        default:
            name = "Zona Desconhecida";
            subtitle = "";
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Sky + atmosphere rendering
// ─────────────────────────────────────────────────────────────────────────────

void DarkWorld::renderSky(float ambientTime, int screenW, int screenH) const {
    // Gradient sky (top to bottom)
    for (int y = 0; y < screenH; y += 4) {
        float t = (float)y / (float)screenH;
        Color c;
        c.r = (unsigned char)(skyTop.r + (skyBottom.r - skyTop.r) * t);
        c.g = (unsigned char)(skyTop.g + (skyBottom.g - skyTop.g) * t);
        c.b = (unsigned char)(skyTop.b + (skyBottom.b - skyTop.b) * t);
        c.a = 255;
        DrawRectangle(0, y, screenW, 4, c);
    }

    // Moon
    float moonX = (float)screenW * 0.82f;
    float moonY = (float)screenH * 0.12f;
    DrawCircleV({moonX, moonY}, 24.0f, {220, 215, 200, 255});
    DrawCircleV({moonX + 8, moonY - 5}, 20.0f, skyTop); // crescent cutout
    // Halo
    DrawCircleV({moonX, moonY}, 35.0f, ColorAlpha({200, 195, 180, 255}, 0.08f));
    DrawCircleV({moonX, moonY}, 50.0f, ColorAlpha({180, 175, 160, 255}, 0.04f));

    // Stars
    unsigned int starSeed = 0xDEADBEEF;
    for (int i = 0; i < 80; ++i) {
        starSeed = starSeed * 1664525u + 1013904223u;
        float sx = (float)(starSeed & 0xFFF) / 4095.0f * screenW;
        starSeed = starSeed * 1664525u + 1013904223u;
        float sy = (float)(starSeed & 0xFFF) / 4095.0f * (screenH * 0.5f);
        starSeed = starSeed * 1664525u + 1013904223u;
        float blink = 0.5f + 0.5f * std::sin(ambientTime * (1.0f + (starSeed & 0x7) * 0.3f) + i);
        DrawPixel((int)sx, (int)sy, ColorAlpha({220, 220, 230, 255}, blink * 0.8f));
    }

    // Moving dark clouds
    for (int c = 0; c < 5; ++c) {
        float cx = std::fmod(ambientTime * (8.0f + c * 3.0f) + c * 250.0f, (float)screenW + 200.0f) - 100.0f;
        float cy = 60.0f + c * 28.0f;
        float cw = 80.0f + c * 20.0f;
        DrawEllipse((int)cx, (int)cy, cw, 22.0f, ColorAlpha({18, 18, 22, 255}, 0.5f));
        DrawEllipse((int)(cx + 30), (int)(cy - 8), cw * 0.6f, 16.0f, ColorAlpha({20, 20, 25, 255}, 0.4f));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Scenery rendering (world-space, inside camera transform)
// ─────────────────────────────────────────────────────────────────────────────

// Hash deterministico por celula (para detalhes de chao reproduziveis)
static inline float dwHash(int x, int y, int salt) {
    unsigned int h = (unsigned)(x * 73856093) ^ (unsigned)(y * 19349663) ^ (unsigned)(salt * 83492791);
    h = (h ^ (h >> 13)) * 1274126177u;
    return (float)((h >> 8) & 0xFFFF) / 65535.0f;
}

// Camada de VIDA AMBIENTE — decalques de chao, vegetacao balancando, vaga-lumes,
// poças e passaros. Determinista por posicao (nao "nada" entre frames), animada por `time`.
static void renderAmbientLife(Vector2 cam, float time) {
    const float VW = 760.0f, VH = 480.0f;   // meia-area visivel
    const int   cell = 88;
    int gx0 = (int)((cam.x - VW) / cell) - 1, gx1 = (int)((cam.x + VW) / cell) + 1;
    int gy0 = (int)((cam.y - VH) / cell) - 1, gy1 = (int)((cam.y + VH) / cell) + 1;

    for (int gy = gy0; gy <= gy1; ++gy)
    for (int gx = gx0; gx <= gx1; ++gx) {
        float r  = dwHash(gx, gy, 1);
        float px = gx * cell + dwHash(gx, gy, 2) * cell;
        float py = gy * cell + dwHash(gx, gy, 3) * cell;

        if (r < 0.30f) {                         // tufo de grama balancando
            float sway = std::sin(time * 1.6f + px * 0.05f) * 1.8f;
            int   gr = 55 + (int)(dwHash(gx,gy,4) * 40);
            Color g = { (unsigned char)(28 + gr/3), (unsigned char)gr, 28, 200 };
            for (int b = 0; b < 3; ++b) {
                float bx = px + (b - 1) * 2.5f;
                DrawLineEx({bx, py}, {bx + sway, py - 6 - (b % 2)}, 1.4f, g);
            }
        } else if (r < 0.40f) {                  // pedrinhas
            DrawCircleV({px, py}, 1.6f, Color{92,90,86,180});
            DrawCircleV({px + 3, py + 1}, 1.1f, Color{70,68,64,150});
        } else if (r < 0.47f) {                  // rachadura no chao
            Color cr = {18,18,20,110};
            DrawLineEx({px, py}, {px + 7, py + 4}, 1.0f, cr);
            DrawLineEx({px + 7, py + 4}, {px + 12, py + 1}, 1.0f, cr);
        } else if (r < 0.53f) {                  // poça refletindo o ceu
            DrawEllipse((int)px, (int)py, 8.0f, 3.2f, Color{34,46,62,110});
            float sh = 0.10f + 0.06f * std::sin(time * 1.2f + px * 0.1f);
            DrawEllipse((int)px, (int)py, 4.0f, 1.5f, ColorAlpha(Color{130,160,190,255}, sh));
        } else if (r < 0.57f) {                  // folhas secas / detritos
            Color lf = {90,70,40,150};
            DrawCircleV({px, py}, 1.4f, lf);
            DrawCircleV({px + 4, py - 2}, 1.1f, ColorAlpha(lf, 0.7f));
        }

        // Vaga-lume / partícula de poeira flutuando no ar (esparso)
        if (dwHash(gx, gy, 7) < 0.09f) {
            float fx = px + std::sin(time * 0.8f + gx) * 16.0f;
            float fy = py - 26 + std::cos(time * 0.7f + gy) * 12.0f;
            float a  = 0.35f + 0.4f * std::sin(time * 3.0f + gx * 1.3f + gy);
            if (a > 0.02f) {
                DrawCircleV({fx, fy}, 3.0f, ColorAlpha(Color{170,235,130,255}, a * 0.22f));
                DrawCircleV({fx, fy}, 1.5f, ColorAlpha(Color{210,255,170,255}, a));
            }
        }
    }

    // Passaros cruzando lentamente o "ceu" (acima da area visivel)
    for (int b = 0; b < 3; ++b) {
        float speed = 38.0f + b * 14.0f;
        float bx = std::fmod(time * speed + b * 640.0f, 2400.0f) - 1200.0f + cam.x;
        float by = cam.y - 360.0f + b * 46.0f + std::sin(time * 0.5f + b) * 18.0f;
        if (bx < cam.x - VW - 40 || bx > cam.x + VW + 40) continue;
        float flap = std::sin(time * 9.0f + b) * 4.0f;
        Color bird = {28,28,36,200};
        DrawLineEx({bx, by}, {bx - 6, by - flap}, 1.6f, bird);
        DrawLineEx({bx, by}, {bx + 6, by - flap}, 1.6f, bird);
    }
}

void DarkWorld::renderScenery(Vector2 cameraCenter, float time) const {
    SpriteBank& sb = SpriteBank::get();

    for (const auto& obj : scenery) {
        // Cull objects far from the camera center (1400px margin covers most screens)
        float dx = obj.position.x - cameraCenter.x;
        float dy = obj.position.y - cameraCenter.y;
        if (dx < -1400 || dx > 1400 || dy < -1400 || dy > 1400) continue;

        bool lights = (obj.tint.r > 128); // tint.r encodes lights-on flag

        // ── Sprites pixel-art (preferencial) ──────────────────────────────────
        if (sb.ready && obj.type >= 0 && obj.type < SpriteBank::NUM_SCENERY) {
            int variant = ((int)(obj.position.x * 0.13f + obj.position.y * 0.07f))
                          % SpriteBank::SCENERY_VARIANTS;
            if (variant < 0) variant += SpriteBank::SCENERY_VARIANTS;
            Texture2D tx = sb.scenery[obj.type][variant];

            // Fator de mundo: deixa os sprites num tamanho legivel e usa obj.scale
            float K = 1.7f * (obj.scale > 0.01f ? obj.scale : 1.0f);
            float w = tx.width  * K;
            float h = tx.height * K;

            // ÁRVORE (type 2): leve sway senoidal no topo (vento) — inclina o sprite
            float skew = 0.0f;
            if (obj.type == 2)
                skew = std::sin(time * 1.3f + obj.position.x * 0.04f) * (w * 0.05f);

            // base (pes) do objeto em obj.position; desenha para cima
            Rectangle dst = { obj.position.x - w * 0.5f + skew, obj.position.y - h, w, h };

            // Sombra elíptica no chao (mais densa = mais "plantado")
            DrawEllipse((int)obj.position.x, (int)obj.position.y,
                        w * 0.44f, h * 0.085f, ColorAlpha(BLACK, 0.40f));
            DrawTexturePro(tx, {0,0,(float)tx.width,(float)tx.height},
                           dst, {0,0}, 0.0f, WHITE);

            // Glow quente de janelas/luz com FLICKER (casa, predio, poste)
            if (lights && (obj.type == 0 || obj.type == 7 || obj.type == 5)) {
                float fl = std::sin(time * 7.3f + obj.position.x) * 0.5f
                         + std::sin(time * 2.1f + obj.position.y) * 0.5f;
                float pulse = 0.55f + 0.30f * fl;
                if (pulse < 0.2f) pulse = 0.2f;
                Vector2 c = { obj.position.x, obj.position.y - h * 0.55f };
                DrawCircleV(c, w * 0.34f, ColorAlpha(Color{255,200,110,255}, 0.10f * pulse));
                DrawCircleV(c, w * 0.18f, ColorAlpha(Color{255,215,140,255}, 0.16f * pulse));
                // Poste: cone de luz no chao
                if (obj.type == 5)
                    DrawEllipse((int)obj.position.x, (int)obj.position.y,
                                w * 0.5f, h * 0.10f, ColorAlpha(Color{255,210,130,255}, 0.07f * pulse));
            }

            // CASA (type 0): fumaca subindo da chamine (canto superior direito)
            if (obj.type == 0) {
                float ox = obj.position.x + w * 0.26f;
                float oy = obj.position.y - h * 0.92f;
                for (int s = 0; s < 4; ++s) {
                    float t2 = std::fmod(time * 0.35f + s * 0.25f + obj.position.x * 0.01f, 1.0f);
                    float sy = oy - t2 * 34.0f;
                    float sx = ox + std::sin(t2 * 6.0f + s) * 6.0f;
                    float a  = (1.0f - t2) * 0.20f;
                    float sr = 2.5f + t2 * 5.0f;
                    DrawCircleV({sx, sy}, sr, ColorAlpha(Color{120,120,128,255}, a));
                }
            }
            continue;
        }

        // ── Fallback geometrico (se sprites nao prontos) ──────────────────────
        switch (obj.type) {
            case 0:  drawHouse(obj.position, obj.scale, lights, time);   break;
            case 1:  drawBarn(obj.position, obj.scale);                  break;
            case 2:  drawDeadTree(obj.position, obj.scale);              break;
            case 3:  drawGravestone(obj.position, obj.rotation);         break;
            case 4:  drawFence(obj.position, obj.rotation, obj.scale);   break;
            case 5:  drawStreetLight(obj.position, lights, time);        break;
            case 6:  drawAbandonedCar(obj.position, obj.rotation);       break;
            case 7:  drawCityBuilding(obj.position, obj.rotation, obj.scale, time); break;
            case 8:  drawSilo(obj.position, obj.scale);                  break;
            case 9:  drawCatacombArch(obj.position);                     break;
            case 10: drawStatue(obj.position, obj.scale);                break;
        }
    }

    // ── Camada de vida ambiente (chao + ar) por cima do cenario base ──────────
    renderAmbientLife(cameraCenter, time);
}

// ─────────────────────────────────────────────────────────────────────────────
// Fog overlay (screen-space, outside camera transform)
// ─────────────────────────────────────────────────────────────────────────────

void DarkWorld::renderFog(float ambientTime, int screenW, int screenH) const {
    if (fogDensity <= 0.0f) return;

    // Base fog tint
    DrawRectangle(0, 0, screenW, screenH,
                  ColorAlpha(fogColor, fogDensity * 0.08f));

    // Rolling fog banks (large animated ellipses)
    for (int i = 0; i < 8; ++i) {
        float t   = ambientTime * (0.4f + i * 0.15f) + i * 1.23f;
        float fx  = (float)screenW  * (0.1f + 0.8f * (0.5f + 0.5f * std::sin(t * 0.7f + i)));
        float fy  = (float)screenH  * (0.3f + 0.5f * (0.5f + 0.5f * std::sin(t * 0.5f + i * 2.1f)));
        float fw  = 180.0f + 60.0f * std::sin(t * 0.3f + i);
        float fh  = 50.0f  + 20.0f * std::cos(t * 0.4f + i * 0.8f);
        DrawEllipse((int)fx, (int)fy, fw, fh,
                    ColorAlpha(fogColor, fogDensity * (0.06f + 0.03f * std::sin(t))));
    }

    // Ground-level fog strip
    DrawRectangle(0, screenH * 2/3, screenW, screenH / 3,
                  ColorAlpha(fogColor, fogDensity * 0.05f));
}

// ─────────────────────────────────────────────────────────────────────────────
// DarkWorldSystem
// ─────────────────────────────────────────────────────────────────────────────

void DarkWorldSystem::load(int zoneId, unsigned int seed) {
    currentZone.generate(zoneId, seed);
    active = true;
    ambientTime = 0.0f;
}

void DarkWorldSystem::applyWorldOffset(Vector2 offset) {
    for (auto& obj : currentZone.scenery) {
        obj.position.x += offset.x;
        obj.position.y += offset.y;
    }
}

void DarkWorldSystem::update(float dt) {
    if (!active) return;
    ambientTime += dt;
}

void DarkWorldSystem::renderBackground(int screenW, int screenH) const {
    if (!active) return;
    currentZone.renderSky(ambientTime, screenW, screenH);
}

void DarkWorldSystem::renderScenery(Vector2 cameraTarget, float zoom) const {
    if (!active) return;
    // Pass negative camera target so renderScenery culls relative to camera center
    currentZone.renderScenery(cameraTarget, ambientTime);
}

void DarkWorldSystem::renderFog(int screenW, int screenH) const {
    if (!active) return;
    currentZone.renderFog(ambientTime, screenW, screenH);
}

/* === INTEGRACAO COM GAME.H / GAME.CPP ===

Em Game.h, adicione:
    #include "DarkWorld.h"
    DarkWorldSystem darkWorld;

Em Game.cpp, onde muda de zona (transitionToZone):
    // Mapeamento ZoneID -> inteiro
    if (dest == ZoneID::Cemetery)       darkWorld.load(4, GetRandomValue(1,99999));
    if (dest == ZoneID::CursedFarm)     darkWorld.load(5, GetRandomValue(1,99999));
    if (dest == ZoneID::GhostCity)      darkWorld.load(6, GetRandomValue(1,99999));
    if (dest == ZoneID::DarkForest)     darkWorld.load(7, GetRandomValue(1,99999));
    if (dest == ZoneID::Catacombs)      darkWorld.load(8, GetRandomValue(1,99999));
    if (dest == ZoneID::AbandonedManor) darkWorld.load(9, GetRandomValue(1,99999));
    // Para zonas normais: darkWorld.active = false;

Em Game.cpp, update():
    darkWorld.update(dt);

Em Game.cpp, render() ANTES de BeginMode2D:
    if (darkWorld.active) darkWorld.renderBackground(screenWidth, screenHeight);

Em Game.cpp, render() DENTRO de BeginMode2D (antes dos inimigos):
    if (darkWorld.active) darkWorld.renderScenery(camera.target, camera.zoom);

Em Game.cpp, render() DEPOIS de EndMode2D:
    if (darkWorld.active) darkWorld.renderFog(screenWidth, screenHeight);

Tabelas de spawn por zona sombria (em spawnEnemy()):
    Cemetery:      Ghost(30) GhostElite(10) Zombie(35) ZombieHorde(15) ShadowWraith(10)
    CursedFarm:    Zombie(40) ZombieRager(20) BansheeHowler(15) ZombieHorde(25)
    GhostCity:     Ghost(25) GhostElite(15) ShadowWraith(20) BansheeHowler(15) Zombie(25)
    DarkForest:    Ghost(20) Zombie(30) ShadowWraith(25) ZombieRager(15) BansheeHowler(10)
    Catacombs:     Zombie(25) ZombieHorde(30) ShadowWraith(20) ZombieLord(boss) Zombie(25)
    AbandonedManor:Ghost(20) GhostElite(25) PoltergeistBoss(boss) ZombieHorde(30) ShadowWraith(25)

Portais sombrios: adicionar portais com ZoneID::Cemetery etc. em setupZoneNPCs() ou buildNPCs()
=== */
