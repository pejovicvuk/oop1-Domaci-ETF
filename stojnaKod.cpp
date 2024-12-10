#include <iostream>
#include <regex>
#include <string>

std::string extractSubstringBetweenQuotes(const std::string& input) {
    std::regex re(R"(\"(.*)\")"); //regex za izvlacenje iz ""
    std::smatch match;

    if (std::regex_search(input, match, re) && match.size() > 1) {
        return match[1].str();
    } else {
        return "";
    }
}

int main() {
    std::string input;

    // Input the string to process
    std::cout << "Enter a string: ";
    std::getline(std::cin, input);

    std::string result = extractSubstringBetweenQuotes(input);

    if (!result.empty()) {
        std::cout << "Substring between the first and last quotation marks: " << result << std::endl;
    } else {
        std::cout << "No substring found between the first and last quotation marks." << std::endl;
    }

    return 0;
}
