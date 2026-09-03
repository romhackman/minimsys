#ifndef BASE_SEARCH_H
#define BASE_SEARCH_H

#include <iostream>
#include <string>
#include <regex>
#include <curl/curl.h>

#include "colors.h"

// ============================================================
// CURL : Write Callback
// ============================================================

inline size_t BaseWriteCallback(
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
// HTTP GET
// ============================================================

inline bool fetchBaseUrl(
const std::string& url,
std::string& html,
long& httpCode)
{
CURL* curl = curl_easy_init();

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


// --------------------------------------------------------
// CURL configuration
// --------------------------------------------------------

curl_easy_setopt(
    curl,
    CURLOPT_URL,
    url.c_str()
);

curl_easy_setopt(
    curl,
    CURLOPT_WRITEFUNCTION,
    BaseWriteCallback
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


// --------------------------------------------------------
// HTTP request
// --------------------------------------------------------

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


// --------------------------------------------------------
// HTTP status code
// --------------------------------------------------------

curl_easy_getinfo(
    curl,
    CURLINFO_RESPONSE_CODE,
    &httpCode
);


curl_easy_cleanup(curl);

return true;


}

// ============================================================
// Get Base Package
//
// Examples:
//
// mingw-w64-x86_64-SDL2_ttf
// |
// +----> mingw-w64-x86_64-SDL2
//
// foo_bar
// |
// +----> foo
//
// ============================================================

inline std::string getBasePackage(
const std::string& package)
{
std::string basePackage = package;

std::size_t pos =
    basePackage.find_last_of('_');


if (pos != std::string::npos)
{
    basePackage =
        basePackage.substr(0, pos);
}


return basePackage;


}

// ============================================================
// Search Base Package
// ============================================================

inline void searchBasePackage(
const std::string& package)
{
// --------------------------------------------------------
// Determine Base Package
// --------------------------------------------------------

std::string basePackage =
    getBasePackage(package);


// --------------------------------------------------------
// Base Package URL
// --------------------------------------------------------

std::string baseUrl =
    "https://packages.msys2.org/base/" +
    basePackage;


std::cout
    << "\n"
    << Colors::Cyan
    << Colors::Bold
    << "INFO"
    << Colors::Reset
    << ": Searching Base Package\n";


std::cout
    << "  "
    << Colors::Dim
    << "Package: "
    << basePackage
    << Colors::Reset
    << "\n";


std::cout
    << "  "
    << Colors::Dim
    << "URL: "
    << baseUrl
    << Colors::Reset
    << "\n";


std::string html;
long httpCode = 0;


// ========================================================
// HTTP request
// ========================================================

if (!fetchBaseUrl(
        baseUrl,
        html,
        httpCode))
{
    return;
}


// ========================================================
// HTTP status
// ========================================================

std::cout
    << "  HTTP status: ";


if (httpCode == 200)
{
    std::cout
        << Colors::Green
        << httpCode
        << Colors::Reset
        << "\n";
}
else
{
    std::cout
        << Colors::Red
        << httpCode
        << Colors::Reset
        << "\n";
}


// ========================================================
// Base Package not found
// ========================================================

if (httpCode == 404)
{
    std::cout
        << "\n"
        << Colors::Red
        << Colors::Bold
        << "ERROR"
        << Colors::Reset
        << ": Base Package not found.\n";

    return;
}


// ========================================================
// Other HTTP error
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

    return;
}


// ========================================================
// Find Binary Packages section
// ========================================================

std::regex binaryPackagesRegex(
    R"(Binary Packages:\s*</dt>\s*<dd\s+class=["']col-sm-9["'][^>]*>([\s\S]*?)</dd>\s*</dl>)",
    std::regex::icase
);


std::smatch binaryMatch;


if (!std::regex_search(
        html,
        binaryMatch,
        binaryPackagesRegex))
{
    std::cout
        << "\n"
        << Colors::Yellow
        << "WARNING"
        << Colors::Reset
        << ": Binary Packages section not found.\n";

    return;
}


std::string binaryHtml =
    binaryMatch[1].str();


// ========================================================
// Extract environments and packages
// ========================================================

std::regex environmentRegex(
    R"(<dt\s+class=["']text-muted\s+small["'][^>]*>\s*([^<]+?)\s*</dt>\s*<dd[^>]*>[\s\S]*?<a\s+href=["']([^"']+)["'][^>]*>\s*([^<]+?)\s*</a>)",
    std::regex::icase
);


auto begin =
    std::sregex_iterator(
        binaryHtml.begin(),
        binaryHtml.end(),
        environmentRegex
    );


auto end =
    std::sregex_iterator();


int count = 0;


// ========================================================
// Display packages
// ========================================================

std::cout
    << "\n"
    << Colors::Blue
    << Colors::Bold
    << "Available Binary Packages"
    << Colors::Reset
    << "\n\n";


for (auto it = begin;
     it != end;
     ++it)
{
    std::string environment =
        (*it)[1].str();


    std::string packageUrl =
        (*it)[2].str();


    std::string packageName =
        (*it)[3].str();


    // ----------------------------------------------------
    // Trim whitespace
    // ----------------------------------------------------

    environment = std::regex_replace(
        environment,
        std::regex(R"(^\s+|\s+$)"),
        ""
    );


    packageName = std::regex_replace(
        packageName,
        std::regex(R"(^\s+|\s+$)"),
        ""
    );


    if (environment.empty() ||
        packageName.empty())
    {
        continue;
    }


    // ----------------------------------------------------
    // Display environment
    // ----------------------------------------------------

    std::cout
        << "  "
        << Colors::Yellow
        << "["
        << environment
        << "]"
        << Colors::Reset
        << "\n";


    // ----------------------------------------------------
    // Display package
    // ----------------------------------------------------

    std::cout
        << "    "
        << Colors::Green
        << packageName
        << Colors::Reset
        << "\n";


    count++;
}


// ========================================================
// No packages
// ========================================================

if (count == 0)
{
    std::cout
        << Colors::Yellow
        << "WARNING"
        << Colors::Reset
        << ": No binary packages found.\n";

    return;
}


// ========================================================
// Total
// ========================================================

std::cout
    << "\n"
    << Colors::Dim
    << "Total: "
    << count
    << " package(s)"
    << Colors::Reset
    << "\n";


}

#endif