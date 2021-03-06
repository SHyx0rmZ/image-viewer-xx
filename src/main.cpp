#include <algorithm>
#include <fstream>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <unordered_map>

#include "DirectoryScanner.hh"

const auto company = u8"即死ゲーム開発会社";
const auto product = "image viewer ++";
const int defaultWidth = 800;
const int defaultHeight = 600;

#define CACHE_IMAGES 1

// TODO:
// - speed up
// - add support for (animated) .gif files

struct RenderOptions
{
    float scale = 1.0f;
    SDL_Rect offset = { 0 };

    void reset()
    {
        this->scale = 1.0f;
        this->offset = { 0 };
    }
};

SDL_Surface *loadImage(const std::string &path) {
#if CACHE_IMAGES == 1
    static std::unordered_map<std::string, SDL_Surface *> cache;

    if (cache.find(path) == cache.end()) {
        cache[path] = IMG_Load(path.c_str());
    }

    return cache[path];
#else
    return IMG_Load(path.c_str());
#endif
}

void newImage(SDL_Window *window, SDL_Surface *screen, const std::string &path, RenderOptions &options, bool resizeWindow = false)
{
    auto image = loadImage(path);

    SDL_Rect imageSize;

    SDL_GetClipRect(image, &imageSize);

    if (resizeWindow) {
        if (imageSize.w > imageSize.h) {
            auto width = defaultWidth;
            auto height = static_cast<int>(static_cast<float>(imageSize.h) / static_cast<float>(imageSize.w) * defaultWidth);
            SDL_SetWindowSize(window, width, height);
        } else {
            auto width = static_cast<int>(static_cast<float>(imageSize.w) / static_cast<float>(imageSize.h) * defaultHeight);
            auto height = defaultHeight;
            SDL_SetWindowSize(window, width, height);
        }

        screen = SDL_GetWindowSurface(window);
    }

    SDL_Rect windowSize;

    SDL_GetClipRect(screen, &windowSize);

    int windowWidth;
    int windowHeight;

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    float imageWidthCorrected;
    float imageHeightCorrected;
    float imageWidthUnscaled = imageSize.w;
    float imageHeightUnscaled = imageSize.h;

    if (imageWidthUnscaled> imageHeightUnscaled) {
        imageWidthCorrected = static_cast<float>(windowWidth);
        imageHeightCorrected = static_cast<float>(windowWidth) * imageHeightUnscaled / imageWidthUnscaled;

        if (imageHeightCorrected > static_cast<float>(windowHeight)) {
            imageWidthCorrected = static_cast<float>(windowHeight) * imageWidthUnscaled / imageHeightUnscaled;
            imageHeightCorrected = static_cast<float>(windowHeight);
        }
    } else {
        imageWidthCorrected = static_cast<float>(windowHeight) * imageWidthUnscaled / imageHeightUnscaled;
        imageHeightCorrected = static_cast<float>(windowHeight);

        if (imageWidthCorrected > static_cast<float>(windowWidth)) {
            imageWidthCorrected = static_cast<float>(windowWidth);
            imageHeightCorrected = static_cast<float>(windowWidth) * imageHeightUnscaled / imageWidthUnscaled;
        }
    }

    float imageWidth = imageWidthCorrected * options.scale;
    float imageHeight = imageHeightCorrected * options.scale;
    float halfImageWidth = imageWidth / 2.0f;
    float halfImageHeight = imageHeight / 2.0f;
    float halfWindowWidth = windowWidth / 2.0f;
    float halfWindowHeight = windowHeight / 2.0f;
    float imageAbscissa = halfWindowWidth - halfImageWidth;
    float imageOrdinate = halfWindowHeight - halfImageHeight;

    if (options.scale > 1.0) {
        if (imageWidth >= windowWidth && options.offset.x - halfImageWidth + halfWindowWidth > 0.0f) {
            options.offset.x = static_cast<int>(halfImageWidth - halfWindowWidth);
        }

        if (imageHeight >= windowHeight && options.offset.y - halfImageHeight + halfWindowHeight > 0.0f) {
            options.offset.y = static_cast<int>(halfImageHeight - halfWindowHeight);
        }

        if (imageWidth >= windowWidth && options.offset.x + halfImageWidth - halfWindowWidth < 0.0f) {
            options.offset.x = static_cast<int>(halfWindowWidth - halfImageWidth);
        }

        if (imageHeight >= windowHeight && options.offset.y + imageHeight / 2.0 - halfWindowHeight < 0.0f) {
            options.offset.y = static_cast<int>(halfWindowHeight - halfImageHeight);
        }

        imageAbscissa = halfWindowWidth - halfImageWidth + options.offset.x;
        imageOrdinate = halfWindowHeight - halfImageHeight + options.offset.y;
    }

    SDL_Rect imagePosition;

    imagePosition.x = static_cast<int>(imageAbscissa);
    imagePosition.y = static_cast<int>(imageOrdinate);
    imagePosition.w = static_cast<int>(imageWidth);
    imagePosition.h = static_cast<int>(imageHeight);

    SDL_SetWindowTitle(window, path.c_str());

    SDL_FillRect(screen, nullptr, 0);

    SDL_BlitScaled(image, nullptr, screen, &imagePosition);

#if CACHE_IMAGES == 0
    SDL_FreeSurface(image);
#endif

    SDL_UpdateWindowSurface(window);

    SDL_FlushEvent(SDL_KEYDOWN);
    SDL_FlushEvent(SDL_MOUSEWHEEL);
    SDL_FlushEvent(SDL_MOUSEMOTION);
}

