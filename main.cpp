#include <iostream>
#include <SDL.h>
#include <SDL_image.h>

#include "DirectoryScanner.hh"

int main(int argc, char *argv[]) {
    DirectoryScanner scanner(".", 0);

    auto list = scanner.enumerate();

    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    for (auto file : list) {
        std::cout << file << std::endl;

        auto surface = IMG_Load(file.c_str());

        std::cout << "\t" << (void*)surface << std::endl;

        SDL_FreeSurface(surface);
    }

    IMG_Quit();

    return 0;
}
