#include <cstdlib>
#include <ctime>
#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include <emscripten.h>

namespace Config
{
    constexpr int gridCountX = 30;  // 横方向のグリッド数
    constexpr int gridCountY = 24;  // 縦方向のグリッド数
    constexpr int gridSize = 20;    // グリッドサイズ
    constexpr int windowWidth = gridCountX * gridSize;  // ウィンドウ幅
    constexpr int windowHeight = gridCountY * gridSize; // ウィンドウ高さ
    constexpr int gameSpeed = 100;  // ミリ秒
}

struct Point
{
    int x, y;
};

struct GameState
{
    std::vector<Point> snake;
    int directionX;
    int directionY;
    Point food;
    bool running;
    bool gameOver;
};

GameState gameState;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

void spawnFood(GameState& state)
{
    state.food.x = rand() % Config::gridCountX;
    state.food.y = rand() % Config::gridCountY;
    std::cout << "Food spawned at: (" << state.food.x << ", " << state.food.y << ")" << std::endl;
}

void updateSnake(GameState& state)
{
    Point newHead = { state.snake[0].x + state.directionX, state.snake[0].y + state.directionY };
    state.snake.insert(state.snake.begin(), newHead);  // 頭を追加
    state.snake.pop_back();  // 尾を削除（スネークの長さを維持）
}

void renderSnake(GameState& state)
{
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // 緑
    for (const auto& segment : state.snake)
    {
        SDL_Rect rect = {
            segment.x * Config::gridSize,
            segment.y * Config::gridSize,
            Config::gridSize,
            Config::gridSize
        };
        SDL_RenderFillRect(renderer, &rect);
    }
}

void checkFoodCollision(GameState& state)
{
    if (state.snake[0].x == state.food.x && state.snake[0].y == state.food.y)
    {
        state.snake.push_back(state.snake.back());  // 尾を伸ばす
        spawnFood(state);  // 新しいエサを生成
    }
}

void renderFood(GameState& state)
{
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // 赤
    SDL_Rect rect = {
        state.food.x * Config::gridSize,
        state.food.y * Config::gridSize,
        Config::gridSize,
        Config::gridSize
    };
    SDL_RenderFillRect(renderer, &rect);
}

bool checkWallCollision(GameState& state)
{
    return (
        state.snake[0].x < 0
        || state.snake[0].x >= Config::gridCountX
        || state.snake[0].y < 0
        || state.snake[0].y >= Config::gridCountY
    );
}

bool checkSelfCollision(GameState& state)
{
    for (size_t i = 1; i < state.snake.size(); ++i)
    {
        if (state.snake[0].x == state.snake[i].x && state.snake[0].y == state.snake[i].y)
        {
            return true;
        }
    }
    return false;
}

void resetGame(GameState& state)
{
    state.snake = { {5, 5}, {4, 5}, {3, 5} };  // 初期のスネーク（3ブロック）
    state.directionX = 1;  // 初期の進行方向（右）
    state.directionY = 0;
    spawnFood(state);
    state.gameOver = false;
    state.running = true;
    std::cout << "Game restarted!" << std::endl;
}

void handleInput(GameState& state)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
        {
            state.running = false;
            state.gameOver = true;
        }
        if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_UP:
                    if (state.directionY == 0)
                    {
                        state.directionX = 0;
                        state.directionY = -1;
                    }
                    break;
                case SDLK_DOWN:
                    if (state.directionY == 0)
                    {
                        state.directionX = 0;
                        state.directionY = 1;
                    }
                    break;
                case SDLK_LEFT:
                    if (state.directionX == 0)
                    {
                        state.directionX = -1;
                        state.directionY = 0;
                    }
                    break;
                case SDLK_RIGHT:
                    if (state.directionX == 0)
                    {
                        state.directionX = 1;
                        state.directionY = 0;
                    }
                    break;
            }
        }
    }
}

void init(GameState& state)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL Initialization Failed: " << SDL_GetError() << std::endl;
        exit(1);
    }

    window = SDL_CreateWindow(
        "Snake Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        Config::windowWidth, Config::windowHeight,
        SDL_WINDOW_SHOWN
    );
    if (!window)
    {
        std::cerr << "Window Creation Failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Renderer Creation Failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    std::srand(static_cast<unsigned int>(std::time(nullptr)));  // シードを現在時刻で初期化
    resetGame(state);  // 初期状態に設定
}

void mainLoop()
{
    if (!gameState.running)
    {
        return;
    }

    if (!gameState.gameOver)
    {
        handleInput(gameState);      // 入力処理
        updateSnake(gameState);      // スネークを更新
        checkFoodCollision(gameState);  // エサの取得を判定

        if (checkWallCollision(gameState) || checkSelfCollision(gameState))
        {
            std::cout << "Game Over! Press R to restart." << std::endl;
            gameState.gameOver = true;
        }
    }
    else
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r)
            {
                resetGame(gameState);  // ゲームをリスタート
            }
            else if (event.type == SDL_QUIT)
            {
                gameState.running = false;
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);  // 背景色
    SDL_RenderClear(renderer);

    if (!gameState.gameOver)
    {
        renderSnake(gameState);  // スネークを描画
        renderFood(gameState);   // エサを描画
    }

    SDL_RenderPresent(renderer);  // 描画更新

    SDL_Delay(Config::gameSpeed);  // ゲーム速度を調整
}

int main()
{
    init(gameState);  // 初期化処理
    emscripten_set_main_loop(mainLoop, 0, 1);  // メインループ設定
    return 0;
}

// How to use:
// source ./emsdk/emsdk_env.sh
// emcc snake.cpp -o index.html --shell-file shell_minimal.html -s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1
// emrun --no_browser --port 8080 .