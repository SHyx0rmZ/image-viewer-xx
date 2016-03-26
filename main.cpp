#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <SDL.h>
#include <SDL_image.h>
//#include <unistd.h>
//#include <rpc.h>

#include "DirectoryScanner.hh"

const auto company = u8"即死ゲーム開発会社";
const auto product = "image viewer ++";
const int defaultWidth = 800;
const int defaultHeight = 600;

void newImage(SDL_Window *window, SDL_Surface *screen, const std::string &path, float scale, SDL_Rect &offset, bool resizeWindow = false)
{
    SDL_FillRect(screen, nullptr, 0);

    auto image = IMG_Load(path.c_str());

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

    float num1;
    float num2;

    if (imageSize.w > imageSize.h) {
        num1 = static_cast<float>(windowSize.w);
        num2 = static_cast<float>(windowSize.w) * static_cast<float>(imageSize.h) / static_cast<float>(imageSize.w);

        if (num2 > static_cast<float>(windowSize.h)) {
            num1 = static_cast<float>(windowSize.h) * static_cast<float>(imageSize.w) / static_cast<float>(imageSize.h);
            num2 = static_cast<float>(windowSize.h);
        }
    } else {
        num1 = static_cast<float>(windowSize.h) * static_cast<float>(imageSize.w) / static_cast<float>(imageSize.h);
        num2 = static_cast<float>(windowSize.h);

        if (num1 > static_cast<float>(windowSize.w)) {
            num1 = static_cast<float>(windowSize.w);
            num2 = static_cast<float>(windowSize.w) * static_cast<float>(imageSize.h) / static_cast<float>(imageSize.w);
        }
    }

    float width1 = num1 * scale;
    float height1 = num2 * scale;
    float x1 = static_cast<float>(windowSize.w) / 2.0f - width1 / 2.0f;
    float y1 = static_cast<float>(windowSize.h) / 2.0f - height1 / 2.0f;

    if (scale > 1.0) {
        if (width1 >= windowSize.w && offset.x - width1 / 2.0f + windowSize.w / 2.0f > 0.0f) {
            offset.x = static_cast<int>(width1 / 2.0f - windowSize.w / 2.0f);
        }

        if (height1 >= windowSize.h && offset.y - height1 / 2.0f + windowSize.h / 2.0f > 0.0f) {
            offset.y = static_cast<int>(height1 / 2.0f - windowSize.h / 2.0f);
        }

        if (width1 >= windowSize.w && offset.x + width1 / 2.0f - windowSize.w / 2.0f < 0.0f) {
            offset.x = static_cast<int>(windowSize.w / 2.0f - width1 / 2.0f);
        }

        if (height1 >= windowSize.h && offset.y + height1 / 2.0 - windowSize.h / 2.0f < 0.0f) {
            offset.y = static_cast<int>(windowSize.h / 2.0f - height1 / 2.0f);
        }

        x1 = static_cast<float>(windowSize.w) / 2.0f - width1 / 2.0f + offset.x;
        y1 = static_cast<float>(windowSize.h) / 2.0f - height1 / 2.0f + offset.y;
    }

    SDL_Rect imagePosition;

    imagePosition.x = static_cast<int>(x1);
    imagePosition.y = static_cast<int>(y1);
    imagePosition.w = static_cast<int>(width1);
    imagePosition.h = static_cast<int>(height1);

    SDL_SetWindowTitle(window, path.c_str());

    SDL_BlitScaled(image, nullptr, screen, &imagePosition);

    SDL_FreeSurface(image);

    SDL_UpdateWindowSurface(window);

    SDL_FlushEvent(SDL_KEYDOWN);
    SDL_FlushEvent(SDL_MOUSEWHEEL);
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
    std::tuple<int, int> origin;
    float scale = 1.0f;
    SDL_Rect offset = { 0 };

    newImage(window, screen, *current, scale, offset, true);

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

                            scale = 1.0f;
                            offset = { 0 };
                            newImage(window, screen, *current, scale, offset);
                            break;

                        case SDLK_RIGHT:
                            ++current;

                            if (current == list.cend()) {
                                current = list.cbegin();
                            }

                            scale = 1.0f;
                            offset = { 0 };
                            newImage(window, screen, *current, scale, offset);
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
                            if ((SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) == 0) {
                                newImage(window, screen, *current, scale, offset, true);
                            } else {
                                auto image = IMG_Load(current->c_str());

                                SDL_Rect imageSize;
                                SDL_Rect realWindowSize;
                                SDL_GetClipRect(image, &imageSize);
                                SDL_GetClipRect(screen, &realWindowSize);

                                SDL_FreeSurface(image);

                                scale = static_cast<float>(realWindowSize.w) / (static_cast<float>(realWindowSize.h) * static_cast<float>(imageSize.w) / static_cast<float>(imageSize.h));
                                offset.x = 0;
                                offset.y = static_cast<int>(static_cast<float>(imageSize.h) / 2.0f * scale);
                                newImage(window, screen, *current, scale, offset);
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

                        scale = 1.0f;
                        offset = { 0 };
                        newImage(window, screen, *current, scale, offset);
                    } else {
                        if (event.wheel.y > 0) {
                            scale *= 1.4128f;
                        } else {
                            scale *= 0.7078143f;
                        }

                        newImage(window, screen, *current, scale, offset);
                    }

                    break;

                case SDL_MOUSEBUTTONDOWN:
                    switch (event.button.button) {
                        case SDL_BUTTON_LEFT:
                            if (event.button.clicks == 1) {
                                moving = true;
                                SDL_SetRelativeMouseMode(SDL_TRUE);
                            } else if (event.button.clicks == 2) {
                                if ((SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) == 0) {
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
                            origin = std::make_tuple(0, 0);
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
                            origin = std::make_tuple(-1, -1);

                            break;

                        case SDL_BUTTON_RIGHT:
                            resizing = false;

                            break;

                        default:
                            break;
                    }

                    break;

                case SDL_MOUSEMOTION:
                    if (((SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) == 0) && resizing) {
                        windowSize.w = std::max(0, windowSize.w + event.motion.xrel);
                        windowSize.h = std::max(0, windowSize.h + event.motion.yrel);
                        SDL_SetWindowSize(window, windowSize.w, windowSize.h);
                        origin = std::make_tuple(event.motion.x, event.motion.y);
                    } else if (moving) {
                        if (zooming) {
                            offset.x += event.motion.xrel;
                            offset.y += event.motion.yrel;
                            newImage(window, screen, *current, scale, offset);
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
                            newImage(window, screen, *current, scale, offset);
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
