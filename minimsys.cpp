#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::filesystem::path exePath = std::filesystem::absolute(argv[0]);
    std::filesystem::path exeDir = exePath.parent_path(); // dossier de l'exe

    std::vector<std::string> packages;
    bool capture = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-S") {
            capture = true;
            continue;
        }
        if (capture) {
            packages.push_back(arg);
        }
    }

    if (packages.empty()) {
        std::cerr << "Usage: minimsys -S <package1> <package2> ...\n";
        return 1;
    }

    // chemin complet vers le python de la venv
    std::filesystem::path pythonVenv = exeDir / ".venv" / "Scripts" / "python.exe";
    // chemin complet vers main.py
    std::filesystem::path mainPy = exeDir / "main.py";

    // construction de la commande SANS guillemets
    std::string cmd = pythonVenv.string() + " " + mainPy.string() + " -S ";
    for (const auto& p : packages) {
        cmd += p + " ";
    }

    cmd += "-o " + std::filesystem::current_path().string();

    std::cout << "Exécution : " << cmd << std::endl;

    int ret = std::system(cmd.c_str());

    return ret;
}