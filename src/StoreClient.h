#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// StoreClient — fachada para a loja premium do backend Node.js.
// Faz login (JWT), busca o catálogo (/store), saldo de gems (/me), compra de
// itens com gems (/store/buy-item, validado no servidor) e abre o Stripe Checkout
// para comprar gems (/store/buy-gems). Todas as chamadas de rede rodam em threads
// de fundo; a UI lê o estado por getters protegidos por mutex.
// ─────────────────────────────────────────────────────────────────────────────
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>

struct PremiumItem { std::string id, name, type; int gems = 0; };
struct GemPack     { std::string id; int gems = 0; double priceBRL = 0.0; };

class StoreClient {
public:
    std::string host      = "127.0.0.1";
    int         port      = 9000;
    // Prefixo de rota quando a API fica atrás do gateway nginx (/api/* -> /api).
    // Vazio = conexão direta no game-server (dev local). Configurado pelo jogo
    // via KRONOSFALL_API_URL (ex.: "https://kronosfall.seudominio.com" -> /api + TLS).
    std::string apiPrefix = "";
    bool        useTls    = false;   // HTTPS (WinHTTP) na API configurada

    ~StoreClient();   // espera threads de rede em voo (evita use-after-free no shutdown)

    // Token JWT obtido no login (usado também pelo NetClient no handshake WS).
    std::string token() const;

    // Todas assíncronas (disparam thread de fundo, retornam imediatamente).
    void loginAsync(const std::string& email, const std::string& password); // POST /auth/login -> token+id
    void fetchStoreAsync();                      // GET  /store      -> catálogo
    void refreshAsync();                         // GET  /me         -> gems/inventário
    void buyItemAsync(const std::string& itemId);// POST /store/buy-item (server valida)
    void buyGemsAsync(const std::string& packId);// POST /store/buy-gems -> abre navegador

    // ── Getters de UI (thread-safe) ───────────────────────────────────────────
    bool        loggedIn() const { return logged_.load(); }
    bool        busy()     const { return busy_.load(); }
    int         gems()     const;
    std::vector<PremiumItem> items() const;
    std::vector<GemPack>     packs() const;
    std::string lastMessage() const;
    bool        ownsItem(const std::string& id) const;

private:
    mutable std::mutex        mtx_;
    std::string               token_;
    std::string               playerId_;
    std::atomic<bool>         logged_{false};
    std::atomic<bool>         busy_{false};
    std::atomic<int>          activeThreads_{0};   // threads de rede em voo
    std::vector<std::thread>  threads_;            // threads ativas (join no destrutor)
    int                       gems_ = 0;
    std::vector<PremiumItem>  items_;
    std::vector<GemPack>      packs_;
    std::vector<std::string>  inventory_;
    std::string               msg_;

    void setMsg(const std::string& m);
    void startThread(std::thread&& t);             // helper: guarda e limpa threads finalizadas
};
