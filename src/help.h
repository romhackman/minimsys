#ifndef HELP_H
#define HELP_H

#include <iostream>

inline void printHelp(const char* program)
{
    std::cout
        << "minimsys v0.2.0\n"
        << "\n"
        << "Usage:\n"
        << "  " << program << " [option] [package]\n"
        << "\n"
        << "Options:\n"
        << "  -S <package>       Afficher les informations d'un package\n"
        << "  -S <package> -y    Telecharger le package + toutes ses dependances (sans confirmation)\n"
        << "  -S <package> -n    Telecharger uniquement le package, sans les dependances (sans confirmation)\n"
        << "  -Ss <query>        Rechercher des packages\n"
        << "  -h, --help         Afficher cette aide\n"
        << "  --version          Afficher la version\n"
        << "\n"
        << "Exemples:\n"
        << "  " << program << " -S mingw-w64-x86_64-gcc\n"
        << "  " << program << " -S mingw-w64-ucrt-x86_64-curl -y\n"
        << "  " << program << " -S mingw-w64-ucrt-x86_64-curl -n\n"
        << "  " << program << " -Ss gcc\n"
        << "  " << program << " -h\n"
        << "  " << program << " --version\n";
}

#endif
