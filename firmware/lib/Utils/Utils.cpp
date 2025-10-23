#include "Utils.h"

std::vector<String> splitArgs(const String& input) {
    std::vector<String> tokens;
    String token;
    for (unsigned int i = 0; i < input.length(); i++) {
        char c = input[i];
        if (c == ' ') {
            if (token.length() > 0) {
                tokens.push_back(token);
                token = "";
            }
        } else {
            token += c;
        }
    }
    if (token.length() > 0) tokens.push_back(token);
    return tokens;
}