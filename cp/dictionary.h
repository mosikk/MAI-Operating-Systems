#ifndef KEYLOGGER_DICTIONARY_H
#define KEYLOGGER_DICTIONARY_H

#include <unordered_set>
#include <algorithm>

std::string dictionary_file = "dictionary1.txt";

class dictionary {
private:
    std::unordered_set<std::string> dict;
    bool active = true;
public:
    dictionary() {
        std::ifstream in;
        in.open(dictionary_file, std::ios::in);
        if (!in.is_open()) {
            MessageBeep(MB_ICONERROR);
            active = false;
        }
        std::string word;
        while (in >> word) {
            std::transform(word.begin(), word.end(), word.begin(),
                           [](char c) { return std::tolower(c); });
            dict.insert(word);
        }
        in.close();
    }

    bool check(std::string &str) {
        std::transform(str.begin(), str.end(), str.begin(),
                       [](char c) { return std::tolower(c); });
        return dict.count(str);
    }

    bool is_active() const {
        return active;
    }
};


#endif //KEYLOGGER_DICTIONARY_H
