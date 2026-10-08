#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

const int SCREEN_WIDTH = 300;
const int SCREEN_HEIGHT = 600;
const int BLOCK_SIZE = 30;
const int GRID_WIDTH = 10;
const int GRID_HEIGHT = 20;

// Tetromino shapes (4x4 grids)
const int SHAPES[7][4][4] = {
    // I
    {{0,0,0,0},
     {1,1,1,1},
     {0,0,0,0},
     {0,0,0,0}},
    // O
    {{0,0,0,0},
     {0,1,1,0},
     {0,1,1,0},
     {0,0,0,0}},
    // T
    {{0,0,0,0},
     {0,1,0,0},
     {1,1,1,0},
     {0,0,0,0}},
    // S
    {{0,0,0,0},
     {0,1,1,0},
     {1,1,0,0},
     {0,0,0,0}},
    // Z
    {{0,0,0,0},
     {1,1,0,0},
     {0,1,1,0},
     {0,0,0,0}},
    // L
    {{0,0,0,0},
     {0,0,1,0},
     {1,1,1,0},
     {0,0,0,0}},
    // J
    {{0,0,0,0},
     {1,0,0,0},
     {1,1,1,0},
     {0,0,0,0}}
};

struct Piece {
    int shape[4][4];
    int x, y;
    SDL_Color color;
};

class Tetris {
public:
    Tetris() : grid(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0)), 
               gameOver(false), score(0), fallTimer(0), fallInterval(500) {
        srand(time(nullptr));
        colors = {
            {0, 255, 255, 255}, // I - Cyan
            {255, 255, 0, 255}, // O - Yellow
            {128, 0, 128, 255}, // T - Purple
            {0, 255, 0, 255},   // S - Green
            {255, 0, 0, 255},   // Z - Red
            {255, 165, 0, 255}, // L - Orange
            {0, 0, 255, 255}    // J - Blue
        };
        spawnPiece();
    }

    void handleInput(SDL_Event& e) {
        if (gameOver) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
                reset();
            }
            return;
        }

        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_LEFT:
                    movePiece(-1, 0);
                    break;
                case SDLK_RIGHT:
                    movePiece(1, 0);
                    break;
                case SDLK_DOWN:
                    movePiece(0, 1);
                    break;
                case SDLK_UP:
                    rotatePiece();
                    break;
                case SDLK_SPACE:
                    hardDrop();
                    break;
            }
        }
    }

    void update(int deltaTime) {
        if (gameOver) return;

        fallTimer += deltaTime;
        if (fallTimer >= fallInterval) {
            fallTimer = 0;
            if (!movePiece(0, 1)) {
                lockPiece();
                clearLines();
                spawnPiece();
                if (checkCollision(currentPiece.x, currentPiece.y)) {
                    gameOver = true;
                }
            }
        }
    }

    void render(SDL_Renderer* renderer) {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw grid background
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                SDL_Rect rect = {x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE - 1, BLOCK_SIZE - 1};
                SDL_RenderDrawRect(renderer, &rect);
            }
        }

        // Draw locked pieces
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid[y][x] != 0) {
                    SDL_SetRenderDrawColor(renderer, colors[grid[y][x] - 1].r, 
                                         colors[grid[y][x] - 1].g, 
                                         colors[grid[y][x] - 1].b, 255);
                    SDL_Rect rect = {x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE - 1, BLOCK_SIZE - 1};
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        // Draw current piece
        if (!gameOver) {
            SDL_SetRenderDrawColor(renderer, currentPiece.color.r, 
                                 currentPiece.color.g, 
                                 currentPiece.color.b, 255);
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 4; x++) {
                    if (currentPiece.shape[y][x]) {
                        int px = (currentPiece.x + x) * BLOCK_SIZE;
                        int py = (currentPiece.y + y) * BLOCK_SIZE;
                        SDL_Rect rect = {px, py, BLOCK_SIZE - 1, BLOCK_SIZE - 1};
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }
        }

        // Draw game over text
        if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect gameOverRect = {50, 250, 200, 100};
            SDL_RenderFillRect(renderer, &gameOverRect);
            
            // Simple white border
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &gameOverRect);
        }
    }

