#include <iostream>
#include <memory>
#include <sstream>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <regex>
#include <thread>
#include <set>
using namespace std;

string currentPrompt = "$";

class Command {
protected:
    string option;
    string input;
    string output;
public:
    Command(string _option = "", string _input = "", string _output = "") : option(_option), input(_input), output((_output)) {}

    virtual void execute() {}

    virtual void OutputToFile(string input, string fileName) {
        ofstream file(fileName);
        if (file) {
            file << input;
            file.close();
            cout << "Text written to " << fileName << endl;
        } else {
            cerr << "Unable to open output file " << fileName << endl;
        }
    }
    virtual string FileToString(string _string, string fileName) {
        if (ifstream file(fileName); file) {
            _string.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            file.close();
            return _string;
        } else {
            cerr << "Error: Unable to open input file " << fileName << endl;
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
    int countChars(string input) {
        int count = 0;
        for (char c : input) {
            if (!isspace(c)) {
                count++;
            }
        }
        return count;
    }

    virtual ~Command() {}
};
class ErrorHandler {
    vector<int> errorPositions;
    set<string> errorMessages;
public:
     void clear() {
        errorPositions.clear();
        errorMessages.clear();
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
            return {};
        }

        if (!tokensRaw.empty()) tokens[0] = tokensRaw[0]; //prvi token je uvek ime komande
        for (size_t i = 1; i < tokensRaw.size(); ++i) {
            if (tokensRaw[i][0] == '-' && i == 1) {
                tokens[1] = tokensRaw[i];
            } else if (tokensRaw[i][0] == '>') {
                tokens[3] = tokensRaw[i];
            } else {
                tokens[2] = tokensRaw[i];
            }
        }

         //provera viska inputa
         vector<string> assignedTokens = {tokens[0], tokens[1], tokens[2], tokens[3]};
         string errorMessage;

         for (const string& token : tokensRaw) {
             if (find(assignedTokens.begin(), assignedTokens.end(), token) == assignedTokens.end()) {
                 if (token[0] == '-') {
                     cerr << "Error: Extra option detected: " << endl;
                 } else if (token[0] == '>') {
                     cerr << "Error: Extra output detected: " << endl;
                 } else {
                     cerr << "Error: Extra input detected: " << endl;
                 }
                 return {};
             }
         }
        // brisanje > iz outputa
        if (!tokens[3].empty()) {
            tokens[3].erase(0, 1);
        }
        return tokens;
    }
    static void displayError(const string& command, const vector<int>& errorPositions, const string& errorMessage) {

        string marker(command.length(), ' ');
        for (int pos : errorPositions) {
            if (pos >= 0 && pos < static_cast<int>(command.length())) {
                marker[pos] = '^';
            }
        }
        cerr << marker << endl;
        cerr << errorMessage << endl;
    }
    bool ValidateCommand(const string& command) {
        vector<string> tokens = tokenize(command);
        if (tokens.empty()) return false;

        string errorMessage;
        string combinedMarker(command.length(), ' ');
        int errorCounter = 0;

        if (!validateCommandName(tokens[0], errorMessage, errorPositions)) {
            for (int& pos : errorPositions) {
                pos++;
            }
            for (int pos : errorPositions) {
                if (pos >= 0) {
                    combinedMarker[pos] = '^';
                }
            }
            errorMessages.insert(errorMessage);
            errorCounter++;
        }
        errorMessage.clear();
        if (!validateCommandOption(tokens[1], errorMessage, errorPositions)) {
            size_t position = command.find(tokens[1]) + currentPrompt.length();
            for (int& pos : errorPositions) {
                pos += position;
            }
            for (int pos : errorPositions) {
                if (pos >= 0 ) {
                    combinedMarker[pos] = '^';
                }
            }
            errorMessages.insert(errorMessage);
            errorCounter++;
        }
        errorMessage.clear();
        if (!validateInput(tokens[2], errorMessage, errorPositions)) {
            size_t position = command.find(tokens[2]) + currentPrompt.length();
            for (int& pos : errorPositions) {
                pos += position;
            }
            for (int pos : errorPositions) {
                if (pos >= 0 ) {
                    combinedMarker[pos] = '^';
                }
            }
            errorMessages.insert(errorMessage);
            errorCounter++;
        }
        errorMessage.clear();
        if (!validateOutput(tokens[3], errorMessage, errorPositions)) {
            size_t position = command.find(tokens[3]) + currentPrompt.length();
            for (int& pos : errorPositions) {
                pos += position;
            }
            for (int pos : errorPositions) {
                if (pos >= 0 ) {
                    combinedMarker[pos] = '^';
                }
            }
            errorMessages.insert(errorMessage);
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
        vector<int> localErrorPositions;
        for (size_t i = 0; i < token.length(); ++i) {
            if (!isalpha(token[i])) {
                localErrorPositions.push_back(i);
            }
        }
        if (!localErrorPositions.empty()) {
            errorPositions = localErrorPositions;
            error = "Invalid Command Name: Only alphabetic characters are allowed.";
            return false;
        }
        return true;
    }

    bool validateCommandOption(const string& token, string& error, vector<int>& errorPositions) {
        // vector<int> localErrorPositions;
        // if (token.empty()) {
        //     return true;
        // }
        // for (size_t i = 1; i < token.length(); ++i) {
        //     if (!isalpha(token[i])) {
        //         localErrorPositions.push_back(i);
        //     }
        // }
        // if (!localErrorPositions.empty()) {
        //     errorPositions = localErrorPositions;
        //     error = "Invalid Command Option: Only alphabetic characters are allowed.";
        //     return false;
        // }
        return true;
    }

    bool validateInput(const string& token, string& error, vector<int>& errorPositions) {
        vector<int> localErrorPositions;
        if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
            return true;
        }
        static const string invalidChars = "<>:\"/\\|?*-";
        for (size_t i = 0; i < token.size(); ++i) {
            if (invalidChars.find(token[i]) != string::npos) {
                localErrorPositions.push_back(i);
            }
        }
        if (token.size() > 255) {
            error = "Invalid Input Syntax: File name exceeds the maximum allowed length (255 characters).";
            return false;
        }
        if (!localErrorPositions.empty()) {
            errorPositions = localErrorPositions;
            error = "Invalid Input Syntax: File name contains invalid characters.";
            return false;
        }
        return true;
    }

    bool validateOutput(const string& token, string& error, vector<int>& errorPositions) {
        vector<int> localErrorPositions;
        static const string invalidChars = "<>:\"/\\|?*-";
        for (size_t i = 1; i < token.size(); ++i) {
            if (invalidChars.find(token[i]) != string::npos) {
                localErrorPositions.push_back(i);
            }
        }
        if (token.size() > 255) {
            error = "Invalid Output Syntax: File name exceeds the maximum allowed length (255 characters).";
            return false;
        }
        if (!localErrorPositions.empty()) {
            errorPositions = localErrorPositions;
            error = "Invalid Output Syntax: File name contains invalid characters.";
            return false;
        }
        return true;
    }
};
class Prompt : public Command {
public:
    Prompt(string input) : Command("", input, "") {}

