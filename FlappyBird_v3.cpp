// Mary-Anne Ibeh  Chloe Quijano Masuma Begum
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <list>
#include <fstream>
#include <iostream>
#include <ctime>
#include <string>
#include <SDL_mixer.h>
#include <vector>

using namespace std;

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600

bool birdAlive = true;
SDL_Texture* bgTexture = nullptr;
static int lastPowerUpX = -1000;

enum GameState
{
    START_SCREEN,
    IN_GAME,
    GAME_OVER
};

struct Pipe {
    int x;
    int gapY;
    static const int gapHeight = 200;
    static const int pipeWidth = 80;

};

struct Bird {
    int x, y;
    int width, height;
    float velocity;
    float gravity;
};

void initBird(Bird& bird) {
    bird.x = 50;
    bird.y = WINDOW_HEIGHT / 2;
    bird.width = 40;
    bird.height = 40;
    bird.velocity = 0;
    bird.gravity = 0.25f;
}

struct PowerUp {
    int x, y;
    int width, height;
    bool active;
    static const int duration = 00; // Duration of power-up effect in milliseconds
};

static void initPowerUp(PowerUp& powerUp, const std::list<Pipe>& pipes) {
    // Ensure there's a minimum spacing between power-ups
    const int minSpacing = 300; // Minimum spacing between power-ups

    // Check if we can spawn a new power-up based on the last spawned position
    if (!pipes.empty() && (lastPowerUpX < 0 || WINDOW_WIDTH - lastPowerUpX > minSpacing)) {
        std::vector<Pipe> pipesVector(pipes.begin(), pipes.end());
        int randomPipeIndex = rand() % pipesVector.size();
        const Pipe& selectedPipe = pipesVector[randomPipeIndex];

        powerUp.x = selectedPipe.x + Pipe::pipeWidth / 2;
        powerUp.y = selectedPipe.gapY;

        const int safeMargin = 20;
        int gapHalfHeight = Pipe::gapHeight / 2 - powerUp.height / 2 - safeMargin;
        powerUp.y += rand() % (gapHalfHeight * 2) - gapHalfHeight;

        powerUp.width = 40;
        powerUp.height = 40;
        powerUp.active = true;

        lastPowerUpX = powerUp.x; // Update the last power-up's position
    }
}

void trySpawnPowerUp(vector<PowerUp>& powerUps, const std::list<Pipe>& pipes) {
    if (pipes.size() < 2) return; // Ensure there are enough pipes to find a gap

    const int minSpacingFromPipe = 150; // Minimum horizontal spacing from the next pipe
    for (const auto& pipe : pipes) {
        // Simple check to ensure we're not spawning too close to a pipe
        if (WINDOW_WIDTH - pipe.x > minSpacingFromPipe && WINDOW_WIDTH - pipe.x < WINDOW_WIDTH / 2) {
            PowerUp newPowerUp;
            newPowerUp.x = pipe.x + Pipe::pipeWidth + (rand() % (minSpacingFromPipe - Pipe::pipeWidth)); // Random position within the gap
            newPowerUp.y = pipe.gapY - Pipe::gapHeight / 4 + (rand() % (Pipe::gapHeight / 2)); // Random position within the vertical gap
            newPowerUp.width = 40;
            newPowerUp.height = 40;
            newPowerUp.active = true;

            powerUps.push_back(newPowerUp);
            break; // Only spawn one power-up for now
        }
    }
}

// Function to render power-ups
void renderPowerUps(SDL_Renderer* renderer, SDL_Texture* featherTexture, const vector<PowerUp>& powerUps) {
    for (const auto& powerUp : powerUps) {
        if (powerUp.active) {
            SDL_Rect powerUpRect = { powerUp.x, powerUp.y, powerUp.width, powerUp.height };
            SDL_RenderCopy(renderer, featherTexture, NULL, &powerUpRect);
        }
    }
}

