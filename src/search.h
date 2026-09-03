#ifndef SEARCH_H
#define SEARCH_H

#include <iostream>
#include <string>
#include <regex>
#include <vector>

#include <curl/curl.h>

#include "colors.h"
#include "base_search.h"
#include "download.h"
#include "packages_json.h"

// ============================================================
// Mode de téléchargement
//
// Ask : demande confirmation (comportement par défaut)
// Yes : télécharge le package + toutes ses dépendances,
//       sans demander de confirmation
// No  : télécharge uniquement le package, sans les
//       dépendances, sans demander de confirmation
// ============================================================

enum class DownloadMode
{
    Ask,
    Yes,
    No
};

// ============================================================
// CURL : Write Callback
// ============================================================

inline size_t WriteCallback(
    void* contents,
    size_t size,
    size_t nmemb,
    void* userp)
{
    size_t totalSize =
        size * nmemb;

    std::string* html =
        static_cast<std::string*>(userp);

    html->append(
        static_cast<char*>(contents),
        totalSize
    );

    return totalSize;
}

// ============================================================
// HTTP GET
// ============================================================

inline bool fetchUrl(
    const std::string& url,
    std::string& html,
    long& httpCode)
{
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

    html.clear();

    httpCode = 0;

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
        WriteCallback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &html
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
        15L
    );

    // ========================================================
    // Requête HTTP
    // ========================================================

    CURLcode result =
        curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        std::cerr
            << Colors::Red
            << Colors::Bold
            << "ERROR"
            << Colors::Reset
            << ": CURL request failed: "
            << curl_easy_strerror(result)
            << "\n";

        curl_easy_cleanup(curl);

        return false;
    }

    // ========================================================
    // Code HTTP
    // ========================================================

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &httpCode
    );

    curl_easy_cleanup(curl);

    return true;
}

// ============================================================
// Trim
// ============================================================

inline std::string trimString(
    std::string value)
{
    value =
        std::regex_replace(
            value,
            std::regex(R"(^\s+|\s+$)"),
            ""
        );

    return value;
}

// ============================================================
// Trouver le lien du package
// ============================================================

inline bool findDownloadLink(
    const std::string& html,
    std::string& downloadUrl,
    std::string& fileName)
{
    // ========================================================
    // Recherche d'un fichier .pkg.tar.zst
    // ========================================================

    std::regex downloadRegex(
        R"(href=["']([^"']*\.pkg\.tar\.zst)["'])",
        std::regex::icase
    );

    std::smatch match;

    if (!std::regex_search(
            html,
            match,
            downloadRegex))
    {
        return false;
    }

    if (match.size() < 2)
    {
        return false;
    }

    std::string link =
        match[1].str();

    // ========================================================
    // Nom du fichier
    // ========================================================

    std::size_t position =
        link.find_last_of('/');

    if (position != std::string::npos)
    {
        fileName =
            link.substr(
                position + 1
            );
    }
    else
    {
        fileName = link;
    }

    // ========================================================
    // Construire l'URL complète
    // ========================================================

    if (
        link.rfind("https://", 0) == 0 ||
        link.rfind("http://", 0) == 0
    )
    {
        downloadUrl = link;
    }
    else if (
        link.rfind("//", 0) == 0
    )
    {
        downloadUrl =
            "https:" + link;
    }
    else if (
        link.rfind("/", 0) == 0
    )
    {
        downloadUrl =
            "https://packages.msys2.org" +
            link;
    }
    else
    {
        downloadUrl =
            "https://packages.msys2.org/" +
            link;
    }

    return true;
}

// ============================================================
// Télécharger une dépendance
// (utilisé en mode -y)
// ============================================================

