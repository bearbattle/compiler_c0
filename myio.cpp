//
// Created by bear on 2020/9/19.
//

#include <iostream>

void printf(const char *str) {
    std::cout << str << std::endl;
}

void printf(int num) {
    std::cout << num << std::endl;
}

void printf(char ch) {
    std::cout << ch << std::endl;
}

void printf(const char *str, int num) {
    std::cout << str << num << std::endl;
}

void printf(const char *str, char ch) {
    std::cout << str << ch << std::endl;
}

void scanf(int &num) {
    std::cin >> num;
}

void scanf(char &ch) {
    std::cin >> ch;
}

void my_main();

int main() {
    my_main();
    return 0;
}