    void execute() override {
        if (input.empty()) {
            cout << "The 'prompt' command requires an argument." << endl;
        } else {
            currentPrompt = input.substr(1, input.size() - 2);;
            cout << "Prompt updated to: " << currentPrompt << endl;
        }
    }
};
class Echo : public Command {
public:
    Echo(const string &input, const string &output) : Command("", input, output) {}
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
    Time(string output) : Command("", "", output) {}
    void execute() override {
        string currentTime = GetCurrentTime("%H:%M:%S");
        if (output.empty()) {
            cout << currentTime << endl;
        }
        else if (filesystem::exists(output)) {
            OutputToFile(currentTime, output);
        }
        else {
            cerr << "Error: Unable to open output file " << output << endl;
        }
    }
};
class Date : public Command {
    public:
    Date(string output) : Command("", "", output) {}
    void execute() override {
        string currentTime = GetCurrentTime("%Y-%m-%d");
        if (output.empty()) {
            cout << currentTime << endl;
        }
        else if (filesystem::exists(output)) {
            OutputToFile(currentTime, output);
        }
        else {
            cerr << "Error: Unable to open output file " << output << endl;
        }
    }
};
class Touch : public Command {
    public:
    Touch(string input) : Command("", input, "") {}
    void execute() override {
        if (input.empty()) {
            cerr << "Error: No file name provided." << endl;
        }
        else if (filesystem::exists(input)) {
            cerr << "File \"" << input << "\" already exists." << endl;
        } else {
            ofstream MyFile(input);
            if (MyFile.is_open()) {
                cout << "File \"" << input << "\" created successfully." << endl;
                MyFile.close();
            } else {
                cerr << "Failed to create file \"" << input << "\"." << endl;
            }
        }
    }
};
class Truncate : public Command {
    public:
    Truncate(string input) : Command("", input, "") {}
    void execute() override {
        if (input.empty()) {
            cerr << "Error: No file name provided." << endl;
        } else if (filesystem::exists(input)) {
            ofstream ofs;
            ofs.open(input, ofstream::out | ofstream::trunc);
            ofs.close();
            cout<< "Contents of " << input << " have successfully been deleted." << endl;
        }
        else {
            cerr << "File " << input << " does not exist." << endl;
        }
    }
};
class Rm : public Command {
    public:
    Rm(string input) : Command("", input, "") {}
    void execute() override {
        if (input.empty()) {
            cerr << "Error: No file name provided." << endl;
        }
        else if (filesystem::exists(input)) {
            filesystem::remove(input);
            cout << "File \"" << input << "\" deleted successfully." << endl;
        } else {
            cerr << "File \"" << input << "\" does not exist." << endl;
        }
    }
};
class Wc : public Command {
    public:
    Wc(string option, string input, string output) : Command(option,input, output) {}
    void execute() override {
        int wordCount = 0;
        int charCount = 0;
        if (option == "-w" && input[0] == '"' && output.empty()) {
            wordCount = countWords(input.substr(1, input.length() - 2));
            cout << wordCount << endl;
        } else if (option == "-w" && input[0] == '"' && !output.empty()){
            wordCount = countWords(input.substr(1, input.length() - 2));
            OutputToFile(to_string(wordCount), output);
        } else if (option == "-w" && input[0] != '"' && output.empty()) {
            FileToString( input, input);
            wordCount = countWords(input.substr(1, input.length() - 2));
            cout << wordCount << endl;
        } else  if (option == "-w" && input[0] != '"' && !output.empty()){
            FileToString( input, input);
            wordCount = countWords(input.substr(1, input.length() - 2));
            OutputToFile(to_string(wordCount), input);
        }
        else if (option == "-c" && input[0] == '"' && output.empty()) {
            charCount = countChars(input.substr(1, input.length() - 2));
            cout << charCount << endl;
        } else if (option == "-c" && input[0] == '"' && !output.empty()){
            charCount = countChars(input.substr(1, input.length() - 2));
            OutputToFile(to_string(charCount), output);
        } else if (option == "-c" && input[0] != '"' && output.empty()) {
            FileToString( input, input);
            charCount = countChars(input.substr(1, input.length() - 2));
            cout << charCount << endl;
        } else  if (option == "-c" && input[0] != '"' && !output.empty()){
            FileToString( input, input);
            charCount = countChars(input.substr(1, input.length() - 2));
            OutputToFile(to_string(charCount), input);
        }
    }
};
class Head : public Command {
public:
    Head(const string& option, const string& input, const string& output) : Command(option, input, output) {}

