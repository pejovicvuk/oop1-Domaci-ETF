#include <iostream>
#include <memory>
#include <sstream>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <regex>
using namespace std;

string currentPrompt = "$";
string extractInputFromArgument(const string& input) {
    regex re(R"(\"(.*)\")");
    smatch match;

    if (regex_search(input, match, re) && match.size() > 1) {
        return match[0].str();
    } else {
        size_t pos = input.find_first_of(" \t");
        if (pos != string::npos) {
            return input.substr(0, pos);
        } else {
            return input;
        }
    }
}
string extractOutputFromArgument(const string& input) {
    string extractedInput = extractInputFromArgument(input);

    size_t inputEndPos = input.find(extractedInput) + extractedInput.length();
    if (inputEndPos < input.length()) {
        size_t outputStartPos = input.find_first_not_of(" \t", inputEndPos);
        if (outputStartPos != string::npos) {
            return input.substr(outputStartPos);
        }
    }
    return "";
}

class Command {
protected:
    string option;
    string argument;
public:
    Command(string _option = "", string _argument = "") : option(_option), argument(_argument) {}

    virtual void execute() {}

    virtual ~Command() {}
};

class Prompt : public Command {
public:
    Prompt(string argument) : Command("", argument) {}

    void execute() override {
        if (argument.empty()) {
            cout << "The 'prompt' command requires an argument." << endl;
        } else {
            currentPrompt = argument;
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

        string textToOutput;
        if (!input.empty() && input[0] == '"') {
            textToOutput = input.substr(1, input.length() - 2);
        } else {
            ifstream inputFile(input);
            if (inputFile) {
                textToOutput.assign((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
                inputFile.close();
            } else {
                cout << "Error: Unable to open input file " << input << endl;
                return;
            }
        }

        if (!output.empty()) {
            ofstream outputFile(output);
            if (outputFile) {
                outputFile << textToOutput;
                outputFile.close();
                cout << "Text written to " << output << endl;
            } else {
                cout << "Error: Unable to open output file " << output << endl;
            }
        } else {
            cout << textToOutput << endl;
        }
    }
};
class Time : public Command {
public:
    Time() : Command("", "") {}

    void execute() override {
        auto now = chrono::system_clock::now();
        time_t currentTime = chrono::system_clock::to_time_t(now);
        tm localTime = *localtime(&currentTime);

        cout << put_time(&localTime, "%H:%M:%S")<< endl;
    }
};
class Date : public Command {
    public:
    Date() : Command("", "") {}
    void execute() override {
        auto now = chrono::system_clock::now();
        time_t currentTime = chrono::system_clock::to_time_t(now);
        tm localTime = *localtime(&currentTime);

        cout << put_time(&localTime, "%Y-%m-%d") << endl;
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
            cout<< "Contents of " << argument << "have successfully been deleted." << endl;
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

unique_ptr<Command> CommandFactory(const string& commandName, const string& option, const string& argument) {
    if (commandName == "echo") {
        return make_unique<Echo>(argument);
    } else if (commandName == "prompt") {
        return make_unique<Prompt>(argument);
    } else if (commandName == "time") {
        return make_unique<Time>();
    } else if (commandName == "date") {
        return make_unique<Date>();
    } else if (commandName == "touch") {
        return make_unique<Touch>(argument);
    } else if (commandName == "rm") {
        return make_unique<Rm>(argument);
    } else if (commandName == "truncate") {
        return make_unique<Truncate>(argument);
    }
    return nullptr;
}

int main() {
    while (true) {
        cout << currentPrompt;
        string inputLine;
        getline(cin, inputLine);

        if (inputLine.empty()) continue;

        string commandName;
        string option;
        string argument;

        istringstream inputStream(inputLine);

        inputStream >> commandName;

        string temp;
        inputStream >> temp;
        if (!temp.empty() && temp[0] == '-') {
            option = temp;
            getline(inputStream, argument);
        } else {
            getline(inputStream, argument);
            if (!temp.empty()) {
                argument = temp + (argument.empty() ? "" : "" + argument);
            }
        }

        if (!argument.empty() && argument[0] == ' ') {
            argument.erase(0, 1);
        }

        if (commandName == "exit") {
            cout << "Exiting the command prompt." << endl;
            break;
        }

        unique_ptr<Command> cmd = CommandFactory(commandName, option, argument);
        if (cmd) {
            cmd->execute();
        } else {
            cout << "Unknown or invalid command: " << commandName << endl;
        }
    }

    return 0;
}