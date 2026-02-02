#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include "main.h"

static constexpr Sint32 TILE_SIZE = 32;
static constexpr Sint32 ANIM_ROW_BEGIN = 0;
static constexpr Sint32 ANIM_ROW_END = 0;
static constexpr Sint32 ANIM_COL_BEGIN = 0;
static constexpr Sint32 ANIM_COL_END = 6;
static constexpr Sint32 PRESENT_SIZE = 8;

static Uint32 TimerCallback(void *userdata, SDL_TimerID timerID, Uint32 interval)
{
    bool *updateFlag = static_cast<bool *>(userdata);
    *updateFlag = true;
    return interval;
}

class ValentineApp{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *spritesheet = nullptr;
    SDL_Texture *spriteDeer = nullptr;//pour rajouter une image de cerf
    SDL_Texture *spriteHeart = nullptr; //Pour rajouter les images de coeurs

    float animTimer = 0.0f;
    int currentCol = ANIM_COL_BEGIN;
    int currentRow = ANIM_ROW_BEGIN;
    const float frameDuration = 0.15f;

    float colorTime = 0.0f;
    Uint8 r = 255, g = 192, b = 203;
//Fonts et textes
    TTF_Font *font = nullptr;
    TTF_TextEngine *textEngine = nullptr;
    TTF_Text *fpsText = nullptr;
    TTF_Text *Title = nullptr;
    TTF_Text *textYes = nullptr; //Text dans le rectangle blue
    TTF_Text *textNo = nullptr;  //Text dans le rectangle rouge


    std::vector<float> frameTimes;
    const size_t MAX_SAMPLES = 100;
    bool shouldUpdateText = false;
    SDL_TimerID fpsTimerID;
    float currentFPS = 0.0f;

    float animationRotation = 45.0f;//l'animation tourne a 45 degree
    float animationHeartRotation = -25.0f;//animationtourne a . degree par la gaucher
    //Pour les boutons
    SDL_FRect YesButtonRect = { 250, 600, 200, 60 };
    SDL_FRect NoButtonRect = { 550, 600, 200, 60 };
    //Si appuyer sur bouton oui alors text qui s'affiche
    bool hasClickedOnYes = false;
    TTF_Text *hasClickedOnYesText = nullptr;
    //Si la personne clic sur faux alors on lui redommande une autre fois + Grossissement du Yes Boutton
    bool hasClickedOnNoOnce = false;
    bool hasClickedOnNoTwice = false;
    bool hasClickedOnNoAThirdTime = false;
    TTF_Text *hasClickedOnNoOnceText = nullptr;
    TTF_Text *hasClickedOnNoTwiceText = nullptr;
    TTF_Text *hasClickedOnNoAThirdTimeText = nullptr;