    void execute() override {
        int count = 0;
        if (option.length() > 1 && option[0] == '-') {
            try {
                count = stoi(option.substr(1));
                if (count <= 0) {
                    cerr << "Error: Option must specify a positive number of lines." << endl;
                    return;
                }
            } catch (const std::invalid_argument&) {
                cerr << "Error: Invalid number format in option." << endl;
                return;
            } catch (const std::out_of_range&) {
                cerr << "Error: Number in option is out of range." << endl;
                return;
            }
        } else {
            cerr << "Error: Invalid option format. Expected '-<number>'." << endl;
            return;
        }

        vector<string> lines;
        if (!input.empty() && input[0] == '"') {
            if (input.back() == '"') {
                string text = input.substr(1, input.length() - 2);

                istringstream stream(text);
                string line;
                while (getline(stream, line) && static_cast<int>(lines.size()) < count) {
                    lines.push_back(line);
                }
            } else {
                cerr << "Error: Mismatched quotes in input." << endl;
                return;
            }
        } else {
            ifstream inputFile(input);
            if (!inputFile) {
                cerr << "Error: Unable to open input file " << input << endl;
                return;
            }
            string line;
            while (getline(inputFile, line) && static_cast<int>(lines.size()) < count) {
                lines.push_back(line);
            }
            inputFile.close();
        }

        if (output.empty()) {
            for (const string& l : lines) {
                cout << l << endl;
            }
        } else {
            ofstream outputFile(output);
            if (!outputFile) {
                cerr << "Error: Unable to open output file " << output << endl;
                return;
            }
            for (const string& l : lines) {
                outputFile << l << endl;
            }
            outputFile.close();
        }
    }
};

unique_ptr<Command> CommandFactory(const string& command, const string& commandName, const string& option, const string& input, const string& output) {
    if (commandName == "echo") {
        if (!option.empty()) {
            size_t position = command.find(option);
            vector<int> errorPositions(option.length());
            for (size_t i = 0; i < option.length(); ++i) {
                errorPositions[i] = static_cast<int>(position + i);
            }

            ErrorHandler::displayError(command, errorPositions, "Command Echo does not allow an option.");
            return nullptr;
        }
        if (input.empty()) {
            cerr << "Error: No file name provided." << endl;
            return nullptr;
        }
        return make_unique<Echo>(input, output);

    } else if (commandName == "prompt") {
        if (!option.empty() || !output.empty()) {
            string invalidArgument = !option.empty() ? option : output;
            size_t position = command.find(invalidArgument);
            vector<int> errorPositions(invalidArgument.length());
            for (size_t i = 0; i < invalidArgument.length(); ++i) {
                errorPositions[i] = static_cast<int>(position + i);
            }
            ErrorHandler::displayError(command, errorPositions, "Command Prompt does not allow options or outputs.");
            return nullptr;
        }
        if (input.empty() || input.front() != '"' || input.back() != '"') {
            size_t position = command.find(input);
            vector<int> errorPositions(input.length());
            for (size_t i = 0; i < input.length(); ++i) {
                errorPositions[i] = static_cast<int>(position + i);
            }

            ErrorHandler::displayError(command, errorPositions, "Command Prompt requires input enclosed in quotes.");
            return nullptr;
        }
        return make_unique<Prompt>(input);

    } else if (commandName == "time") {
        if (!option.empty() || !input.empty()) {
            string invalidArgument = !option.empty() ? option : input;
            size_t position = command.find(invalidArgument);
            vector<int> errorPositions(invalidArgument.length());
            for (size_t i = 0; i < invalidArgument.length(); ++i) {
                errorPositions[i] = static_cast<int>(position + i);
            }
            ErrorHandler::displayError(command, errorPositions, "Command Time does not allow options or inputs.");
            return nullptr;
        }
        return make_unique<Time>(output);
    } else if (commandName == "date") {
        if (!option.empty() || !input.empty()) {
            string invalidArgument = !option.empty() ? option : input;
            size_t position = command.find(invalidArgument);
            vector<int> errorPositions(invalidArgument.length());
            for (size_t i = 0; i < invalidArgument.length(); ++i) {
                errorPositions[i] = static_cast<int>(position + i);
            }
            ErrorHandler::displayError(command, errorPositions, "Command Date does not allow options or inputs.");
            return nullptr;
        }
        return make_unique<Date>(output);
        } else if (commandName == "touch") {
            if (!option.empty() || !output.empty()) {
                string invalidArgument = !option.empty() ? option : output;
                size_t position = command.find(invalidArgument);
                vector<int> errorPositions(invalidArgument.length());
                for (size_t i = 0; i < invalidArgument.length(); ++i) {
                    errorPositions[i] = static_cast<int>(position + i);
                }
                ErrorHandler::displayError(command, errorPositions, "Command Touch only allows a filename.");
                return nullptr;
            }
            if (input.front() == '"' || input.back() == '"') {
                size_t position = command.find(input);
                vector<int> errorPositions(input.length());
                for (size_t i = 0; i < input.length(); ++i) {
                    errorPositions[i] = static_cast<int>(position + i);
                }

                ErrorHandler::displayError(command, errorPositions, "Command Touch requires a filename without quotes.");
                return nullptr;
            }
            if (input.empty()) {
                cerr << "Error: No file name provided." << endl;
                return nullptr;
            }
            return make_unique<Touch>(input);
        } else if (commandName == "rm") {
            if (!option.empty() || !output.empty()) {
                string invalidArgument = !option.empty() ? option : output;
                size_t position = command.find(invalidArgument);
                vector<int> errorPositions(invalidArgument.length());
                for (size_t i = 0; i < invalidArgument.length(); ++i) {
                    errorPositions[i] = static_cast<int>(position + i);
                }
                ErrorHandler::displayError(command, errorPositions, "Command Rm only allows a filename.");
                return nullptr;
            }
            if (input.front() == '"' || input.back() == '"') {
                size_t position = command.find(input);
                vector<int> errorPositions(input.length());
                for (size_t i = 0; i < input.length(); ++i) {
                    errorPositions[i] = static_cast<int>(position + i);
                }

                ErrorHandler::displayError(command, errorPositions, "Command Rm requires a filename without quotes.");
                return nullptr;
            }
            if (input.empty()) {
                cerr << "Error: No file name provided." << endl;
                return nullptr;
            }
            return make_unique<Rm>(input);
        } else if (commandName == "truncate") {
            if (!option.empty() || !output.empty()) {
                string invalidArgument = !option.empty() ? option : output;
                size_t position = command.find(invalidArgument);                //ovaj deo koda se ponavlja za svaku komandu, napravi funkciju
                vector<int> errorPositions(invalidArgument.length());
                for (size_t i = 0; i < invalidArgument.length(); ++i) {
                    errorPositions[i] = static_cast<int>(position + i);
                }
                ErrorHandler::displayError(command, errorPositions, "Command Truncate only allows a filename.");
                return nullptr;
            }
            if (input.front() == '"' || input.back() == '"') {
                size_t position = command.find(input);
                vector<int> errorPositions(input.length());
                for (size_t i = 0; i < input.length(); ++i) {
                    errorPositions[i] = static_cast<int>(position + i);
                }

                ErrorHandler::displayError(command, errorPositions, "Command Truncate requires a filename without quotes.");
                return nullptr;
            }
            if (input.empty()) {
                cerr << "Error: No file name provided." << endl;
                return nullptr;
            }
            return make_unique<Truncate>(input);
        } else if (commandName == "wc") {
            if (option.empty()) {
                cerr << "Error: No option provided." << endl;
            }
            else if (option != "-w" && option != "-c") {
                size_t position = command.find(output);
                vector<int> errorPositions(input.length());
                for (size_t i = 0; i < input.length(); ++i) {
                    errorPositions[i] = static_cast<int>(position + i);
                }
                ErrorHandler::displayError(command, errorPositions, "Error: Unrecognized option .");
                return nullptr;
            }
            if (input.empty()) {
                cerr << "Error: No file name provided." << endl;
                return nullptr;
            }
            return make_unique<Wc>(option, input, output);
        } else if (commandName == "head") {
            if (option.empty()) {
                cerr << "Error: No option provided." << endl;
                return nullptr;
            }
            if (input.empty()) {
                cerr << "Error: No input provided." << endl;
                return nullptr;
            }
            return make_unique<Head>(option, input, output);
        }
    cerr << "Unknown command: " << commandName << endl;
    return nullptr;
}
int main() {
    ErrorHandler error_handler;
    vector <string> tokens(4);
    while (true) {
        error_handler.clear();
        string inputLine;
        this_thread::sleep_for(chrono::milliseconds(200));
        cout << currentPrompt;
        getline(cin, inputLine);

        if (inputLine == "exit") {
            return 0;
        }

        if (!error_handler.ValidateCommand(inputLine)) {;
            continue;
        } else {
            tokens = error_handler.tokenize(inputLine);
        }
        unique_ptr<Command> cmd = CommandFactory(inputLine, tokens[0], tokens[1], tokens[2], tokens[3]);
        if (cmd) {
            cmd->execute();
        }
    }
}