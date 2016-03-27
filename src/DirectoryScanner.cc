// Created by Patrick Pokatilo on 2016/03/14.
// Copyright (c) 2016 即死ゲーム開発会社. All rights reserved.

#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

#include "DirectoryScanner.hh"

using std::move;
using std::string;
using std::strncmp;
using std::vector;

vector<string> DirectoryScanner::enumerate() {
    return this->enumerate_recursive(this->root, 0);
}

vector<string> DirectoryScanner::enumerate_recursive(const string &root, int depth) {
    if (depth > this->maxDepth) {
        return vector<string>();
    }

    vector<string> entries;
    auto dir = opendir(root.c_str());

    while (auto entry = readdir(dir)) {
        auto fullPath = root + "/" + entry->d_name;

        struct stat status;

        stat(fullPath.c_str(), &status);

        if ((status.st_mode & S_IFMT) == S_IFDIR && strncmp(entry->d_name, ".", 1) && strncmp(entry->d_name, "..", 2)) {
            for (auto child : this->enumerate_recursive(fullPath, depth + 1)) {
                entries.emplace_back(move(child));
            }
        } else if ((status.st_mode & S_IFMT) == S_IFREG) {
            auto lastDot = fullPath.rfind(".");
            auto extension = fullPath.substr(lastDot == string::npos ? 0 : lastDot);

            if (extension == ".jpg" || extension == ".jpeg") {
                entries.emplace_back(fullPath);
            } else if (extension == ".png") {
                entries.emplace_back(fullPath);
            } else if (extension == ".gif") {
                entries.emplace_back(fullPath);
            } else if (extension == ".bmp") {
                entries.emplace_back(fullPath);
            }
        }
    }

    closedir(dir);
    return entries;
}
