#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <list>
#include <iostream>
#include <ctime>
#include <string>

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600

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

void renderPipes(SDL_Renderer* renderer, SDL_Texture* upperPipeTexture, SDL_Texture* lowerPipeTexture, std::list<Pipe>& pipes) {
    for (Pipe& pipe : pipes) {
        // Upper pipe
        SDL_Rect topRect = { pipe.x, 0, Pipe::pipeWidth, pipe.gapY - Pipe::gapHeight / 2 };
        SDL_RenderCopy(renderer, upperPipeTexture, NULL, &topRect);

        // Lower pipe
        SDL_Rect bottomRect = { pipe.x, pipe.gapY + Pipe::gapHeight / 2, Pipe::pipeWidth, WINDOW_HEIGHT - (pipe.gapY + Pipe::gapHeight / 2) };
        SDL_RenderCopy(renderer, lowerPipeTexture, NULL, &bottomRect);
    }
}

bool checkCollision(const Bird& bird, const Pipe& pipe) {
    if (bird.x < pipe.x + Pipe::pipeWidth && bird.x + bird.width > pipe.x) {
        if (bird.y < pipe.gapY - Pipe::gapHeight / 2 || bird.y + bird.height > pipe.gapY + Pipe::gapHeight / 2) {
            return true;
        }
    }
    return false;
}

int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();


    SDL_Window* window = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* bgTexture = IMG_LoadTexture(renderer, "sprites/background.png");
    SDL_Texture* birdTexture = IMG_LoadTexture(renderer, "sprites/bird.png");
    SDL_Texture* upperPipeTexture = IMG_LoadTexture(renderer, "sprites/upper_pipe.png");
    SDL_Texture* lowerPipeTexture = IMG_LoadTexture(renderer, "sprites/lower_pipe.png");

    TTF_Font* font = TTF_OpenFont("font.ttf", 24);

    bool running = true;
    SDL_Event event;
    Bird bird;
    initBird(bird);
    std::list<Pipe> pipes;
    int score = 0;

    // Create an initial pipe to show on screen immediately
    //Pipe initialPipe;
    //initialPipe.x = WINDOW_WIDTH / 2; // Adjust X position as necessary
    //initialPipe.gapY = rand() % (WINDOW_HEIGHT - 2 * Pipe::gapHeight) + Pipe::gapHeight;
    //pipes.push_back(initialPipe);

    Uint32 lastTime = SDL_GetTicks() - 1500; // Adjust the time so the first pipe spawns quickly
    Uint32 currentTime;
    srand(static_cast<unsigned int>(time(nullptr)));


    while (running) {
        currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= 1500) { // Pipe spawn interval (in milliseconds)
            Pipe newPipe;
            newPipe.x = WINDOW_WIDTH;
            newPipe.gapY = rand() % (WINDOW_HEIGHT - 2 * Pipe::gapHeight) + Pipe::gapHeight;
            pipes.push_back(newPipe);
            lastTime = currentTime;
        }



        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                bird.velocity = -5;
            }
        }

        bird.velocity += bird.gravity;
        bird.y += static_cast<int>(bird.velocity);

        if (bird.y < 0 || bird.y + bird.height > WINDOW_HEIGHT) {
            running = false;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        renderPipes(renderer, upperPipeTexture, lowerPipeTexture, pipes);


        SDL_Rect birdRect = { bird.x, bird.y, bird.width, bird.height };
        SDL_RenderCopy(renderer, birdTexture, NULL, &birdRect);

        for (auto it = pipes.begin(); it != pipes.end();) {
            it->x -= 2;
            if (it->x + Pipe::pipeWidth < 0) {
                it = pipes.erase(it);
                score++;
            }
            else {
                if (checkCollision(bird, *it)) {
                    running = false;
                }
                ++it;
            }
        }

        SDL_Color textColor = { 255, 255, 255 };
        std::string scoreText = "Score: " + std::to_string(score);
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(birdTexture);
    SDL_DestroyTexture(upperPipeTexture);
    SDL_DestroyTexture(lowerPipeTexture);
    TTF_CloseFont(font);

    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

