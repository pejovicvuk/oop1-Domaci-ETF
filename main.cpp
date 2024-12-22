#include <iostream>
#include <memory>
#include <sstream>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <regex>
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
    Echo(string argument) : Command("",argument) {}

    void execute() {
        string input = extractInputFromArgument(argument);
        string output = extractOutputFromArgument(argument);

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
    Time(string argument) : Command("",argument) {}

    void execute() override {
        string currentTime = GetCurrentTime("%H:%M:%S");
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
    bool ValidateCommand(const string& command);
// private:
    bool validateCommandName(const string& token, string& error);
    bool validateCommandOption(const string& token, string& error) {
        
    }
    bool validateInput(const string& token, string& error);
    bool validateOutput(const string& token, string& error);

    vector<string> tokenize(const string& command) {
        regex wordRegex(R"((\"[^\"]*\")|\S+)");
        vector<string> tokens;
        sregex_iterator iter(command.begin(), command.end(), wordRegex), end;
        while (iter != end) {
            tokens.push_back(iter->str());
            iter++;
        }
        if (tokens.size() > 4) {
            cerr << "Error: Too many arguments." << endl;
            return {};
        }
        return tokens;
    }
    void displayError(const string& command, const vector<int>& errorPositions, const string& errorMessage);

};

int main() {
    ErrorHandler errorHandler;
    while (true) {
        cout << currentPrompt;
        string inputLine;
        getline(cin, inputLine);

        if (inputLine.empty()) continue;

        vector<string> tokens = errorHandler.tokenize(inputLine);


        unique_ptr<Command> cmd = CommandFactory(commandName, option, argument);
        if (cmd) {
            cmd->execute();
        } else {
            cout << "Unknown or invalid command: " << commandName << endl;
        }
    }

    return 0;
}
