//
// Created by chao4 on 2020/10/28 0028.
//

#include "Utility.h"

string tolower(const string &str) {
    string lowercase;
    for (char c : str) {
        lowercase += tolower(c);
    }
    return lowercase;
}