bool windowsIsNotMaximized(SDL_Window *window) {
    return (SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) == 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <image>" << std::endl;

        return EXIT_FAILURE;
    }

    std::string argument(argv[1]);

    if (argument.find('\\') != std::string::npos) {
        std::replace(argument.begin(), argument.end(), '\\', '/');
    }

    auto lastSeparator = argument.rfind('/');
    std::string directory;

    if (lastSeparator != std::string::npos) {
        directory = argument.substr(0, lastSeparator);
    } else {
        directory = ".";
    }

    std::cout << directory << std::endl;

    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    SDL_Rect windowSize = { 0 };
    windowSize.w = defaultWidth;
    windowSize.h = defaultHeight;

    SDL_Window *window = SDL_CreateWindow(product, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, defaultWidth,
        defaultHeight,
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL);
    SDL_Surface *screen = SDL_GetWindowSurface(window);

    auto path = SDL_GetPrefPath(company, product);

    auto pathFull = path + std::string("config.ini");

    std::cout << path << std::endl;

    SDL_RWops *file = SDL_RWFromFile(pathFull.c_str(), "w");

    SDL_RWwrite(file, "Hello, world!", 1, 13);
    SDL_RWclose(file);

    SDL_free(path);

    DirectoryScanner scanner(directory, 0);

    auto list = scanner.enumerate();
    auto current = std::find(list.cbegin(), list.cend(), argument);

    bool alive = true;
    bool zooming = false;
    bool resizing = false;
    bool moving = false;
    RenderOptions options;

    newImage(window, screen, *current, options, true);

    while (alive) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            alive = false;
                            break;

                        case SDLK_LEFT:
                            if (current == list.cbegin()) {
                                current = list.cend();
                            }

                            --current;

                            options.reset();
                            newImage(window, screen, *current, options);
                            break;

                        case SDLK_RIGHT:
                            ++current;

                            if (current == list.cend()) {
                                current = list.cbegin();
                            }

                            options.reset();
                            newImage(window, screen, *current, options);
                            break;

                        case SDLK_TAB:
                            zooming = !zooming;

                            break;

                        case SDLK_LSHIFT:
                            resizing = true;
                            SDL_SetRelativeMouseMode(SDL_TRUE);

                            break;

                        case SDLK_LCTRL:
                            moving = true;
                            SDL_SetRelativeMouseMode(SDL_TRUE);

                            break;

                        case SDLK_SPACE:
                            if (windowsIsNotMaximized(window)) {
                                newImage(window, screen, *current, options, true);
                            } else {
                                auto image = IMG_Load(current->c_str());

                                SDL_Rect imageSize;
                                SDL_Rect realWindowSize;
                                SDL_GetClipRect(image, &imageSize);
                                SDL_GetClipRect(screen, &realWindowSize);

                                SDL_FreeSurface(image);

                                options.scale = static_cast<float>(realWindowSize.w) /(static_cast<float>(realWindowSize.h) * static_cast<float>(imageSize.w) / static_cast<float>(imageSize.h));
                                options.offset.x = 0;
                                options.offset.y = static_cast<int>(static_cast<float>(imageSize.h) / 2.0f * options.scale);
                                newImage(window, screen, *current, options);
                            }

                            break;

                        default:
                            break;
                    }
                    break;

                case SDL_KEYUP:
                    if (event.key.keysym.sym == SDLK_LSHIFT) {
                        resizing = false;
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                        SDL_WarpMouseInWindow(window, windowSize.w / 2, windowSize.h / 2);
                    } else if (event.key.keysym.sym == SDLK_LCTRL) {
                        moving = false;
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                        SDL_WarpMouseInWindow(window, windowSize.w / 2, windowSize.h / 2);
                    }

                    break;

                case SDL_MOUSEWHEEL:
                    if (!zooming) {
                        if (event.wheel.y > 0) {
                            if (current == list.cbegin()) {
                                current = list.cend();
                            }

                            --current;
                        } else {
                            ++current;

                            if (current == list.cend()) {
                                current = list.cbegin();
                            }
                        }

                        options.reset();
                        newImage(window, screen, *current, options);
                    } else {
                        if (event.wheel.y > 0) {
                            options.scale *= 1.4128f;
                        } else {
                            options.scale *= 0.7078143f;
                        }

                        newImage(window, screen, *current, options);
                    }

                    break;

                case SDL_MOUSEBUTTONDOWN:
                    switch (event.button.button) {
                        case SDL_BUTTON_LEFT:
                            if (event.button.clicks == 1) {
                                moving = true;
                                SDL_SetRelativeMouseMode(SDL_TRUE);
                            } else if (event.button.clicks == 2) {
                                if (windowsIsNotMaximized(window)) {
                                    SDL_MaximizeWindow(window);
                                    SDL_Rect maximizedSize;
                                    SDL_Surface *maximizedScreen = SDL_GetWindowSurface(window);
                                    SDL_GetClipRect(maximizedScreen, &maximizedSize);
                                    SDL_WarpMouseInWindow(window, maximizedSize.w / 2, maximizedSize.h / 2);
                                } else {
                                    SDL_RestoreWindow(window);
                                    SDL_WarpMouseInWindow(window, windowSize.w / 2, windowSize.h / 2);
                                }
                            }

                            break;

                        case SDL_BUTTON_RIGHT:
                            resizing = true;
                            SDL_SetRelativeMouseMode(SDL_TRUE);

                            break;

                        case SDL_BUTTON_MIDDLE:
                            alive = false;

                            break;

                        default:
                            break;
                    }

                    break;

                case SDL_MOUSEBUTTONUP:
                    if (moving || resizing) {
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                        SDL_WarpMouseInWindow(window, windowSize.w / 2, windowSize.h / 2);
                        SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
                    }

                    switch (event.button.button) {
                        case SDL_BUTTON_LEFT:
                            moving = false;

                            break;

                        case SDL_BUTTON_RIGHT:
                            resizing = false;

                            break;

                        default:
                            break;
                    }

                    break;

                case SDL_MOUSEMOTION:
                    if (windowsIsNotMaximized(window) && resizing) {
                        windowSize.w = std::max(0, windowSize.w + event.motion.xrel);
                        windowSize.h = std::max(0, windowSize.h + event.motion.yrel);
                        SDL_SetWindowSize(window, windowSize.w, windowSize.h);
                    } else if (moving) {
                        if (zooming) {
                            options.offset.x += event.motion.xrel;
                            options.offset.y += event.motion.yrel;
                            newImage(window, screen, *current, options);
                        } else if (((SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) == 0)) {
                            windowSize.x += event.motion.xrel;
                            windowSize.y += event.motion.yrel;
                            SDL_SetWindowPosition(window, windowSize.x, windowSize.y);
                        }
                    }

                    break;

                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            screen = SDL_GetWindowSurface(window);
                            newImage(window, screen, *current, options);
                            break;

                        default:
                            break;
                    }

                    break;

                default:
                    break;
            }
        }
    }

    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(1);

    IMG_Quit();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
