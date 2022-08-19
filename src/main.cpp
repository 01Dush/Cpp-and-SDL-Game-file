#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <stdlib.h>

#include <header/renderwindow.h>
#include <header/entity.h>
#include <header/player.h>
#include <header/ground.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 480;

const int ALIVE = 0;
const int CURSOR_DEATH = 1;
const int HOLE_DEATH = 2;

const Uint8 *keyState;

RenderWindow window;

std::vector<SDL_Texture *> playerTex;
SDL_Texture *groundTex[4];
SDL_Texture *arrow;
SDL_Texture *highscoreBox;
SDL_Texture *deathOverlay;
SDL_Texture *logo;

TTF_Font *font32;
TTF_Font *font32_outline;
TTF_Font *font24;
TTF_Font *font16;

SDL_Color white = {255, 255, 255};
SDL_Color black = {0, 0, 0};

Mix_Chunk *bkgSfx;

bool gameRunning = true;
bool playedDeathSFX = false;
bool mainMenu = true;

bool init()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    window.create("Cursed", SCREEN_WIDTH, SCREEN_HEIGHT);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    srand((unsigned)time(0));

    playerTex.push_back(window.loadTexture("res/player/player0.png"));
    playerTex.push_back(window.loadTexture("res/player/player_1.png"));
    playerTex.push_back(window.loadTexture("res/player/player_2.png"));
    playerTex.push_back(window.loadTexture("res/player/player_3.png"));
    playerTex.push_back(window.loadTexture("res/player/player_4.png"));

    groundTex[0] = window.loadTexture("res/ground/left.png");
    groundTex[1] = window.loadTexture("res/ground/center.png");
    groundTex[2] = window.loadTexture("res/ground/right.png");
    groundTex[3] = window.loadTexture("res/ground/hole.png");

    arrow = window.loadTexture("res/arrow.png");
    highscoreBox = window.loadTexture("res/highscore_box.png");
    deathOverlay = window.loadTexture("res/death_overlay.png");
    logo = window.loadTexture("res/logo1.png");

    font32 = TTF_OpenFont("fonts/cocogoose.ttf", 32);
    font32_outline = TTF_OpenFont("fonts/cocogoose.ttf", 32);
    font24 = TTF_OpenFont("fonts/cocogoose.ttf", 24);
    font16 = TTF_OpenFont("fonts/cocogoose.ttf", 16);
    TTF_SetFontOutline(font32_outline, 3);

    bkgSfx = Mix_LoadWAV("sound/bkg.wav");

    return true;
}

bool load = init();

Player player(0, 0, playerTex);
Ground ground(groundTex[0], groundTex[1], groundTex[2], groundTex[3]);

void reset()
{
    player.reset();
    ground.reset();
}
void gameLoop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        Mix_PlayChannel(-1, bkgSfx, 0);
        switch (event.type)
        {
        case SDL_QUIT:
        {
            gameRunning = false;
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            if (mainMenu)
            {
                if (event.button.button == SDL_BUTTON_LEFT && SDL_GetTicks() > 2500)
                {
                    mainMenu = false;
                }
            }
            else
            {
                if (event.button.button == SDL_BUTTON_LEFT && player.isDead() == ALIVE)
                {
                    if (player.jump())
                    {
                    }
                }
                else if (player.isDead() != ALIVE)
                {
                    reset();
                }
            }
            break;
        }
        }
    }
    if (mainMenu)
    {
        if (SDL_GetTicks() < 10000)
        {
            if (SDL_GetTicks() < 5000)
            {
                window.clear();
                window.renderCenter(-80, -60, "Inspired by", font24, black);
                window.renderCenter(0, -24, "Cursor Custodian \n and \n LIMBO", font24, white);
                window.display();
            }
            else if (SDL_GetTicks() > 5000 && SDL_GetTicks() < 10000)
            {
                window.clear();
                window.renderCenter(-80, -60, "Developed by", font24, white);
                window.renderCenter(0, -24, "Dushyant Sharma", font32, black);
                window.display();
            }
        }
        else
        {
            window.clear();
            window.render(SCREEN_WIDTH / 2 - 234, SCREEN_HEIGHT / 2 - 94 - 30, logo);
            window.renderCenter(0, 100 + sin(SDL_GetTicks() / 100) * 2, "Click to start", font24, white);

            for (int i = 0; i < ground.getLength(); i++)
            {
                window.render(ground.getTile(i));
            }
            window.display();
        }
    }
    else
    {
        if (player.isDead() != CURSOR_DEATH)
        {
            ground.update(player.getScoreInt());
        }

        if (player.isDead() == ALIVE)
        {
            player.update(ground);
        }

        window.clear();

        window.render(player);
        for (int i = 0; i < ground.getLength(); i++)
        {
            window.render(ground.getTile(i));
        }
        window.render(25, 20, player.getScore(), font32_outline, black);
        window.render(28, 23, player.getScore(), font32, white);
        window.render(0, 65, highscoreBox);
        window.render(65, 64, player.getHighscore(), font16, white);

        if (player.isDead() != ALIVE)
        {
            window.render(deathOverlay);
            if (player.isDead() == CURSOR_DEATH)
            {
                window.renderCenter(0, -24, "The cursor is deadly to the player...", font24, white);
            }
            else if (player.isDead() == HOLE_DEATH)
            {
                window.renderCenter(0, -24, "Bottomless hole!", font24, white);
            }
            window.renderCenter(0, 100, "Click to retry.", font16, white);
        }
        window.display();
    }
}

int main(int argc, char *args[])
{
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(gameLoop, 0, 1);
#else
    while (gameRunning)
    {
        gameLoop();
        SDL_Delay(16);
    }
#endif

    window.cleanUp();
    TTF_CloseFont(font32);
    TTF_CloseFont(font32_outline);
    TTF_CloseFont(font24);
    TTF_CloseFont(font16);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
