#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <list>
#include <iostream>
#include <ctime>
#include <string>

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600

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
    bird.width = 40; // Ensure this is set before calculating the position
    bird.height = 50; // Ensure this is set before calculating the position
    bird.x = (WINDOW_WIDTH / 2) - (bird.width / 2); // Center the bird horizontally
    bird.y = (WINDOW_HEIGHT / 2) - (bird.height / 2); // Center the bird vertically
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

void renderStartScreen(SDL_Renderer* renderer, TTF_Font* font)
{
    SDL_Color textColor = { 255, 255, 255 };
    const char* message = "Press SPACE to start";

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, message, textColor);
    if (textSurface == NULL)
    {
        std::cerr << "Unable to create text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }
    else
    {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture == NULL)
        {
            std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        }
        else
        {
            int textWidth = textSurface->w;
            int textHeight = textSurface->h;

            SDL_Rect textRect = { (WINDOW_WIDTH - textWidth) / 2, (WINDOW_HEIGHT - textHeight) / 2, textWidth, textHeight };

            SDL_RenderCopy(renderer, textTexture, NULL, &textRect); // Render the text to the screen

            SDL_DestroyTexture(textTexture); // Clean up
        }

        // Free the surface
        SDL_FreeSurface(textSurface);
    }
}

void renderGameOverScreen(SDL_Renderer* renderer, TTF_Font* font, int score)
{
    SDL_Color textColor = { 255, 255, 255 };
    std::string gameOverText = "Game Over! Score: " + std::to_string(score) + ". Press SPACE to exit.";

    SDL_Surface* surface = TTF_RenderText_Solid(font, gameOverText.c_str(), textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    int textWidth = surface->w;
    int textHeight = surface->h;
    SDL_Rect renderQuad = { (WINDOW_WIDTH - textWidth) / 2, (WINDOW_HEIGHT - textHeight) / 2, textWidth, textHeight };

    SDL_RenderCopy(renderer, texture, NULL, &renderQuad);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
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
    SDL_Texture* bgTexture = IMG_LoadTexture(renderer, "sprites/background_2.png");
    SDL_Texture* birdTexture = IMG_LoadTexture(renderer, "sprites/bird_2.png");
    SDL_Texture* upperPipeTexture = IMG_LoadTexture(renderer, "sprites/upper_pipe_2.png");
    SDL_Texture* lowerPipeTexture = IMG_LoadTexture(renderer, "sprites/lower_pipe_2.png");

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

    GameState gameState = START_SCREEN; // keep state of the screen we are on

    Uint32 lastTime = SDL_GetTicks() - 1500; // Adjust the time so the first pipe spawns quickly
    Uint32 currentTime;
    srand(static_cast<unsigned int>(time(nullptr)));


    while (running) {
        currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= 1400) { // Pipe spawn interval (in milliseconds)
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

            // starting of game
            if (gameState == START_SCREEN && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                bird.velocity = -5;
                gameState = IN_GAME;
            }

            // continues game
            if (gameState == IN_GAME && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
            {
                bird.velocity = -5;
            }
        }

        // loads in start screen
        if (gameState == START_SCREEN) {
            SDL_RenderClear(renderer); // Clear screen
            renderStartScreen(renderer, font);
            SDL_RenderPresent(renderer);
            SDL_Delay(100);
            continue;
        }

        // loads in game over screen
        if (gameState == GAME_OVER)
        {
            SDL_RenderClear(renderer);
            renderGameOverScreen(renderer, font, score);
            SDL_RenderPresent(renderer);

            bool waitingForInput = true;
            while (waitingForInput)
            {
                while (SDL_PollEvent(&event))
                {
                    if (event.type == SDL_QUIT)
                    {
                        waitingForInput = false;
                        running = false;
                    }
                    else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
                    {
                        waitingForInput = false;
                        running = false; // can remove if we want to restart the game
                    }
                }
            }
            continue;
        }

        bird.velocity += bird.gravity;
        bird.y += static_cast<int>(bird.velocity);

        if (bird.y < 0 || bird.y + bird.height > WINDOW_HEIGHT) {
            gameState = GAME_OVER;
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
                    gameState = GAME_OVER;
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
