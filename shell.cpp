#include <bits/stdc++.h>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>

using namespace std;

// Function declarations for all built-in commands
int shell_cd(vector<string> args);
int shell_ls(vector<string> args);
int shell_mkdir(vector<string> args);
int shell_touch(vector<string> args);
int shell_rm(vector<string> args);
int shell_cp(vector<string> args);
int shell_mv(vector<string> args);
int shell_echo(vector<string> args);
int shell_cat(vector<string> args);
int shell_grep(vector<string> args);
int shell_help(vector<string> args);
int shell_exit(vector<string> args);
int shell_wait(vector<string> args);
int shell_clear(vector<string> args);

// Map to associate command strings with function calls
map<string, int (*)(vector<string>)> command_map = {
    {"cd", shell_cd},
    {"ls", shell_ls},
    {"mkdir", shell_mkdir},
    {"touch", shell_touch},
    {"rm", shell_rm},
    {"cp", shell_cp},
    {"mv", shell_mv},
    {"echo", shell_echo},
    {"cat", shell_cat},
    {"grep", shell_grep},
    {"help", shell_help},
    {"exit", shell_exit},
    {"wait", shell_wait},
    {"clear", shell_clear}
};

// Read a line from standard input
string read_line() {
    string input;
    getline(cin, input);
    return input;
}

// Split the input line into tokens
vector<string> split_line(const string& line) {
    vector<string> args;
    size_t start = 0, end = line.find(' ');
    while (end != string::npos) {
        args.push_back(line.substr(start, end - start));
        start = end + 1;
        end = line.find(' ', start);
    }
    args.push_back(line.substr(start));
    return args;
}

// Execute shell built-in or external command
int execute(vector<string> args) {
    if (args.empty()) {
        return 1; // No command entered
    }

    string command = args[0];
    args.erase(args.begin());

    if (command_map.count(command)) {
        return command_map[command](args);
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            vector<char*> c_args(args.size() + 2);
            c_args[0] = strdup(command.c_str());
            for (size_t i = 0; i < args.size(); i++) {
                c_args[i + 1] = strdup(args[i].c_str());
            }
            c_args[args.size() + 1] = NULL;

            execvp(c_args[0], c_args.data());
            cerr << "Command not found" << endl;
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            cerr << "Failed to fork" << endl;
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
    return 1;
}

// Command loop for shell input/output
void shell_loop() {
    string line;
    vector<string> args;
    int status;

    do {
        cout << "> ";
        line = read_line();
        args = split_line(line);
        status = execute(args);
    } while (status);
}

// Main entry point for the shell
int main() {
    shell_loop();
    return EXIT_SUCCESS;
}

// Implementation of built-in shell commands
int shell_cd(vector<string> args) {
    if (args.size() > 1) {
        cerr << "cd: too many arguments" << endl;
        return 1;
    }
    string dir = args.empty() ? getenv("HOME") : args[0];
    if (chdir(dir.c_str()) != 0) {
        perror("cd");
    }
    return 1;
}

int shell_ls(vector<string> args) {
    const char* path = args.empty() ? "." : args[0].c_str();
    DIR* dir = opendir(path);
    if (dir == nullptr) {
        perror("ls");
        return 1;
    }
    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] != '.') { // Skip hidden files by default
            cout << entry->d_name << ' ';
        }
    }
    cout << endl;
    closedir(dir);
    return 1;
}

int shell_mkdir(vector<string> args) {
    if (args.empty()) {
        cerr << "mkdir: missing operand" << endl;
        return 1;
    }
    for (const auto& dir : args) {
        if (mkdir(dir.c_str(), 0777) != 0) { // Permission bits are set to allow all actions
            perror("mkdir");
        }
    }
    return 1;
}

int shell_touch(vector<string> args) {
    if (args.empty()) {
        cerr << "touch: missing operand" << endl;
        return 1;
    }
    for (const auto& filename : args) {
        int fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0666);
        if (fd == -1) {
            perror("touch");
        } else {
            close(fd);
        }
    }
    return 1;
}

int shell_rm(vector<string> args) {
    if (args.empty()) {
        cerr << "rm: missing operand" << endl;
        return 1;
    }
    for (const auto& filename : args) {
        if (remove(filename.c_str()) != 0) {
            perror("rm");
        }
    }
    return 1;
}

int shell_cp(vector<string> args) {
    if (args.size() < 2) {
        cerr << "cp: missing source and destination files" << endl;
        return 1;
    }
    ifstream src(args[0], ios::binary);
    ofstream dst(args[1], ios::binary);
    
    if (!src) {
        perror("cp: source file");
        return 1;
    }
    if (!dst) {
        perror("cp: destination file");
        return 1;
    }
    
    dst << src.rdbuf();
    return 1;
}

int shell_mv(vector<string> args) {
    if (args.size() < 2) {
        cerr << "mv: missing source and destination files" << endl;
        return 1;
    }
    if (rename(args[0].c_str(), args[1].c_str()) != 0) {
        perror("mv");
    }
    return 1;
}

int shell_echo(vector<string> args) {
    for (const auto& arg : args) {
        cout << arg << " ";
    }
    cout << endl;
    return 1;
}

int shell_cat(vector<string> args) {
    if (args.empty()) {
        cerr << "cat: missing operand" << endl;
        return 1;
    }
    for (const auto& filename : args) {
        ifstream file(filename);
        if (!file) {
            perror(("cat: " + filename).c_str());
            continue;
        }
        cout << file.rdbuf();
    }
    return 1;
}

int shell_grep(vector<string> args) {
    if (args.size() < 2) {
        cerr << "grep: missing pattern and file" << endl;
        return 1;
    }
    string pattern = args[0];
    for (size_t i = 1; i < args.size(); ++i) {
        ifstream file(args[i]);
        if (!file) {
            perror(("grep: " + args[i]).c_str());
            continue;
        }
        string line;
        while (getline(file, line)) {
            if (line.find(pattern) != string::npos) {
                cout << line << endl;
            }
        }
    }
    return 1;
}

int shell_help(vector<string> args) {
    cout << "Custom Shell Help\n"
         << "Supported commands:\n";
    for (const auto& cmd : command_map) {
        cout << "  " << cmd.first << endl;
    }
    return 1;
}

int shell_exit(vector<string> args) {
    return 0;
}

int shell_wait(vector<string> args) {
    int status;
    while (wait(&status) > 0);
    return 1;
}

int shell_clear(vector<string> args) {
    cout << "\033[2J\033[1;1H"; // ANSI escape codes to clear screen and move cursor
    return 1;
}
