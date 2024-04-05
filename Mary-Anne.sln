// Mary-Anne Ibeh  Chloe Quijano Masuma Begum
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <list>
#include <iostream>
#include <ctime>
#include <string>
#include <SDL_mixer.h>


#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600

bool birdAlive = true;

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

    // Load textures
    SDL_Texture* bgTexture = IMG_LoadTexture(renderer, "sprites/background.png");
    SDL_Texture* birdTexture = IMG_LoadTexture(renderer, "sprites/bird.png");
    SDL_Texture* upperPipeTexture = IMG_LoadTexture(renderer, "sprites/upper_pipe.png");
    SDL_Texture* lowerPipeTexture = IMG_LoadTexture(renderer, "sprites/lower_pipe.png");
    SDL_Texture* gameOverTexture = IMG_LoadTexture(renderer, "sprites/game_over.png");

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
    int score = 0;
    int life = 4;
    Uint32 lastTime = 0, currentTime;
    int invincibilityTimer = 0;

    // Game loop
    while (running) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && birdAlive) {
                bird.velocity = -5;
            }
        }

        // Game logic
        currentTime = SDL_GetTicks();
        if (birdAlive && currentTime > lastTime + 1500) {
            Pipe newPipe;
            newPipe.x = WINDOW_WIDTH;
            newPipe.gapY = rand() % (WINDOW_HEIGHT - 2 * Pipe::gapHeight) + Pipe::gapHeight;
            pipes.push_back(newPipe);
            lastTime = currentTime;
        }

        if (invincibilityTimer > 0) {
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

        // Rendering
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
        renderPipes(renderer, upperPipeTexture, lowerPipeTexture, pipes);
        SDL_Rect birdRect = { bird.x, bird.y, bird.width, bird.height };
        SDL_RenderCopy(renderer, birdTexture, NULL, &birdRect);
        renderScoreAndLives(renderer, font, score, life);
        if (!birdAlive) {
            SDL_RenderCopy(renderer, gameOverTexture, NULL, NULL);
        }
        SDL_RenderPresent(renderer);

        // Game over
        if (!birdAlive && life == 0) {
            SDL_Delay(2000);
            running = false;
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
