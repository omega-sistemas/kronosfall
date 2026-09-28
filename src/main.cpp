#include "Game.h"
#include <raylib.h>
#include <string>
#include <cstdlib>
#include <ctime>

int main(int argc, char* argv[]) {
    printf("KRONOSFALL v1.0\n");
    printf("Uso: kronosfall.exe [--autobot|--autotest] [--test-seconds=N] [--seed=N] [--start-phase=N]\n");
    printf("  --autotest        bot de teste automatico\n");
    printf("  --test-seconds=N  encerra o teste em N segundos e grava o relatorio\n");
    printf("  --seed=N          mundo REPRODUTIVEL (mesmo seed = mesmo mapa)\n");
    printf("  --start-phase=N   start directly at phase N (0..N-1; useful for audit)\n");
    printf("  --headless        sem janela/GPU (CI): so a simulacao do bot\n\n");

    bool     autoTest = false;
    float    testSecs = 0.0f;   // 0 = usa o padrao (2h)
    unsigned seed     = 0;      // 0 = aleatorio
    bool     headless = false;
    int      startPhase = -1;
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--autotest" || arg == "--autobot") autoTest = true;
        else if (arg == "--headless") headless = true;
        // Sem --test-seconds o autoteste ia ate 7200s: so dava pra medir matando o
        // processo, e ai o relatorio nunca era escrito.
        else if (arg.rfind("--test-seconds=", 0) == 0)
            testSecs = (float)atof(arg.substr(15).c_str());
        // Sem seed fixa todo relatorio de bug virava anedota irreproduzivel.
        else if (arg.rfind("--seed=", 0) == 0)
            seed = (unsigned)strtoul(arg.substr(7).c_str(), nullptr, 10);
        // Jump to advanced phase (audit of phases 5-11).
        else if (arg.rfind("--start-phase=", 0) == 0)
            startPhase = atoi(arg.substr(14).c_str());
    }

    if (seed != 0) {
        SetRandomSeed(seed);
        srand(seed);
        printf("SEED FIXA: %u (mundo reprodutivel)\n", seed);
    } else {
        unsigned t = (unsigned)time(nullptr);
        SetRandomSeed(t);
        srand(t);
    }

    if (headless) printf("HEADLESS: sem janela/GPU (modo CI)\n");

    Game game(headless, startPhase);
    game.worldSeed          = seed;
    game.autoTestSeconds    = testSecs;
    game.runAutoTest(autoTest);

    // Exit code = veredito do portao de validacao (so no autoteste): qualquer
    // script ou agente sabe se a build ficou jogavel sem precisar abrir o jogo.
    return (autoTest && !game.autoTestPassed) ? 1 : 0;
}
