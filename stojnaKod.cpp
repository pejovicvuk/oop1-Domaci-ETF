#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <thread>
using namespace std;

class ErrorHandler {
public:
    bool ValidateCommand(const string& command) {
        vector<string> tokens = tokenize(command);
        if (tokens.empty()) return false;

        string errorMessage;
        vector<int> errorPositions;
        string combinedMarker(command.length(), ' ');
        vector<string> errorMessages;
        int errorCounter = 0;

        //provera commandName
        if (!validateCommandName(tokens[0], errorMessage, errorPositions)) {
            for (int& pos : errorPositions) {
                pos++;
            }
            for (int pos : errorPositions) {
                if (pos >= 0) {
                    combinedMarker[pos] = '^';
                }
            }
            errorMessages.push_back(errorMessage);
            errorCounter++;
        }

        // provera commandOption
        errorMessage.clear();
        errorPositions.clear();
        if (!validateCommandOption(tokens[1], errorMessage, errorPositions)) {
            size_t position = command.find(tokens[1]) + 1; // ovo plus 1 je za $, stavi ga posle +promptLenght
            for (int& pos : errorPositions) {
                pos += position;
            }
            for (int pos : errorPositions) {
                if (pos >= 0 ) {
                    combinedMarker[pos] = '^';
                }
            }
            errorMessages.push_back(errorMessage);
            errorCounter++;
        }

        errorMessage.clear();
        errorPositions.clear();
        if (!validateInput(tokens[2], errorMessage, errorPositions)) {
            size_t position = command.find(tokens[2]) + 1; // ovo plus 1 je za $, stavi ga posle +promptLenght
            for (int& pos : errorPositions) {
                pos += position;
            }
            for (int pos : errorPositions) {
                if (pos >= 0 ) {
                    combinedMarker[pos] = '^';
                }
            }
            errorMessages.push_back(errorMessage);
            errorCounter++;
        }
        errorMessage.clear();
        errorPositions.clear();
        if (!validateOutput(tokens[3], errorMessage, errorPositions)) {
            size_t position = command.find(tokens[3]) + 1; // ovo plus 1 je za $, stavi ga posle +promptLenght
            for (int& pos : errorPositions) {
                pos += position;
            }
            for (int pos : errorPositions) {
                if (pos >= 0 ) {
                    combinedMarker[pos] = '^';
                }
            }
            errorMessages.push_back(errorMessage);
            errorCounter++;
        }
        if (errorCounter > 0) {
            cerr << combinedMarker << endl;
            for (const string& msg : errorMessages) {
                cerr << msg << endl;
            }
            return false;
        }
        return true;
    }


private:
    bool validateCommandName(const string& token, string& error, vector<int>& errorPositions) {
        errorPositions.clear();

        for (size_t i = 0; i < token.length(); ++i) {
            if (!isalpha(token[i])) {
                errorPositions.push_back(i);
            }
        }
        if (!errorPositions.empty()) {
            error = "Invalid Command Name: Only alphabetic characters are allowed.";
            return false;
        }
        return true;
    }

    bool validateCommandOption(const string& token, string& error, vector<int>& errorPositions) {
        errorPositions.clear();

        if (token.empty()) {
            return true;
        }
        // if (token[0] != '-') {
        //     errorPositions.push_back(0);
        //     error = "Invalid Command Option: Must start with '-'.";
        //     return false;
        // }
        for (size_t i = 1; i < token.length(); ++i) {
            if (!isalpha(token[i])) {
                errorPositions.push_back(i);
            }
        }
        if (!errorPositions.empty()) {
            error = "Invalid Command Option: Only alphabetic characters are allowed.";
            return false;
        }
        return true;
    }

    bool validateInput(const string& token, string& error, vector<int>& errorPositions) {
        errorPositions.clear();

        if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
            return true;
        }
        static const string invalidChars = "<>:\"/\\|?*-";
        for (size_t i = 0; i < token.size(); ++i) {
            if (invalidChars.find(token[i]) != string::npos) {
                errorPositions.push_back(i);
            }
        }
        if (token.size() > 255) {
            error = "Invalid Input Syntax: File name exceeds the maximum allowed length (255 characters).";
            return false;
        }
        if (!errorPositions.empty()) {
            error = "Invalid Input Syntax: File name contains invalid characters.";
            return false;
        }
        return true;
    }

    bool validateOutput(const string& token, string& error, vector<int>& errorPositions) {
        errorPositions.clear();


        static const string invalidChars = "<>:\"/\\|?*-";
        for (size_t i = 1; i < token.size(); ++i) {
            if (invalidChars.find(token[i]) != string::npos) {
                errorPositions.push_back(i);
            }
        }
        if (token.size() > 255) {
            error = "Invalid Output Syntax: File name exceeds the maximum allowed length (255 characters).";
            return false;
        }
        if (!errorPositions.empty()) {
            error = "Invalid Output Syntax: File name contains invalid characters.";
            return false;
        }
        return true;
    }

    vector<string> tokenize(const string& command) {
        regex wordRegex(R"((\S*\"[^\"]*\"\S*)|\S+)");

        vector<string> tokensRaw;
        vector<string> tokens(4, "");

        sregex_iterator iter(command.begin(), command.end(), wordRegex), end;
        while (iter != end) {
            tokensRaw.push_back(iter->str());
            ++iter;
        }

        if (tokensRaw.size() > 4) {
            cerr << "Error: Too many arguments." << endl;
            return {}; //sta da radim ovde kada je error u tokenize funkciji
        }

        if (!tokensRaw.empty()) tokens[0] = tokensRaw[0]; //prvi token je uvek ime komande
        // if (tokensRaw[1][0] == '-') { //ako opcija postoji, ona je uvek drugi token
        //     tokens[1] = tokensRaw[1];
        //     if (tokensRaw.size() == 3) {
        //         tokens[2] = tokensRaw[2];
        //     }
        // }
        // //ako ne postoji opcija
        // else if (tokensRaw.size() == 2) {
        //     if (tokens[1][0] == '>') tokens[4] = tokensRaw[1];
        //     else tokens[3]= tokensRaw[1];
        // } else if (tokensRaw.size() == 3) {
        //     tokens[1] = tokensRaw[1];
        //     tokens[2] = tokensRaw[2];
        // }
        for (size_t i = 1; i < tokensRaw.size(); ++i) {
            if (tokensRaw[i][0] == '-' && i == 1) {
                tokens[1] = tokensRaw[i];
            } else if (tokensRaw[i][0] == '>') {
                tokens[3] = tokensRaw[i];
            } else {
                tokens[2] = tokensRaw[i];
            }
        }
        //ne treba ovako
        if (tokensRaw.size() == 4) {
            if (tokens[1][0] != '-') {
                cerr << "Invalid Command Option: Must start with '-'." << endl;
                return {};
            }
            if (tokens[3][0] != '>') {
                cerr << "Invalid Output: Must start with '>'." << endl;
                return {};
            }
        }
        return tokens;
    }


    void displayError(const string& command, const vector<int>& errorPositions, const string& errorMessage) {

        string marker(command.length(), ' ');
        for (int pos : errorPositions) {
            if (pos >= 0 && pos < static_cast<int>(command.length())) {
                marker[pos] = '^';
            }
        }
        cerr << marker << endl;
        cerr << errorMessage << endl;
    }
};

int main() {
    ErrorHandler error_handler;

    while (true) {
        string inputLine;
        cout << "$";
        getline(cin, inputLine);

        if (inputLine == "exit") {
            return 0;
        }

        if (!error_handler.ValidateCommand(inputLine)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
        cout << "Command is valid.\n";
    }
}