    public :        //Constructeur
         ValentineApp() {
        if (SDL_Init(SDL_INIT_VIDEO) == false)
        {
            SDL_LogCritical(1, "SDL failed to initialize! %s", SDL_GetError());
            abort();
        }
        window = SDL_CreateWindow("Valentine Game", 1000, 800, 0);
        if (window == nullptr)
        {
            SDL_LogCritical(1, "SDL failed to create window! %s", SDL_GetError());
            abort();
        }
        renderer = SDL_CreateRenderer(window, nullptr);
        if (renderer == nullptr)
        {
            SDL_LogCritical(1, "SDL failed to create renderer! %s", SDL_GetError());
            abort();
        }
        spritesheet = IMG_LoadTexture(renderer, "assets/spritesheet.png");
        if (spritesheet == nullptr)
        {
            SDL_LogWarn(0, "SDL_image failed to load texture '%s'! %s", "assets/spritesheet.png",
                        SDL_GetError());
        }

        SDL_SetTextureScaleMode(spritesheet, SDL_SCALEMODE_NEAREST);
        if (TTF_Init() == false)
        {
            SDL_LogCritical(1, "SDL_ttf failed to initialize! %s", SDL_GetError());
            abort();
        }
        //rajout du cerf
        spriteDeer = IMG_LoadTexture(renderer, "assets/SpriteSheetDeer.png");
        if (spriteDeer == nullptr) {
            SDL_LogWarn(0, "SDL_image failed to load texture '%s'! %s", "assets/SpriteSheetDeer.png",SDL_GetError());
        }
        //scale pour l'image du cerf
        SDL_SetTextureScaleMode(spriteDeer, SDL_SCALEMODE_NEAREST);
        if (TTF_Init() == false)
        {
            SDL_LogCritical(1, "SDL_ttf failed to initialize! %s", SDL_GetError());
            abort();
        }
        //Rajout des coeurs
        spriteHeart = IMG_LoadTexture(renderer, "assets/spritesheetheart.png");
        if (spriteHeart == nullptr) {
            SDL_LogWarn(0, "SDL_image failed to load texture , assets/spritesheetheart.png", SDL_GetError());
        }
        //Scale de l'image coeurs
        SDL_SetTextureScaleMode(spriteHeart, SDL_SCALEMODE_NEAREST);
        if (TTF_Init() == false)
        {
            SDL_LogCritical(1, "SDL_ttf failed to initialize! %s", SDL_GetError());
            abort();
        }
        textEngine = TTF_CreateRendererTextEngine(renderer);
        if (textEngine == nullptr)
        {
            SDL_LogCritical(1, "SDL_ttf failed to create text engine!! %s", SDL_GetError());
            abort();
        }
        font = TTF_OpenFont("assets/font.ttf", 48);
        if (font == nullptr)
        {
            SDL_LogWarn(0, "SDL_ttf failed to load font '%s'! %s", "assets/font.ttf", SDL_GetError());
        }
        fpsText = TTF_CreateText(textEngine, font, "FPS: 0", 20);
        if (fpsText == nullptr)
        {
            SDL_LogWarn(0, "SDL_ttf failed to create text '%s'! %s", "FPS: 0", SDL_GetError());
        }
        if (TTF_SetTextColor(fpsText, 255, 255, 255, 255) == false)
        {
            SDL_LogWarn(0, "SDL_ttf failed to set text color to (255, 255, 255, 255)! %s", SDL_GetError());
        }
        Title = TTF_CreateText(textEngine, font , "Will you be my Valentine ? :3", 50);
        if (Title == nullptr)
        {
            SDL_LogWarn(0, "SDL_ttf failed to create Title", SDL_GetError());
        }
        fpsTimerID = SDL_AddTimer(1000, TimerCallback, &shouldUpdateText);//interval one seconde
        //Pour le texte sur le bouton oui et non
        textYes = TTF_CreateText(textEngine, font, "Yes :3", 20);
        if (textYes == nullptr) {
            SDL_LogWarn(0, "Error to create Yes on button", SDL_GetError());
        }
        textNo = TTF_CreateText(textEngine, font, "No ;w;", 20);
        if (textNo == nullptr) {
            SDL_LogWarn(0, "Error to create No on button", SDL_GetError());
        }
        hasClickedOnYesText = TTF_CreateText(textEngine,font ,"  AWWWWW Thank you -.- ", 50);

        if (hasClickedOnYesText == nullptr) {
            SDL_LogWarn(0, "SDL_ttf failed to create text", SDL_GetError());
        }
        //Pour les textes non une et deux fois
        hasClickedOnNoOnceText = TTF_CreateText(textEngine, font, "nuuu You sure ? qwp", 40);
        if (hasClickedOnNoOnceText == nullptr) {
            SDL_LogWarn(0, "SDL_ttf hasClickedOnNoOnceText didnt appeared", SDL_GetError());
        }
        hasClickedOnNoTwiceText = TTF_CreateText(textEngine, font, "Really ? qwp ;w;  ", 40);
        if (hasClickedOnNoTwiceText == nullptr) {
            SDL_LogWarn(0, "SDL_ttf failed to create the second no text", SDL_GetError());
        }
        hasClickedOnNoAThirdTimeText = TTF_CreateText(textEngine, font, "Ouch -_- oki ", 40);
        if (hasClickedOnNoAThirdTimeText == nullptr) {
            SDL_LogWarn(0, "Le message du troixieme n'est pas apparue", SDL_GetError());
        }
    }
//Destruction des objects pour optimiser
    ~ValentineApp() {

            SDL_RemoveTimer(fpsTimerID);
            TTF_DestroyText(fpsText);
            TTF_DestroyText(Title);//pour supprimer le titre rajouter
            TTF_DestroyText(textYes); //Pour supprimer Yes
            TTF_DestroyText(textNo);  //Pour supprimer No
            TTF_DestroyText(hasClickedOnYesText);
            TTF_DestroyText(hasClickedOnNoOnceText);
            TTF_DestroyText(hasClickedOnNoTwiceText);
            TTF_DestroyText(hasClickedOnNoAThirdTimeText);
            TTF_DestroyRendererTextEngine(textEngine);
            TTF_CloseFont(font);
            SDL_DestroyTexture(spritesheet);
            SDL_DestroyTexture(spriteDeer);//detruit le cerf
            SDL_DestroyTexture(spriteHeart); //destruction du coeur
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();

    }
    void CalculateFPS(const float deltaTime)
    {
        frameTimes.push_back(deltaTime);
        if (frameTimes.size() > MAX_SAMPLES)
        {
            frameTimes.erase(frameTimes.begin());
        }
        const float sum = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0f);
        const float avgDelta = sum / static_cast<float>(frameTimes.size());
        currentFPS = (avgDelta > 0) ? 1.0f / avgDelta : 0;

