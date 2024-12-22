#include <iostream>
#include <sstream>
#include <string>
#include <regex>
using namespace std;

class ErrorHandler {
public:
    bool ValidateCommand(const string& command) {
        vector<string> tokens = tokenize(command);

        string errorMessage;
        vector<int> errorPositions;

        //proveravamo ime komande
        // Validate commandName
        if (!validateCommandName(tokens[0], errorMessage, errorPositions)) {
            for (int& pos : errorPositions) {
                pos++;
            }
            displayError(command, errorPositions, errorMessage);
            return false;
        }
        // Validate commandOption
        errorMessage.clear();
        errorPositions.clear();
        if (!validateCommandOption(tokens[1], errorMessage, errorPositions)) {
            for (int& pos : errorPositions) {
                pos += tokens[0].length() + 2; // Adjust for "$ " and space after commandName
            }
            displayError(command, errorPositions, errorMessage);
            return false;
        }

        // Additional validations for input and output can go here...

        return true; // Command is valid if all checks pass

    }

private:
    bool validateCommandName(const string& token, string& error, vector<int>& errorPositions) {
        errorPositions.clear(); // Ensure error positions are reset

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
        if (token[0] != '-') {
            errorPositions.push_back(0);
            error = "Invalid Command Option: Must start with '-'.";
            return false;
        }
        for (size_t i = 1; i < token.length(); ++i) {
            if (!isalpha(token[i])) {
                errorPositions.push_back(i);
            }
        }
        if (!errorPositions.empty()) {
            error = "Invalid Command Option: Contains non-alphabetic characters.";
            return false;
        }
        return true;
    }

    bool validateInput(const string& token, string& error);
    bool validateOutput(const string& token, string& error);

    vector<string> tokenize(const string& command) {
        regex wordRegex(R"((\"[^\"]*\")|\S+)");
        vector<string> tokensRaw;
        vector<string> tokens(4, "");

        sregex_iterator iter(command.begin(), command.end(), wordRegex), end;
        while (iter != end) {
            tokensRaw.push_back(iter->str());
            ++iter;
        }

        if (tokensRaw.size() > 4) {
            cerr << "Error: Too many arguments." << endl;
            return {};
        }

        if (!tokensRaw.empty()) tokens[0] = tokensRaw[0];

        for (size_t i = 1; i < tokensRaw.size(); ++i) {
            if (tokensRaw[i][0] == '-') {
                tokens[1] = tokensRaw[i];
            } else if (tokensRaw[i][0] == '<') {
                tokens[3] = tokensRaw[i];
            } else {
                tokens[2] = tokensRaw[i];
            }
        }
        return tokens;
    }


    void displayError(const string& command, const vector<int>& errorPositions, const string& errorMessage) {
        cerr << command << endl;

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
        cout << "$" << flush; // Command prompt
        string inputLine;
        getline(cin, inputLine);

        if (inputLine == "exit") {
            return 0; // Exit the program
        }

        // Validate the command
        if (!error_handler.ValidateCommand(inputLine)) {
            cout << "Validation failed. Please correct the command and try again.\n";
            continue;
        }

        cout << "Command is valid.\n";
    }
}

