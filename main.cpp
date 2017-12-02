#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

// Terminal color codes.
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

// Replaces all occurence of a substring by another substring.
std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;

    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return str;
}

// Launches an application by forking the current process and changing the command.
void launch(char** args, std::string input, std::string output) {
    pid_t pid, wpid;
    int status;
    pid = fork();

    // The forked process ends up here.
    if (pid == 0) {
        if (input != "") {
            freopen(input.c_str(), "r", stdin);
        }

        if (output != "") {
            freopen(output.c_str(), "w", stdout);
        }

        if (execvp(args[0], args) == -1) {
            perror("mussels");
            _exit(EXIT_FAILURE);
        }
    // An error occured with the fork.
    } else if (pid < 0) {
        perror("mussels");
    // The parent process ends up here.
    } else {
        // Waits for the forked process to finish, if not in background.
        if (output != "/dev/null") {
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
}

// Main loop of the shell. Handles commands.
void loop() {
    std::vector<std::string> history;

    while (true) {
        // Print the input prompt.
        std::cout << "(" << KCYN << getenv("PWD") << KNRM << ")" << std::endl;
        std::cout << "> " << KNRM;

        // Get the input.
        std::string input;
        std::getline(std::cin, input);

        // Do input replacements.
        input = replace_all(input, std::string("~"), getenv("HOME"));
        input = replace_all(input, std::string("!!"), history.back());
        input = replace_all(input, std::string("$PWD"), getenv("PWD"));
        input = replace_all(input, std::string("$HOME"), getenv("HOME"));
        input = replace_all(input, std::string("$PATH"), getenv("PATH"));

        // Add to global history.
        history.push_back(input);

        // Parse the input into args.
        std::istringstream iss(input);
        std::vector<std::string> tokens;
        std::copy(std::istream_iterator<std::string>(iss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(tokens));

        std::string command = tokens[0];
        if (command == "cd") {
            if (chdir(tokens[1].c_str()) != 0) {
                perror("mussels");
            } else {
                setenv("PWD", get_current_dir_name(), 1);
            }
        } else if (command == "history") {
            for (auto previous : history) {
                std::cout << previous << std::endl;
            }
        } else if (command == "exit") {
            std::cout << "Bye!" << std::endl;
            exit(EXIT_SUCCESS);
        } else if (command == "help") {
            std::cout << "Mussels is an Unbelievingly Slim and Simple - Even Limiting - Shell" << std::endl;
            std::cout << "For help on a specific command, use the 'man' command." << std::endl;
        } else {
            std::string input;
            std::string output;

            // Convert the C++ strings into C *char.
            char* argv[tokens.size()];
            int argc = 0;
            for (int x = 0; x < tokens.size(); x++) {
                if (tokens[x] == "&") {
                    output = "/dev/null";
                    break;
                } else if (tokens[x] == ">") {
                    output = tokens[x+1];
                    x++;
                } else if (tokens[x] == "<") {
                    input = tokens[x+1];
                    x++;
                } else {
                    argv[argc++] = strdup(tokens[x].c_str());
                }
            }
            argv[argc++] = 0;

            // Launch the command.
            launch(argv, input, output);
        }
    }
}

int main(int argc, char** argv) {
    loop();

    return EXIT_SUCCESS;
}
