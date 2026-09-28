// Game_Network.cpp — multiplayer LAN (NetClient), jogadores remotos e party/alianca.
// Extraido de Game.cpp. Mesma classe Game.
#include "Game.h"
#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <algorithm>
#include <string>
#include <cstdlib>

// ─── Multiplayer LAN (NetClient) ─────────────────────────────────────────────

// URL do WebSocket: por padrão direto no game-server local (dev). Em produção
// aponte KRONOSFALL_WS_URL para o gateway (ex.: ws://kronosfall.seudominio.com/ws).
static std::string wsUrl() {
    const char* u = getenv("KRONOSFALL_WS_URL");
    return (u && *u) ? u : "ws://127.0.0.1:9000/ws";
}

void Game::startNetwork() {
    if (netActive || netPending_) return;
    netId  = (uint32_t)GetRandomValue(1, 2000000000);
    // O servidor de agora exige JWT válido no handshake WS. O token vem do
    // login da loja (StoreClient). Se ainda não logou, enfileira o início —
    // pollStartNetwork() dispara assim que o login concluir (via updateParty).
    if (store.loggedIn()) {
        const std::string u = wsUrl();
        netActive = net.init(Player::className(player.charClass), netId,
                             u.c_str(), store.token().c_str());
    } else {
        startStore();          // garante o login em andamento
        netPending_ = true;
    }
}

void Game::pollStartNetwork() {
    if (netActive || !netPending_ || !store.loggedIn()) return;
    netPending_ = false;
    const std::string u = wsUrl();
    netActive = net.init(Player::className(player.charClass), netId,
                         u.c_str(), store.token().c_str());
}

void Game::renderRemotePlayers() const {
    if (!net.enabled) return;
    Vector2 cam = camera.target;
    static const Color cols[6] = {
        {60,120,220,255},{220,80,140,255},{150,160,175,255},
        {120,80,220,255},{180,120,255,255},{200,130,60,255}
    };
    for (const auto& p : net.peers()) {
        if (std::fabs(p.x - cam.x) > 1100 || std::fabs(p.y - cam.y) > 700) continue;
        Color c = cols[(p.charClass >= 0 && p.charClass < 6) ? p.charClass : 0];
        DrawEllipse((int)p.x, (int)(p.y + 18), 14.0f, 5.0f, ColorAlpha(BLACK, 0.4f));
        DrawRectangle((int)p.x - 9, (int)p.y - 14, 18, 28, c);
        DrawCircle((int)p.x, (int)(p.y - 20), 9.0f, c);
        DrawCircleLines((int)p.x, (int)(p.y - 20), 9.0f, ColorAlpha(WHITE, 0.4f));
        int w = MeasureText(p.name, 11);
        DrawRectangle((int)p.x - w/2 - 3, (int)p.y - 42, w + 6, 14, ColorAlpha(BLACK, 0.6f));
        DrawText(p.name, (int)p.x - w/2, (int)p.y - 40, 11, ColorAlpha(WHITE, 0.95f));
        DrawCircle((int)p.x + 11, (int)(p.y - 30), 3.0f, Color{0,255,80,255}); // online
    }
}

// ─── Grupo / Aliança (party multiplayer) ─────────────────────────────────────

void Game::updateParty() {
    pollStartNetwork();   // inicia o WS assim que o login (JWT) concluir
    if (chatActive) return;
    if (IsKeyPressed(KEY_O)) { partyPanel = !partyPanel; partyInput.clear(); }
    if (!partyPanel) return;

    // Digita um codigo numerico de grupo
    int ch = GetCharPressed();
    while (ch > 0) {
        if (ch >= '0' && ch <= '9' && partyInput.size() < 6) partyInput.push_back((char)ch);
        ch = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !partyInput.empty()) partyInput.pop_back();

    // ENTER entra no grupo digitado; L = sala publica; C = cria grupo (codigo do netId)
    if (IsKeyPressed(KEY_ENTER) && !partyInput.empty()) {
        net.joinParty("party_" + partyInput);
        triggerPlayerSpeech("Entrou no grupo " + partyInput, 2.5f);
        partyPanel = false;
    }
    if (IsKeyPressed(KEY_L)) {
        net.joinParty("lobby");
        triggerPlayerSpeech("Entrou na sala publica.", 2.0f);
        partyPanel = false;
    }
    if (IsKeyPressed(KEY_C)) {
        std::string code = std::to_string(1000 + (int)(netId % 9000));
        partyInput = code;
        net.joinParty("party_" + code);
        triggerPlayerSpeech("Grupo criado! Codigo: " + code, 4.0f);
        partyPanel = false;
    }
}

void Game::drawPartyPanel() const {
    // Indicador permanente: sala atual + nº de aliados online
    Color C_cyan = {0,235,255,255};
    std::string room = net.currentRoom();
    bool isParty = (room.rfind("party_", 0) == 0);
    int allies = (int)net.peers().size();
    const char* roomLbl = isParty ? room.c_str() + 6 : "PUBLICO";
    DrawText(TextFormat("GRUPO: %s  |  Aliados online: %d  [O]",
             isParty ? roomLbl : "PUBLICO", allies),
             14, 30, 11, ColorAlpha(C_cyan, 0.6f));

    if (!partyPanel) return;

    int pw = 460, ph = 250;
    int px = screenWidth/2 - pw/2, py = screenHeight/2 - ph/2;
    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.55f));
    DrawPanel(px, py, pw, ph, C_cyan, 0.95f);
    DrawText("GRUPO / ALIANCA", px + 18, py + 14, 22, C_cyan);
    DrawText("Jogue junto com amigos na mesma sala em tempo real.",
             px + 18, py + 44, 12, ColorAlpha(WHITE, 0.6f));

    int y = py + 78;
    DrawText(TextFormat("Sala atual: %s", isParty ? roomLbl : "PUBLICO (lobby)"),
             px + 18, y, 14, C_cyan); y += 26;
    DrawText(TextFormat("Aliados conectados: %d", allies), px + 18, y, 14,
             Color{0,230,120,255}); y += 30;

    DrawText("Digite um codigo e ENTER para entrar num grupo:", px + 18, y, 12,
             ColorAlpha(WHITE, 0.7f)); y += 20;
    DrawRectangle(px + 18, y, 200, 26, ColorAlpha(BLACK, 0.5f));
    DrawRectangleLinesEx({(float)(px+18),(float)y,200,26}, 1.5f, C_cyan);
    DrawText(partyInput.empty() ? "_" : partyInput.c_str(), px + 26, y + 5, 18, WHITE);
    y += 38;

    DrawText("[C] Criar grupo privado   [L] Sala publica   [O] Fechar",
             px + 18, y, 12, ColorAlpha(C_cyan, 0.8f));
    // Lista de aliados na sala
    y += 26;
    int shown = 0;
    for (const auto& p : net.peers()) {
        if (shown >= 4) break;
        DrawText(TextFormat("- %s", p.name[0] ? p.name : "Operador"),
                 px + 26, y, 12, ColorAlpha(WHITE, 0.75f));
        y += 16; shown++;
    }
}

