#include <iostream>
#include <memory>
#include <sstream>
#include <chrono>
using namespace std;

string currentPrompt = "$";

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
            cout << "The 'prompt' command requires an argument.\n";
        } else {
            currentPrompt = argument;
            cout << "Prompt updated to: " << currentPrompt << endl;
        }
    }
};

class Echo : public Command {
public:
    Echo(string argument) : Command("",argument) {}

    void execute() override {
        cout << "Echo: " << argument << endl;
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


unique_ptr<Command> CommandFactory(const string& commandName, const string& option, const string& argument) {
    if (commandName == "echo") {
        return make_unique<Echo>(argument);
    } else if (commandName == "prompt") {
        return make_unique<Prompt>(argument);
    } else if (commandName == "time") {
        return make_unique<Time>();
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