        if (shouldUpdateText)
        {
            std::string fpsStr = "FPS: " + std::to_string(static_cast<int>(currentFPS));
            TTF_SetTextString(fpsText, fpsStr.c_str(), 0);
            shouldUpdateText = false; // Reset the flag
        }
    }

    void AdvanceAnimation(const float deltaTime)
    {
        animTimer += deltaTime;
        if (animTimer >= frameDuration)
        {
            animTimer = 0.0f;
            currentCol++;

            if (currentCol > ANIM_COL_END)
            {
                currentCol = ANIM_COL_BEGIN;
                currentRow++;
            }

            if (currentRow > ANIM_ROW_END)
            {
                currentRow = ANIM_ROW_BEGIN;
            }
        }
    }

    //No update only simple pink color
    void UpdateBackgroundTint(const float deltaTime)
    {
        constexpr float speed = 1.2f;
        colorTime += deltaTime * speed;

        constexpr float Amplitude = 60.0f;
        constexpr float MidPoint = 60.0f;

        r = static_cast<Uint8>(std::clamp(std::sin(colorTime) * Amplitude + MidPoint, 255.0f, 255.0f));
        g = static_cast<Uint8>(std::clamp(std::sin(colorTime + 2.0f) * Amplitude + MidPoint, 192.0f, 255.0f));
        b = static_cast<Uint8>(std::clamp(std::sin(colorTime + 4.0f) * Amplitude + MidPoint, 203.0f, 255.0f));
    }
    void RenderAnimation() const
    {
        //POUR L'ANIMATION DE BASE
        if (spritesheet != nullptr)
        {
            const SDL_FRect src = {
                static_cast<float>(currentCol * TILE_SIZE),
                static_cast<float>(currentRow * TILE_SIZE),
                static_cast<float>(TILE_SIZE),
                static_cast<float>(TILE_SIZE),
        };

            constexpr SDL_FRect dst = {
                (14500.0f / 2.0f) - ((TILE_SIZE * PRESENT_SIZE) / 2.0f),//en dehord de l'ecran pour ne pas le voir
                (450.0f / 2.0f) - ((TILE_SIZE * PRESENT_SIZE) / 2.0f),

                static_cast<float>((TILE_SIZE * PRESENT_SIZE)),
                static_cast<float>((TILE_SIZE * PRESENT_SIZE)),
        };
            //Le point centre de l'animation
            constexpr SDL_FPoint center = {
                dst.w / 1.0f,
                dst.h / 1.0f
            };
            //
            //SDL_RenderTextureRotated pour
            SDL_RenderTextureRotated(renderer, spritesheet, &src, &dst, animationRotation, &center, SDL_FLIP_NONE);
           // SDL_RenderTexture(renderer, spritesheet, &src, &dst);
        }
        //POUR LE CERF SANS ANIMATION
        if (spriteDeer != nullptr) {

            // Calculer la taille d'une seule sprite (1 cerf)
            float textureW;//Width
            float textureH;//Height
            SDL_GetTextureSize(spriteDeer, &textureW, &textureH);//prend l'adresse de textureW/H

            float oneDeerW = textureW / 4.1f; // 4 colonnes
            float oneDeerH = textureH / 2.0f; // 2 lignes

            // Choisir QUEL cerf afficher
            float targetCol = 0;//4 colognes (0,1,2,3)
            float targetRow = 0;//2 lignes (0,1)
            float deerX; // Position X
            float deerY; // Position Y
            float scaleFactor;//l'agrantissement

            if (hasClickedOnYes) {
            targetCol = 3;
            targetRow = 0;
            deerX = 350.0f;
            deerY = 300.0f;
            scaleFactor = 1.5f;
            }
            else if (hasClickedOnNoAThirdTime) {
                targetCol = 3;
                targetRow = 1;
                deerX = 300.0f;
                deerY = 300.0f;
                scaleFactor = 1.0f;
            }else if (hasClickedOnNoTwice){
                targetCol = 1;
                targetRow = 1;
                deerX = 70.0f;
                deerY = 170.0f;
                scaleFactor = 0.5f;
            }
            else if (hasClickedOnNoOnce) {
                targetCol = 0;
                targetRow = 0;
                deerX = 525.0f;
                deerY = 300.0f;
                scaleFactor = 1.0f;
            }

            else {
                targetCol = 2;
                targetRow = 0;
                deerX = 380.0f;
                deerY = 280.0f;
                scaleFactor = 1.0f;
            }
            SDL_FRect srcDeer = {
                targetCol * oneDeerW,  // X dans l'image
                targetRow * oneDeerH,  // Y dans l'image
                oneDeerW,              // Largeur d'un cerf
                oneDeerH               // Hauteur d'un cerf
            };
            SDL_FRect dstDeer = {
                deerX,
                deerY,
                oneDeerW * scaleFactor, //
                oneDeerH * scaleFactor  // Hauteur
            };
            SDL_RenderTexture(renderer,spriteDeer,&srcDeer, &dstDeer );
        }
        if (spriteHeart != nullptr) {
 // Calculer la taille d'une seule sprite (1 cerf)
            float textureW;//Width
            float textureH;//Height
            SDL_GetTextureSize(spriteHeart, &textureW, &textureH);//prend l'adresse de textureW/H

            float oneHeartW = textureW / 5.3f; // 4 colonnes
            float oneHeartH = textureH / 3.0f; // 2 lignes

            // Choisir QUEL cerf afficher
            float targetCol = 0;//4 colognes (0,1,2,3)
            float targetRow = 0;//2 lignes (0,1)
            float HeartX; // Position X
            float HeartY; // Position Y
            float scaleFactor;//l'agrantissement

            if (hasClickedOnYes) {
            targetCol = 1;
            targetRow = 0;
            HeartX = 120.0f;
            HeartY = 120.0f;
            scaleFactor = 0.35f;
            }
            else if (hasClickedOnNoAThirdTime) {
                targetCol = 2;
                targetRow = 2;
                HeartX = 180.0f;
                HeartY = 120.0f;
                scaleFactor = 0.6f;
            }else if (hasClickedOnNoTwice){
                targetCol = 4;
                targetRow = 1;
                HeartX = 250.0f;
                HeartY = 150.0f;
                scaleFactor = 0.30f;
            }
            else if (hasClickedOnNoOnce) {
                targetCol = 3;
                targetRow = 2;
                HeartX = 180.0f;
                HeartY = 130.0f;
                scaleFactor = 0.30f;
            }

            else {
                targetCol = 3;
                targetRow = 0;
                HeartX = 140.0f;
                HeartY = 110.0f;
                scaleFactor = 0.30f;
            }
            SDL_FRect srcHeart = {
                targetCol * oneHeartW,  // X dans l'image
                targetRow * oneHeartH,  // Y dans l'image
                oneHeartW,              // Largeur d'un cerf
                oneHeartH               // Hauteur d'un cerf
            };
            SDL_FRect dstHeart = {
                HeartX,
                HeartY,
                oneHeartW * scaleFactor, //
                oneHeartH * scaleFactor  // Hauteur
            };
            //Le point centre de l'animation
            SDL_FPoint centerHeart = {
                dstHeart.w / 1.0f,
                dstHeart.h / 1.0f
            };

            SDL_RenderTextureRotated(renderer, spriteHeart, &srcHeart, &dstHeart, animationHeartRotation, &centerHeart, SDL_FLIP_NONE);
        }
    }
    //Pour le titre
    void RenderTitle() {

        TTF_DrawRendererText(Title, 200, 200);
    }

    //Pour les boutons Yes & No                                                     //r,g,b du bouton
    void RenderButtonsYesNo(const SDL_FRect& rect, TTF_Text* buttonText, Uint8 buttonr, Uint8 buttong, Uint8 buttonb ) {
        //Pour dessiner le rectangle du bouton
        SDL_SetRenderDrawColor(renderer, buttonr, buttong, buttonb, 255);
        SDL_RenderFillRect(renderer, &rect);

        //Dessiner Texte au centre du boutton
        if (buttonText != nullptr) {
            int textW, textH;//Longeur/Largeur
            TTF_GetTextSize(buttonText, &textW, &textH);


            float textX = rect.x + (rect.w - textW) / 2.0f;
            float textY = rect.y + (rect.h - textH) / 2.0f;

            TTF_DrawRendererText(buttonText, textX, textY);
        }
    }

    void Run() {
        bool running = true;
        uint64_t lastTime = SDL_GetTicks();

       float exitTimer = 0.0f; //Timer avant que le jeu ce ferme
        while (running)
        {   //SDL_Event is a union of all event structures used in SDL
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                    running = false;

                // Gestion des boutons
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
                {
                    SDL_FPoint mousePt = { event.button.x, event.button.y };

                    if (!hasClickedOnYes && SDL_PointInRectFloat(&mousePt, &YesButtonRect)) {
                        std::cout << "Awww~ >.< " << std::endl;
                        hasClickedOnYes = true;
                    }
                    else if (!hasClickedOnYes && SDL_PointInRectFloat(&mousePt, &NoButtonRect)) {

                        if (!hasClickedOnNoOnce) {
                            hasClickedOnNoOnce = true;
                        }
                        else if (!hasClickedOnNoTwice) {
                            hasClickedOnNoTwice = true;
                        }
                        else  {
                            hasClickedOnNoAThirdTime = true;
                        }
                        //Agrandissement du bouton oui a chaque fois que non est appuyer
                        YesButtonRect.w += 150;
                        YesButtonRect.h += 150;
                        //Ils vont legerement vers le haut
                        YesButtonRect.x += -80;
                        YesButtonRect.y += -150;
                    }
                }
            }

            const uint64_t currentTime = SDL_GetTicks();
            const float deltaTime = static_cast<float>(currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;
            CalculateFPS(deltaTime);
            //tout sur deltaTime
            AdvanceAnimation(deltaTime);
            UpdateBackgroundTint(deltaTime);
            if (hasClickedOnNoAThirdTime) {
                exitTimer += deltaTime;
                if (exitTimer > 3.0f) {
                    running = false;//On arrete le programme apres 3
                }
            }


            //Render (Couleurs)
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderClear(renderer);
            if (hasClickedOnYes) {
                //Devient true et on peut afficher le message
                TTF_DrawRendererText(hasClickedOnYesText, 200, 200);
                RenderAnimation();
            }
            else {
                if (hasClickedOnNoAThirdTime) {
                    TTF_DrawRendererText(hasClickedOnNoAThirdTimeText, 350, 200);
                    RenderAnimation();
                }
                else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Couleur bleue
                    RenderButtonsYesNo(YesButtonRect, textYes, 229, 255, 204);   // Vert pour Oui
                    RenderButtonsYesNo(NoButtonRect, textNo, 255, 125, 125);     // Rouge pour Non
                    //Renders qui ne s'affiche plus apres le premier non
                    if (!hasClickedOnNoOnce) {
                        RenderAnimation();
                        RenderTitle();

                    }
                    TTF_DrawRendererText(fpsText, 10, 10);
                    if (hasClickedOnNoTwice) {
                        TTF_DrawRendererText(hasClickedOnNoTwiceText, 330, 200);//Affichage du texte no twice
                        RenderAnimation();
                    }
                    else if (hasClickedOnNoOnce) {
                        TTF_DrawRendererText(hasClickedOnNoOnceText, 250, 200);
                        RenderAnimation();
                    }
                }

            }


            SDL_RenderPresent(renderer);
        }
    }
};



int main(int argc, char *argv[])
{

    ValentineApp app;

    app.Run();

    return 0;
}