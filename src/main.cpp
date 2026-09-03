#include <iostream>
#include <string>

#include "version.h"
#include "help.h"
#include "search.h"
#include "search_packages.h"

int main(int argc, char* argv[])
{
    // ========================================================
    // Aucun argument
    // ========================================================

    if (argc < 2)
    {
        printHelp(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    // ========================================================
    // --version
    // ========================================================

    if (command == "--version")
    {
        printVersion();
        return 0;
    }

    // ========================================================
    // -h / --help
    // ========================================================

    if (command == "-h" ||
        command == "--help")
    {
        printHelp(argv[0]);
        return 0;
    }

    // ========================================================
    // -Ss : recherche de packages
    // ========================================================

    if (command == "-Ss")
    {
        if (argc < 3)
        {
            std::cerr
                << "Erreur : -Ss necessite une recherche.\n\n";

            std::cout
                << "Exemple :\n"
                << "  "
                << argv[0]
                << " -Ss gcc\n";

            return 1;
        }

        std::string query = argv[2];

        return searchPackages(query);
    }

    // ========================================================
    // -S : informations et téléchargement d'un package
    // ========================================================

    if (command == "-S")
    {
        if (argc < 3)
        {
            std::cerr
                << "Erreur : -S necessite un package.\n\n";

            std::cout
                << "Exemple :\n"
                << "  "
                << argv[0]
                << " -S mingw-w64-x86_64-gcc\n";

            return 1;
        }

        std::string package = argv[2];

        DownloadMode mode = DownloadMode::Ask;

        // ----------------------------------------------------
        // -y : telecharge le package + toutes les dependances
        //      sans demander de confirmation
        // -n : telecharge uniquement le package (sans les
        //      dependances), sans demander de confirmation
        // ----------------------------------------------------

        if (argc >= 4)
        {
            std::string flag = argv[3];

            if (flag == "-y")
            {
                mode = DownloadMode::Yes;
            }
            else if (flag == "-n")
            {
                mode = DownloadMode::No;
            }
            else
            {
                std::cerr
                    << "Erreur : option inconnue : "
                    << flag
                    << "\n\n";

                printHelp(argv[0]);

                return 1;
            }
        }

        return searchPackage(package, mode);
    }

    // ========================================================
    // Commande inconnue
    // ========================================================

    std::cerr
        << "Erreur : option inconnue : "
        << command
        << "\n\n";

    printHelp(argv[0]);

    return 1;
}