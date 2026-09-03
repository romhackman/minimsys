#ifndef SEARCH_PACKAGES_H
#define SEARCH_PACKAGES_H

#include <iostream>
#include <string>
#include <regex>
#include <curl/curl.h>

// ============================================================
// CURL : récupérer le contenu HTML
// ============================================================

inline size_t SearchWriteCallback(
    void* contents,
    size_t size,
    size_t nmemb,
    void* userp)
{
    size_t totalSize = size * nmemb;

    std::string* html =
        static_cast<std::string*>(userp);

    html->append(
        static_cast<char*>(contents),
        totalSize
    );

    return totalSize;
}


// ============================================================
// Recherche de packages avec -Ss
// ============================================================

inline int searchPackages(const std::string& query)
{
    // --------------------------------------------------------
    // URL de recherche MSYS2
    // --------------------------------------------------------

    std::string url =
        "https://packages.msys2.org/search?q=" + query;


    std::cout
        << "Recherche : "
        << query
        << "\n";

    std::cout
        << "URL : "
        << url
        << "\n\n";


    // --------------------------------------------------------
    // Initialisation CURL
    // --------------------------------------------------------

    CURL* curl = curl_easy_init();

    if (!curl)
    {
        std::cerr
            << "Erreur : impossible d'initialiser CURL.\n";

        return 1;
    }


    std::string html;


    // --------------------------------------------------------
    // Configuration CURL
    // --------------------------------------------------------

    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        url.c_str()
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        SearchWriteCallback
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


    // --------------------------------------------------------
    // Requête HTTP
    // --------------------------------------------------------

    CURLcode result =
        curl_easy_perform(curl);


    if (result != CURLE_OK)
    {
        std::cerr
            << "Erreur CURL : "
            << curl_easy_strerror(result)
            << "\n";

        curl_easy_cleanup(curl);

        return 1;
    }


    // --------------------------------------------------------
    // Code HTTP
    // --------------------------------------------------------

    long httpCode = 0;

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &httpCode
    );


    curl_easy_cleanup(curl);


    if (httpCode != 200)
    {
        std::cerr
            << "Erreur HTTP : "
            << httpCode
            << "\n";

        return 1;
    }


    // --------------------------------------------------------
    // Recherche des lignes du tableau
    //
    // <tr>
    //   <td><a href="...">gcc</a></td>
    //   <td>15.3.0-1</td>
    //   <td>The GNU Compiler Collection</td>
    // </tr>
    // --------------------------------------------------------

    std::regex packageRegex(
        R"(<tr>\s*<td>\s*<a\s+href=["'][^"']+["'][^>]*>\s*([^<]+?)\s*</a>\s*</td>\s*<td>\s*([^<]+?)\s*</td>\s*<td>\s*([^<]+?)\s*</td>\s*</tr>)",
        std::regex::icase
    );


    auto begin =
        std::sregex_iterator(
            html.begin(),
            html.end(),
            packageRegex
        );


    auto end =
        std::sregex_iterator();


    int count = 0;


    // --------------------------------------------------------
    // Affichage des résultats
    // --------------------------------------------------------

    for (auto it = begin; it != end; ++it)
    {
        std::string package =
            (*it)[1].str();

        std::string version =
            (*it)[2].str();

        std::string description =
            (*it)[3].str();


        // Nettoyage
        package = std::regex_replace(
            package,
            std::regex(R"(^\s+|\s+$)"),
            ""
        );

        version = std::regex_replace(
            version,
            std::regex(R"(^\s+|\s+$)"),
            ""
        );

        description = std::regex_replace(
            description,
            std::regex(R"(^\s+|\s+$)"),
            ""
        );


        std::cout
            << "  "
            << package
            << "\n";

        std::cout
            << "    Version : "
            << version
            << "\n";

        std::cout
            << "    Description : "
            << description
            << "\n\n";


        count++;
    }


    // --------------------------------------------------------
    // Aucun résultat
    // --------------------------------------------------------

    if (count == 0)
    {
        std::cout
            << "Aucun package trouve pour : "
            << query
            << "\n";

        return 0;
    }


    // --------------------------------------------------------
    // Total
    // --------------------------------------------------------

    std::cout
        << "Total : "
        << count
        << " packages\n";


    return 0;
}

#endif