private:
    std::vector<std::vector<int>> grid;
    std::vector<SDL_Color> colors;
    Piece currentPiece;
    bool gameOver;
    int score;
    int fallTimer;
    int fallInterval;

    void spawnPiece() {
        int shapeIndex = rand() % 7;
        memcpy(currentPiece.shape, SHAPES[shapeIndex], sizeof(currentPiece.shape));
        currentPiece.x = GRID_WIDTH / 2 - 2;
        currentPiece.y = 0;
        currentPiece.color = colors[shapeIndex];
    }

    bool checkCollision(int newX, int newY) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (currentPiece.shape[y][x]) {
                    int gridX = newX + x;
                    int gridY = newY + y;
                    
                    if (gridX < 0 || gridX >= GRID_WIDTH || gridY >= GRID_HEIGHT) {
                        return true;
                    }
                    if (gridY >= 0 && grid[gridY][gridX] != 0) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool movePiece(int dx, int dy) {
        if (!checkCollision(currentPiece.x + dx, currentPiece.y + dy)) {
            currentPiece.x += dx;
            currentPiece.y += dy;
            return true;
        }
        return false;
    }

    void rotatePiece() {
        int rotated[4][4];
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                rotated[x][3 - y] = currentPiece.shape[y][x];
            }
        }

        // Check if rotation is valid
        int oldShape[4][4];
        memcpy(oldShape, currentPiece.shape, sizeof(oldShape));
        memcpy(currentPiece.shape, rotated, sizeof(rotated));

        if (checkCollision(currentPiece.x, currentPiece.y)) {
            // Try to wall kick
            if (!checkCollision(currentPiece.x - 1, currentPiece.y)) {
                currentPiece.x--;
            } else if (!checkCollision(currentPiece.x + 1, currentPiece.y)) {
                currentPiece.x++;
            } else {
                // Rotation not possible, revert
                memcpy(currentPiece.shape, oldShape, sizeof(oldShape));
            }
        }
    }

    void hardDrop() {
        while (movePiece(0, 1)) {}
    }

    void lockPiece() {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (currentPiece.shape[y][x]) {
                    int gridX = currentPiece.x + x;
                    int gridY = currentPiece.y + y;
                    if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH) {
                        // Find color index
                        for (size_t i = 0; i < colors.size(); i++) {
                            if (colors[i].r == currentPiece.color.r &&
                                colors[i].g == currentPiece.color.g &&
                                colors[i].b == currentPiece.color.b) {
                                grid[gridY][gridX] = i + 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    void clearLines() {
        for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
            bool full = true;
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid[y][x] == 0) {
                    full = false;
                    break;
                }
            }
            if (full) {
                // Remove line and shift everything down
                for (int y2 = y; y2 > 0; y2--) {
                    grid[y2] = grid[y2 - 1];
                }
                grid[0] = std::vector<int>(GRID_WIDTH, 0);
                y++; // Check same row again
                score += 100;
                
                // Increase speed
                if (score % 500 == 0 && fallInterval > 100) {
                    fallInterval -= 50;
                }
            }
        }
    }

    void reset() {
        for (auto& row : grid) {
            std::fill(row.begin(), row.end(), 0);
        }
        gameOver = false;
        score = 0;
        fallInterval = 500;
        fallTimer = 0;
        spawnPiece();
    }
};

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Tetris",
                                         SDL_WINDOWPOS_CENTERED,
                                         SDL_WINDOWPOS_CENTERED,
                                         SCREEN_WIDTH, SCREEN_HEIGHT,
                                         SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Tetris tetris;
    bool running = true;
    SDL_Event e;
    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            tetris.handleInput(e);
        }

        Uint32 currentTime = SDL_GetTicks();
        int deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        tetris.update(deltaTime);
        tetris.render(renderer);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}