#include <iostream>
#include <memory>
#include <sstream>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <regex>
#include <thread>
using namespace std;

string currentPrompt = "$";


class Command {
protected:
    string option;
    string input;
    string output;
public:
    Command(string _option = "", string _input = "", string _output = "") : option(_option), input(_input), output(_output) {}

    virtual void execute() {}

    virtual void OutputToFile(string input, string fileName) {
        ofstream file(fileName);
        if (file) {
            file << input;
            file.close();
            cout << "Text written to " << fileName << endl;
        } else {
            cout << "Unable to open file " << fileName << endl;
        }
    }
    virtual string FileToString(string _string, string fileName) {
        if (ifstream file(fileName); file) {
            _string.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            file.close();
            return _string;
        } else {
            cout << "Error: Unable to open input file " << fileName << endl;
            return "";
        }
    }

    string GetCurrentTime(const std::string& format) {
        auto now = std::chrono::system_clock::now();
        time_t currentTime = std::chrono::system_clock::to_time_t(now);
        tm localTime = *localtime(&currentTime);

        std::ostringstream oss;
        oss << std::put_time(&localTime, format.c_str());
        return oss.str();
    }
    int countWords(string input) {
        istringstream stream(input);
        string word;
        int wordCount = 0;
        while (stream >> word) {
            wordCount++;
        }
        return wordCount;
    }

    virtual ~Command() {}
};

class Prompt : public Command {
public:
    Prompt(string input) : Command("", input) {}

    void execute() override {
        if (input.empty()) {
            cout << "The 'prompt' command requires an argument." << endl;
        } else {
            currentPrompt = input;
            cout << "Prompt updated to: " << currentPrompt << endl;
        }
    }
};
class Echo : public Command {
public:
    Echo(string option = "", string input = "", string output = "") : Command(option, input, output) {}

    void execute() {

        string text;
        if (!input.empty() && input[0] == '"') {
            text = input.substr(1, input.length() - 2);
        } else {
            text = FileToString(text, input);
        }
        if (!output.empty()) {
            OutputToFile(text, output);
        } else {
            cout << text << endl;
        }
    }
};
class Time : public Command {
public:
    Time(string option, string input) : Command("",argument) {}

    void execute() override {
        string currentTime = GetCurrentTime("%H:%M:%S");
        if (second.empty() && output.empty()) {
            cout << currentTime << endl;
        }
        else if (filesystem::exists(output) && second.empty()) {
            OutputToFile(currentTime, output);
        }
        else {
            cout << "Error: Unable to open output file " << output << endl;
        }
    }
};
class Date : public Command {
    public:
    Date(string argument) : Command("", argument) {}
    void execute() override {
        string currentTime = GetCurrentTime("%Y-%m-%d");
        string output =  extractInputFromArgument(argument);
        string second = extractOutputFromArgument(argument);
        if (second.empty() && output.empty()) {
            cout << currentTime << endl;
        }
        else if (filesystem::exists(output) && second.empty()) {
            OutputToFile(currentTime, output);
        }
        else {
            cout << "Error: Unable to open output file " << output << endl;
        }
    }
};
class Touch : public Command {
    public:
    Touch(string argument) : Command("",argument) {}
    void execute() override {
        if (argument.empty()) {
            cout << "Error: No file name provided." << endl;
        }
        else if (filesystem::exists(argument)) {
            cout << "File \"" << argument << "\" already exists." << endl;
        } else {
            ofstream MyFile(argument);
            if (MyFile.is_open()) {
                cout << "File \"" << argument << "\" created successfully." << endl;
                MyFile.close();
            } else {
                cout << "Failed to create file \"" << argument << "\"." << endl;
            }
        }
    }
};
class Truncate : public Command {
    public:
    Truncate(string argument) : Command("",argument) {}
    void execute() override {
        if (argument.empty()) {
            cout << "Error: No file name provided." << endl;
        } else if (filesystem::exists(argument)) {
            ofstream ofs;
            ofs.open(argument, ofstream::out | ofstream::trunc);
            ofs.close();
            cout<< "Contents of " << argument << " have successfully been deleted." << endl;
        }
        else {
            cout << "File " << argument << " does not exist." << endl;
        }
    }
};
class Rm : public Command {
    public:
    Rm(string argument) : Command("",argument) {}
    void execute() override {
        if (argument.empty()) {
            cout << "Error: No file name provided." << endl;
        }
        else if (filesystem::exists(argument)) {
            filesystem::remove(argument);
            cout << "File \"" << argument << "\" deleted successfully." << endl;
        } else {
            cout << "File \"" << argument << "\" does not exist." << endl;
        }
    }
};

class Wc : public Command {
    public:
    Wc(string option, string argument) : Command(option,argument) {}
    void execute() override {
        int wordCount = 0;
        string input =  extractInputFromArgument(argument);
        string output = extractOutputFromArgument(argument);
        if (option == "-w" && input[0] == '"' && output.empty()) {
            wordCount = countWords(input.substr(1, input.length() - 2));
            cout << wordCount << endl;
        } else if (option == "-w" && input[0] == '"' && !output.empty()){
            wordCount = countWords(input.substr(1, input.length() - 2));
            OutputToFile(to_string(wordCount), output);
        } else if (option == "-w" && input[0] != '"' && output.empty()) {
            input = FileToString( input, input);
            wordCount = countWords(input);
            cout << wordCount << endl;
        } else if (option == "-w" && input[0] != '"' && !output.empty()) {
            input = FileToString( input, input);
            wordCount = countWords(input);
            OutputToFile(to_string(wordCount), output);
        }
        else {
            cout << "Error: Invalid option." << endl;
        }
    }
};
unique_ptr<Command> CommandFactory(const string& commandName, const string& option, const string& input, const string& output) {
    if (commandName == "echo") {
        return make_unique<Echo>(input, output);
    } else if (commandName == "prompt") {
        return make_unique<Prompt>(input);
    } else if (commandName == "time") {
        return make_unique<Time>(output);
    } else if (commandName == "date") {
        return make_unique<Date>(output);
    } else if (commandName == "touch") {
        return make_unique<Touch>(input);
    } else if (commandName == "rm") {
        return make_unique<Rm>(input);
    } else if (commandName == "truncate") {
        return make_unique<Truncate>(input);
    } else if (commandName == "wc") {
        return make_unique<Wc>(option, input, output);
    }
    return nullptr;
}

class ErrorHandler {
public:
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
    vector<string>tokens;

    while (true) {
        string inputLine;
        cout << "$";
        getline(cin, inputLine);

        if (inputLine == "exit") {
            return 0;
        }

        if (!error_handler.ValidateCommand(inputLine)) {
            this_thread::sleep_for(chrono::milliseconds(200));
            continue;
        }else tokens = error_handler.tokenize(inputLine);
        unique_ptr<Command> cmd = CommandFactory(tokens[0], tokens[1], tokens[2], tokens[3]);
        if (cmd) {
            cmd->execute();
        } else {
            cout << "Unknown or invalid command: " << tokens[0] << endl;
        }
        cout << "Command is valid.\n";
    }
}
