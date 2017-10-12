#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void launch(char** args) {
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("mussels");
            _exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("mussels");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

void loop() {
    while (true) {
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);

        std::istringstream iss(input);
        std::vector<std::string> tokens;
        std::copy(std::istream_iterator<std::string>(iss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(tokens));

        std::string command = tokens[0];
        if (command == "cd") {
            setenv("PWD", "/home/finiks", 1);
            if (chdir("/home/finiks") != 0) {
                perror("mussels");
            }
        } else if (command == "exit") {
            std::cout << "Bye!" << std::endl;
            exit(EXIT_SUCCESS);
        } else if (command == "help") {
            std::cout << "** mussels **" << std::endl;
        } else {
            char* argv[tokens.size()];
            int argc = 0;
            for (int x = 0; x < tokens.size(); x++) {
                argv[argc++] = strdup(tokens[x].c_str());
            }
            argv[argc++] = 0;

            launch(argv);
        }
    }
}

int main(int argc, char** argv) {
    loop();

    return EXIT_SUCCESS;
}
