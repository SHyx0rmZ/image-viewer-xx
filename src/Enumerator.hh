// Created by Patrick Pokatilo on 2016/12/18.
// Copyright (c) 2016 即死ゲーム開発会社. All rights reserved.

#ifndef ENUMERATOR_HH
#define ENUMERATOR_HH

#include <string>
#include <vector>

struct Enumerator {
    virtual std::vector<std::string> enumerate() = 0;
};

#endif //ENUMERATOR_HH