inline bool downloadDependency(
    const std::string& depName)
{
    std::string url =
        "https://packages.msys2.org/packages/" +
        depName;

    std::cout
        << "\n"
        << Colors::Blue
        << "-> "
        << Colors::Reset
        << depName
        << "\n";

    std::string html;
    long httpCode = 0;

    if (!fetchUrl(
            url,
            html,
            httpCode))
    {
        return false;
    }

    if (httpCode != 200)
    {
        std::cout
            << "  "
            << Colors::Yellow
            << "WARNING"
            << Colors::Reset
            << ": HTTP "
            << httpCode
            << ", dependance ignoree.\n";

        return false;
    }

    std::string downloadUrl;
    std::string fileName;

    if (!findDownloadLink(
            html,
            downloadUrl,
            fileName))
    {
        std::cout
            << "  "
            << Colors::Yellow
            << "WARNING"
            << Colors::Reset
            << ": Lien de telechargement introuvable, dependance ignoree.\n";

        return false;
    }

    if (!downloadPackage(
            depName,
            downloadUrl,
            fileName))
    {
        return false;
    }

    logInstalledPackage(
        fileName,
        downloadUrl
    );

    return true;
}

// ============================================================
// Search Package
// ============================================================

inline int searchPackage(
    const std::string& package,
    DownloadMode mode = DownloadMode::Ask)
{
    // ========================================================
    // URL du package
    // ========================================================

    std::string url =
        "https://packages.msys2.org/packages/" +
        package;

    std::cout
        << Colors::Cyan
        << Colors::Bold
        << "URL"
        << Colors::Reset
        << ": "
        << url
        << "\n";

    // ========================================================
    // Requête HTTP
    // ========================================================

    std::string html;

    long httpCode = 0;

    if (!fetchUrl(
            url,
            html,
            httpCode))
    {
        return 1;
    }

    // ========================================================
    // HTTP status
    // ========================================================

    std::cout
        << Colors::Cyan
        << Colors::Bold
        << "HTTP"
        << Colors::Reset
        << ": ";

    if (httpCode == 200)
    {
        std::cout
            << Colors::Green
            << httpCode
            << Colors::Reset
            << "\n";
    }
    else if (httpCode == 404)
    {
        std::cout
            << Colors::Red
            << httpCode
            << Colors::Reset
            << "\n";
    }
    else
    {
        std::cout
            << Colors::Yellow
            << httpCode
            << Colors::Reset
            << "\n";
    }

    // ========================================================
    // Package introuvable
    // ========================================================

    if (httpCode == 404)
    {
        std::cout
            << "\n"
            << Colors::Red
            << Colors::Bold
            << "ERROR"
            << Colors::Reset
            << ": Package not found.\n";

        searchBasePackage(package);

        return 1;
    }

    // ========================================================
    // Autre erreur HTTP
    // ========================================================

    if (httpCode != 200)
    {
        std::cout
            << "\n"
            << Colors::Red
            << Colors::Bold
            << "ERROR"
            << Colors::Reset
            << ": HTTP request failed with status "
            << httpCode
            << ".\n";

        return 1;
    }

    // ========================================================
    // Package trouvé
    // ========================================================

    std::cout
        << "\n"
        << Colors::Green
        << Colors::Bold
        << "OK"
        << Colors::Reset
        << ": Package found!\n";

    // ========================================================
    // Dependencies
    // ========================================================

    std::vector<std::string> dependencyNames;

    std::regex dependenciesRegex(
        R"(Dependencies:\s*</dt>\s*<dd\s+class=["']col-sm-9["'][^>]*>([\s\S]*?)</dd>)",
        std::regex::icase
    );

    std::smatch dependenciesMatch;

    if (!std::regex_search(
            html,
            dependenciesMatch,
            dependenciesRegex))
    {
        std::cout
            << "\n"
            << Colors::Yellow
            << "WARNING"
            << Colors::Reset
            << ": Dependencies section not found.\n";
    }
    else
    {
        std::string dependenciesHtml =
            dependenciesMatch[1].str();

        // ====================================================
        // Titre
        // ====================================================

        std::cout
            << "\n"
            << Colors::Blue
            << Colors::Bold
            << "Dependencies"
            << Colors::Reset
            << ":\n\n";

        // ====================================================
        // Regex dependencies
        // ====================================================

        std::regex dependencyRegex(
            R"(<li[^>]*>\s*<a\s+href=["'][^"']+["'][^>]*>\s*([^<]+?)\s*</a>\s*(?:<span\s+class=["']text-muted["'][^>]*>\s*([^<]*?)\s*</span>)?\s*</li>)",
            std::regex::icase
        );

        auto dependencyBegin =
            std::sregex_iterator(
                dependenciesHtml.begin(),
                dependenciesHtml.end(),
                dependencyRegex
            );

        auto dependencyEnd =
            std::sregex_iterator();

        int count = 0;

        // ====================================================
        // Afficher les dependencies
        // ====================================================

        for (
            auto it = dependencyBegin;
            it != dependencyEnd;
            ++it)
        {
            std::string dependency =
                (*it)[1].str();

            std::string version =
                (*it)[2].str();

            dependency =
                trimString(dependency);

            version =
                trimString(version);

            if (dependency.empty())
            {
                continue;
            }

            dependencyNames.push_back(dependency);

            std::cout
                << "  "
                << Colors::Green
                << "-"
                << Colors::Reset
                << " "
                << dependency;

            if (!version.empty())
            {
                std::cout
                    << " "
                    << Colors::Dim
                    << version
                    << Colors::Reset;
            }

            std::cout
                << "\n";

            count++;
        }

        // ====================================================
        // Total
        // ====================================================

        std::cout
            << "\n"
            << Colors::Dim
            << "Total: "
            << count
            << " dependency(ies)"
            << Colors::Reset
            << "\n";
    }

    // ========================================================
    // Chercher le lien de téléchargement
    // ========================================================

    std::string downloadUrl;
    std::string fileName;

    if (!findDownloadLink(
            html,
            downloadUrl,
            fileName))
    {
        std::cout
            << "\n"
            << Colors::Yellow
            << "WARNING"
            << Colors::Reset
            << ": Download link not found.\n";

        return 0;
    }

    // ========================================================
    // Informations téléchargement
    // ========================================================

    std::cout
        << "\n"
        << Colors::Blue
        << Colors::Bold
        << "Package"
        << Colors::Reset
        << ": "
        << fileName
        << "\n";

    // ========================================================
    // Confirmation
    //
    // - mode Ask : demande confirmation (comportement normal)
    // - mode Yes / No : pas de confirmation
    // ========================================================

    bool proceed = true;

    if (mode == DownloadMode::Ask)
    {
        proceed = askDownloadConfirmation(fileName);
    }

    if (!proceed)
    {
        std::cout
            << "\n"
            << Colors::Yellow
            << "Download cancelled."
            << Colors::Reset
            << "\n";

        return 0;
    }

    // ========================================================
    // Télécharger dans ./cache
    // ========================================================

    if (!downloadPackage(
            package,
            downloadUrl,
            fileName))
    {
        return 1;
    }

    // ========================================================
    // Log dans ./packages.json
    // ========================================================

    logInstalledPackage(
        fileName,
        downloadUrl
    );

    // ========================================================
    // Mode -y : télécharger aussi toutes les dépendances
    // ========================================================

    if (mode == DownloadMode::Yes &&
        !dependencyNames.empty())
    {
        std::cout
            << "\n"
            << Colors::Cyan
            << Colors::Bold
            << "INFO"
            << Colors::Reset
            << ": Downloading "
            << dependencyNames.size()
            << " dependency(ies)...\n";

        int depOk = 0;
        int depFail = 0;

        for (const auto& depName : dependencyNames)
        {
            if (downloadDependency(depName))
            {
                depOk++;
            }
            else
            {
                depFail++;
            }
        }

        std::cout
            << "\n"
            << Colors::Dim
            << "Dependencies downloaded: "
            << depOk
            << ", failed/skipped: "
            << depFail
            << Colors::Reset
            << "\n";
    }

    return 0;
}

#endif