#include <iostream>
#include <memory>
#include <sstream>
using namespace std;

class Command {
protected:
    string option;
    string argument;
public:
    Command(string _option = "", string _argument = "") : option(_option), argument(_argument) {}

    virtual void execute(string& currentPrompt) {}

    virtual ~Command() {}
};

class Prompt : public Command {
    public:
        Prompt(string argument) : Command("", argument) {}

        void execute(string& currentPrompt) override {
            if (argument.empty()) {
                cout << "The 'prompt command requires an argument.\n";
            } else {
                currentPrompt = argument;
                cout << "Prompt updated to: " << currentPrompt << endl;
            }
        }

};

class Echo : public Command {
public:
    Echo(string argument) : Command("",argument) {}

    void execute(string& currentPrompt) override {
        cout << "Echo: " << argument << endl;
    }
};


unique_ptr<Command> CommandFactory(const string& commandName, const string& option, const string& argument) {
    if (commandName == "echo") {
        return make_unique<Echo>(argument);
    } else if (commandName == "prompt") {
        return make_unique<Prompt>(argument);
    }
    return nullptr;
}

int main() {
    string currentPrompt = "$";
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

        // Option (if present) starts with '-'
        string temp;
        inputStream >> temp;
        if (!temp.empty() && temp[0] == '-') {
            option = temp;
            getline(inputStream, argument);
        } else {
            getline(inputStream, argument);
            if (!temp.empty()) {
                argument = temp + (argument.empty() ? "" : " " + argument);
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
            cmd->execute(currentPrompt);
        } else {
            cout << "Unknown or invalid command: " << commandName << endl;
        }
    }

    return 0;
}

