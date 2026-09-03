#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <iostream>
#include <string>
#include <cstdio>
#include <filesystem>

#include <curl/curl.h>

#include "colors.h"

// ============================================================
// CURL : Write File Callback
// ============================================================

inline size_t WriteFileCallback(
    void* contents,
    size_t size,
    size_t nmemb,
    void* userp)
{
    FILE* file =
        static_cast<FILE*>(userp);

    return fwrite(
        contents,
        size,
        nmemb,
        file
    );
}

// ============================================================
// Demander confirmation
// ============================================================

inline bool askDownloadConfirmation(
    const std::string& package)
{
    std::cout
        << "\n"
        << Colors::Cyan
        << Colors::Bold
        << "Download"
        << Colors::Reset
        << ": Do you want to download "
        << Colors::Green
        << package
        << Colors::Reset
        << "? [y/n] ";

    std::string answer;

    std::getline(
        std::cin,
        answer
    );

    return
        answer == "y" ||
        answer == "Y" ||
        answer == "yes" ||
        answer == "Yes" ||
        answer == "YES";
}

// ============================================================
// Télécharger un package
// ============================================================

inline bool downloadPackage(
    const std::string& package,
    const std::string& url,
    const std::string& fileName)
{
    // ========================================================
    // Dossier courant
    // ========================================================

    std::filesystem::path currentDirectory =
        std::filesystem::current_path();

    // ========================================================
    // Dossier cache
    // ========================================================

    std::filesystem::path cacheDirectory =
        currentDirectory / "cache";

    // ========================================================
    // Créer le dossier cache
    // ========================================================

    try
    {
        if (!std::filesystem::exists(cacheDirectory))
        {
            std::filesystem::create_directories(
                cacheDirectory
            );

            std::cout
                << Colors::Green
                << "Created cache directory: "
                << Colors::Reset
                << cacheDirectory.string()
                << "\n";
        }
    }
    catch (const std::filesystem::filesystem_error& error)
    {
        std::cerr
            << Colors::Red
            << Colors::Bold
            << "ERROR"
            << Colors::Reset
            << ": Cannot create cache directory.\n"
            << error.what()
            << "\n";

        return false;
    }

    // ========================================================
    // Chemin du fichier
    // ========================================================

    std::filesystem::path outputPath =
        cacheDirectory / fileName;

    std::string outputFile =
        outputPath.string();

    // ========================================================
    // Vérifier si le package existe déjà
    // ========================================================

    if (std::filesystem::exists(outputPath))
    {
        std::cout
            << "\n"
            << Colors::Yellow
            << "WARNING"
            << Colors::Reset
            << ": Package already exists in cache.\n";

        std::cout
            << "File: "
            << outputFile
            << "\n";

        return true;
    }

    // ========================================================
    // Initialiser CURL
    // ========================================================

    CURL* curl =
        curl_easy_init();

    if (!curl)
    {
        std::cerr
            << Colors::Red
            << Colors::Bold
            << "ERROR"
            << Colors::Reset
            << ": Failed to initialize CURL.\n";

        return false;
    }

    // ========================================================
    // Créer le fichier
    // ========================================================

    FILE* file =
        fopen(
            outputFile.c_str(),
            "wb"
        );

    if (!file)
    {
        std::cerr
            << Colors::Red
            << Colors::Bold
            << "ERROR"
            << Colors::Reset
            << ": Cannot create file:\n"
            << outputFile
            << "\n";

        curl_easy_cleanup(curl);

        return false;
    }

    // ========================================================
    // Configuration CURL
    // ========================================================

    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        url.c_str()
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        WriteFileCallback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        file
    );

    curl_easy_setopt(
        curl,
        CURLOPT_FOLLOWLOCATION,
        1L
    );

    curl_easy_setopt(
        curl,
        CURLOPT_USERAGENT,
        "minimsys/0.2.0"
    );

    curl_easy_setopt(
        curl,
        CURLOPT_TIMEOUT,
        120L
    );

    // ========================================================
    // Affichage
    // ========================================================

    std::cout
        << "\n"
        << Colors::Cyan
        << Colors::Bold
        << "Downloading"
        << Colors::Reset
        << ": "
        << package
        << "\n";

    std::cout
        << Colors::Dim
        << "URL: "
        << url
        << Colors::Reset
        << "\n";

    std::cout
        << Colors::Dim
        << "Destination: "
        << outputFile
        << Colors::Reset
        << "\n";

    // ========================================================
    // Télécharger
    // ========================================================

    CURLcode result =
        curl_easy_perform(curl);

    // ========================================================
    // Récupérer le code HTTP
    // ========================================================

    long httpCode = 0;

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &httpCode
    );

    // ========================================================
    // Fermer le fichier
    // ========================================================

    fclose(file);

    // ========================================================
    // Nettoyer CURL
    // ========================================================

    curl_easy_cleanup(curl);

    // ========================================================
    // Erreur CURL
    // ========================================================

    if (result != CURLE_OK)
    {
        std::remove(
            outputFile.c_str()
        );

        std::cerr
            << Colors::Red
            << Colors::Bold
            << "ERROR"
            << Colors::Reset
            << ": Download failed: "
            << curl_easy_strerror(result)
            << "\n";

        return false;
    }

    // ========================================================
    // Erreur HTTP
    // ========================================================

    if (httpCode != 200)
    {
        std::remove(
            outputFile.c_str()
        );

        std::cerr
            << Colors::Red
            << Colors::Bold
            << "ERROR"
            << Colors::Reset
            << ": HTTP "
            << httpCode
            << "\n";

        return false;
    }

    // ========================================================
    // Succès
    // ========================================================

    std::cout
        << "\n"
        << Colors::Green
        << Colors::Bold
        << "OK"
        << Colors::Reset
        << ": Download completed.\n";

    std::cout
        << "File: "
        << outputFile
        << "\n";

    return true;
}

#endif