// Function to handle collision between bird and power-up
void handlePowerUpCollision(Bird& bird, vector<PowerUp>& powerUps, int& a, Mix_Chunk* scoreSound) {
    for (auto& powerUp : powerUps) {
        if (powerUp.active && bird.x < powerUp.x + powerUp.width && bird.x + bird.width > powerUp.x &&
            bird.y < powerUp.y + powerUp.height && bird.y + bird.height > powerUp.y) {
            // If bird collides with active power-up
            Mix_PlayChannel(-1, scoreSound, 0);
            a += 2;
            bird.gravity /= 1.2; // Reduce gravity to slow down descent
            powerUp.active = false; // Deactivate power-up
            SDL_Delay(PowerUp::duration); // Delay to indicate power-up duration
            bird.gravity *= 1.25; // Restore original gravity
        }
    }
}

// Function to update power-up position
void updatePowerUps(vector<PowerUp>& powerUps) {
    for (auto& powerUp : powerUps) {
        if (powerUp.active) {
            powerUp.x -= 2; // Move power-up towards the left
        }
    }
}

static void renderScoreAndLives(SDL_Renderer* renderer, TTF_Font* font, int score, int life) {
    SDL_Color textColor = { 255, 255, 255 };
    std::string scoreText = "Score: " + std::to_string(score) + " Lives: " + std::to_string(life);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

static void renderPipes(SDL_Renderer* renderer, SDL_Texture* upperPipeTexture, SDL_Texture* lowerPipeTexture, std::list<Pipe>& pipes) {
    for (Pipe& pipe : pipes) {
        // Upper pipe
        SDL_Rect topRect = { pipe.x, 0, Pipe::pipeWidth, pipe.gapY - Pipe::gapHeight / 2 };
        SDL_RenderCopy(renderer, upperPipeTexture, NULL, &topRect);

        // Lower pipe
        SDL_Rect bottomRect = { pipe.x, pipe.gapY + Pipe::gapHeight / 2, Pipe::pipeWidth, WINDOW_HEIGHT - (pipe.gapY + Pipe::gapHeight / 2) };
        SDL_RenderCopy(renderer, lowerPipeTexture, NULL, &bottomRect);
    }
}

void renderStartScreen(SDL_Renderer* renderer, TTF_Font* font)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

    // Draw a semi-transparent overlay
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // RGBA, 128 for 50% transparency
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect fillRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_RenderFillRect(renderer, &fillRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Reset to default

    // Render the start text
    SDL_Color textColor = { 255, 255, 255 }; // White color for the text
    const char* message = "Press SPACE to start";
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, message, textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_Rect textRect = { (WINDOW_WIDTH - textWidth) / 2, (WINDOW_HEIGHT - textHeight) / 2, textWidth, textHeight };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect); // Render the text

    // Clean up
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void renderGameOverScreen(SDL_Renderer* renderer, TTF_Font* font, int score, int highScore)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

    // Draw semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 192); // Semi-transparent black
    SDL_Rect backgroundRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_RenderFillRect(renderer, &backgroundRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Reset to default

    // Construct the game over texts
    std::string gameOverText = "Game Over!";
    std::string scoreText = "Score: " + std::to_string(score);
    std::string highScoreText = "High Score: " + std::to_string(highScore);
    std::string exitText = "Press SPACE to exit.";

    SDL_Color textColor = { 255, 255, 255 };

    int lineHeight = TTF_FontHeight(font) + 10; // Assuming a 10 pixel margin between lines
    int startY = (WINDOW_HEIGHT - lineHeight * 3) / 2; // Adjust starting Y position

    std::string texts[] = { gameOverText, scoreText, highScoreText, exitText };
    int numOfTexts = sizeof(texts) / sizeof(texts[0]);

    for (int i = 0; i < numOfTexts; i++)
    {
        SDL_Surface* surface = TTF_RenderText_Solid(font, texts[i].c_str(), textColor);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_Rect renderQuad = {
            (WINDOW_WIDTH - surface->w) / 2, // Centered horizontally
            startY + i * lineHeight, // Each line below the previous
            surface->w,
            surface->h
        };

        SDL_RenderCopy(renderer, texture, NULL, &renderQuad);

        // Cleanup
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}

static bool checkCollision(const Bird& bird, const Pipe& pipe) {
    // Check for collision with upper pipe
    if (bird.x + bird.width > pipe.x && bird.x < pipe.x + Pipe::pipeWidth && bird.y < pipe.gapY - Pipe::gapHeight / 2) {
        return true;
    }
    // Check for collision with lower pipe
    if (bird.x + bird.width > pipe.x && bird.x < pipe.x + Pipe::pipeWidth && bird.y + bird.height > pipe.gapY + Pipe::gapHeight / 2) {
        return true;
    }
    return false;
}


static void highScoreCheck(SDL_Renderer* renderer, TTF_Font* font, int score) {
    int highScore = 0;

    ifstream highScoreFile("highScore.txt");
    if (highScoreFile.is_open()) {
        highScoreFile >> highScore;
        highScoreFile.close();
    };

    if (score > highScore) {
        highScore = score;
        ofstream outFile("highScore.txt");
        outFile << highScore;
        outFile.close();
    };

    SDL_Color textColor = { 255, 255, 255 };
    std::string highScoreText = "High Score: " + std::to_string(highScore);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, highScoreText.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = { 100, 100, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

int getHighScore()
{
    int highScore = 0;
    std::ifstream highScoreFile("highScore.txt");
    if (highScoreFile.is_open())
    {
        highScoreFile >> highScore;
        highScoreFile.close();
    }
    return highScore;
}


int main(int argc, char* args[]) {
    // Initialize SDL, SDL_image, SDL_ttf, SDL_mixer
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (!IMG_Init(IMG_INIT_PNG)) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
        return 1;
    }
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        return 1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
        return 1;
    }

    // Create window and renderer
    SDL_Window* window = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Load textures // TODO: CHANGE FILE PATH TO NEW GRAPHICS
    bgTexture = IMG_LoadTexture(renderer, "sprites/background.png");
    SDL_Texture* birdTexture = IMG_LoadTexture(renderer, "sprites/bird_2.png");
    SDL_Texture* upperPipeTexture = IMG_LoadTexture(renderer, "sprites/upper_pipe_2.png");
    SDL_Texture* lowerPipeTexture = IMG_LoadTexture(renderer, "sprites/lower_pipe_2.png");
    SDL_Texture* gameOverTexture = IMG_LoadTexture(renderer, "sprites/game_over.png");
    SDL_Texture* coinTexture = IMG_LoadTexture(renderer, "sprites/coin.png");


    // Load font
    TTF_Font* font = TTF_OpenFont("font.ttf", 24);
    if (!font) {
        std::cerr << "Font loading failed: " << TTF_GetError() << std::endl;
        return 1;
    }

    // Load sounds
    Mix_Chunk* hitSound = Mix_LoadWAV("sounds/hit.wav");
    Mix_Chunk* scoreSound = Mix_LoadWAV("sounds/score.wav");
    Mix_Chunk* endSound = Mix_LoadWAV("sounds/end.wav");

    // Game variables
    bool running = true;
    SDL_Event event;
    Bird bird;
    std::list<Pipe> pipes;
    vector<PowerUp> powerUps;
    int score = 0;
    int life = 4;
    Uint32 lastTime = 0, lastPipeTime = 0, currentTime;
    int invincibilityTimer = 0;

    GameState gameState = START_SCREEN; // keep state of the screen we are on

    // Game loop
    while (running) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            // starting of game
            if (gameState == START_SCREEN && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
            {
                bird.velocity = -5;
                gameState = IN_GAME;
            }

            // continuation of game
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && birdAlive) {
                bird.velocity = -5;
            }
        }

        // loads in start screen
        if (gameState == START_SCREEN)
        {
            SDL_RenderClear(renderer); // Clear screen
            renderStartScreen(renderer, font);
            SDL_RenderPresent(renderer);
            SDL_Delay(100);
            continue;
        }

        // loads in game over screen
        if (gameState == GAME_OVER)
        {
            int highScore = getHighScore();
            SDL_RenderClear(renderer);
            renderGameOverScreen(renderer, font, score, highScore);
            SDL_RenderPresent(renderer);

            bool waitingForInput = true;
            while (waitingForInput)
            {
                while (SDL_PollEvent(&event))
                {
                    if (event.type == SDL_QUIT)
                    {
                        waitingForInput = false;
                        running = false; // Exit the game loop
                    }
                    else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
                    {
                        // Reset game state and relevant variables for restart
                        gameState = START_SCREEN;
                        score = 0;
                        life = 4; // Assuming you start with 4 lives
                        birdAlive = true;
                        initBird(bird); // Reset bird's position and state
                        pipes.clear(); // Clear any existing pipes
                        powerUps.clear(); // Clear power-ups
                        waitingForInput = false; // Exit the waiting loop
                        // No need to set running to false here unless you're exiting the game
                    }
                }
            }
        }

        // Game logic
        Uint32 currentTime = SDL_GetTicks();
        static Uint32 lastPowerUpSpawnTick = 0;
        const Uint32 powerUpSpawnInterval = 5000; // Adjust as needed, for example, 5000ms = 5 seconds

        if (currentTime - lastPowerUpSpawnTick > powerUpSpawnInterval) {
            trySpawnPowerUp(powerUps, pipes);
            lastPowerUpSpawnTick = currentTime; // Reset the timer
        }

        if (birdAlive && invincibilityTimer > 0) {
            invincibilityTimer--;
        }

        if (birdAlive) {
            bird.velocity += bird.gravity;
            bird.y += static_cast<int>(bird.velocity);
            if (bird.y < 0 || bird.y + bird.height > WINDOW_HEIGHT) {
                life--;
                if (life == 0) {
                    birdAlive = false;
                    Mix_PlayChannel(-1, endSound, 0);
                }
                else {
                    initBird(bird);
                }
            }
        }

        // Generate pipes
        if (birdAlive && currentTime > lastPipeTime + 1500) {
            Pipe newPipe;
            newPipe.x = WINDOW_WIDTH;
            newPipe.gapY = rand() % (WINDOW_HEIGHT - 2 * Pipe::gapHeight) + Pipe::gapHeight;
            pipes.push_back(newPipe);
            lastPipeTime = currentTime;
        }

        // Rendering
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
        renderPipes(renderer, upperPipeTexture, lowerPipeTexture, pipes);
        renderPowerUps(renderer, coinTexture, powerUps);
        SDL_Rect birdRect = { bird.x, bird.y, bird.width, bird.height };
        SDL_RenderCopy(renderer, birdTexture, NULL, &birdRect);
        renderScoreAndLives(renderer, font, score, life);
        SDL_RenderPresent(renderer);

        // Game over
        if (!birdAlive && life == 0) {
            highScoreCheck(renderer, font, score);
            gameState = GAME_OVER;
        }

        // Update pipes
        for (auto it = pipes.begin(); it != pipes.end();) {
            it->x -= 2;
            if (it->x + Pipe::pipeWidth < 0) {
                it = pipes.erase(it);
                if (birdAlive) {
                    Mix_PlayChannel(-1, scoreSound, 0);
                    score++;
                }
            }
            else if (birdAlive && invincibilityTimer == 0 && checkCollision(bird, *it)) {
                Mix_PlayChannel(-1, hitSound, 0);
                life--;
                if (life == 0) {
                    birdAlive = false;
                    Mix_PlayChannel(-1, endSound, 0);
                }
                else {
                    initBird(bird);
                    invincibilityTimer = 50;
                }
                ++it;
            }
            else {
                ++it;
            }
        }

        // Update power-ups
        updatePowerUps(powerUps);

        // Handle collision with power-ups
        handlePowerUpCollision(bird, powerUps, score, scoreSound);

        // Delay
        SDL_Delay(20);
    }

    // Clean up
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(birdTexture);
    SDL_DestroyTexture(upperPipeTexture);
    SDL_DestroyTexture(lowerPipeTexture);
    SDL_DestroyTexture(gameOverTexture);
    TTF_CloseFont(font);
    Mix_FreeChunk(hitSound);
    Mix_FreeChunk(scoreSound);
    Mix_FreeChunk(endSound);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}