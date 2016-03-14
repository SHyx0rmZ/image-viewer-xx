// Created by Patrick Pokatilo on 2016/03/14.
// Copyright (c) 2016 即死ゲーム開発会社. All rights reserved.

#ifndef IMAGE_VIEWER_DIRECTORYSCANNER_HH
#define IMAGE_VIEWER_DIRECTORYSCANNER_HH

#include <string>
#include <limits>
#include <vector>

struct DirectoryScanner {
    DirectoryScanner(const std::string &root, int maxDepth = std::numeric_limits<int>::max()) : root(root), maxDepth(maxDepth) {
    }

    std::vector<std::string> enumerate();

private:
    std::string root;
    int maxDepth;

    std::vector<std::string> enumerate_recursive(const std::string &root, int depth);
};

#endif //IMAGE_VIEWER_DIRECTORYSCANNER_HH
