cmake_minimum_required(VERSION 3.14)
project(Kronosfall LANGUAGES CXX)

# Compat: raylib 5.0 declares an old cmake_minimum_required that CMake >= 3.31
# rejects. Keeps FetchContent working on newer toolchains.
if(NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)
FetchContent_MakeAvailable(raylib)

add_executable(darknet
    src/main.cpp
    src/Game.cpp
    src/Game_Menus.cpp
    src/Game_Shaders.cpp
    src/Game_Network.cpp
    src/Game_PremiumStore.cpp
    src/Game_Resources.cpp
    src/Game_Evolution.cpp
    src/Game_QuestsNPC.cpp
    src/Game_Phases.cpp
    src/Game_Spawn.cpp
    src/Game_WorldGen.cpp
    src/Game_WorldRender.cpp
    src/Game_Gameplay.cpp
    src/Enemy_Render.cpp
    src/Game_HUD.cpp
    src/Game_Bot.cpp
    src/Game_SkillTree.cpp
    src/SkillTree.cpp
    src/SpriteExtrude.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Item.cpp
    src/Projectile.cpp
    src/EnemyProjectile.cpp
    src/Skill.cpp
    src/Tilemap.cpp
    src/NPC.cpp
    src/Quest.cpp
    src/XPOrb.cpp
    src/Particle.cpp
    src/AudioManager.cpp
    src/SaveManager.cpp
    src/Background.cpp
    src/BotController.cpp
    src/EnemyDirector.cpp
    src/Companion.cpp
    src/BuildingSystem.cpp
    src/ShopSystem.cpp
    src/DarkWorld.cpp
    src/AnomalyPortal.cpp
    src/LightSystem.cpp
    src/CraftingSystem.cpp
    src/InfernoZone.cpp
    src/Achievement.cpp
    src/TutorialSystem.cpp
    src/SpriteGen.cpp
    src/NetClient.cpp
    src/HttpClient.cpp
    src/StoreClient.cpp
)

target_include_directories(darknet PRIVATE src third_party)
target_link_libraries(darknet PRIVATE raylib)

if (WIN32)
    target_link_libraries(darknet PRIVATE winmm ws2_32 winhttp shell32)
endif()

# ── Unit tests (doctest, vendored in third_party/doctest) ─────────────────────
# Compiles ONLY the .cpp files of the tested classes (pure rules) — not the
# whole game. Links raylib because Enemy/CraftingSystem call raylib functions
# in their renders (never executed in tests; no window is opened).
enable_testing()
add_executable(darknet_tests
    tests/test_main.cpp
    tests/test_cases.cpp
    src/CraftingSystem.cpp
    src/Enemy.cpp
    src/SpriteGen.cpp     # SpriteBank, referenced by Enemy::render (not executed)
    src/SaveManager.cpp   # save/load roundtrip (V5 + legacy V4) — pure file rules
    src/Player.cpp        # used by SaveManager::load (equipItem/applyClass/loadSavedProgress)
    src/Item.cpp          # Item::createRandom on inventory load
    src/Quest.cpp         # quest construction/update in tests
    src/Skill.cpp         # Player::update/useSkill reference Skill
    src/SkillTree.cpp     # Player::refreshSkillVectors/getEffectiveDamage use perks
    src/Projectile.cpp    # projectile tests
    src/Tilemap.cpp       # tilemap tests
    src/BotController.cpp # telegraph-dodge geometry tests
)
target_include_directories(darknet_tests PRIVATE src third_party)
target_link_libraries(darknet_tests PRIVATE raylib)
if (WIN32)
    target_link_libraries(darknet_tests PRIVATE winmm)
endif()
add_test(NAME darknet_tests COMMAND darknet_tests)

# Copy resources to the executable directory (Release/Debug) so the game runs
# when launched directly, not only from the project directory.
# content/ = DATA content (phases etc). It ships with the exe so it can be edited
# without recompiling — the basis of the "engine that evolves without touching C++".
if (EXISTS "${CMAKE_SOURCE_DIR}/content")
    add_custom_command(TARGET darknet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/content" "$<TARGET_FILE_DIR:darknet>/content"
        COMMENT "Copying content/ to the executable directory")
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/resources")
    add_custom_command(TARGET darknet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:darknet>/resources"
        COMMENT "Copying resources/ to the executable directory")
endif()
