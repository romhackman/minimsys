#ifndef PACKAGES_JSON_H
#define PACKAGES_JSON_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <filesystem>

#include "colors.h"

// ============================================================
// Date / heure courante (YYYY-MM-DD HH:MM:SS)
// ============================================================

inline std::string getCurrentDateTime()
{
std::time_t now =
    std::time(nullptr);

std::tm localTime{};

#if defined(_WIN32)
localtime_s(&localTime, &now);
#else
localtime_r(&now, &localTime);
#endif

char buffer[32];

std::strftime(
    buffer,
    sizeof(buffer),
    "%Y-%m-%d %H:%M:%S",
    &localTime
);

return std::string(buffer);


}

// ============================================================
// Échapper une chaîne pour du JSON
// ============================================================

inline std::string escapeJson(
const std::string& value)
{
std::string result;
result.reserve(value.size());

for (char c : value)
{
    switch (c)
    {
        case '"':  result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n";  break;
        case '\r': result += "\\r";  break;
        case '\t': result += "\\t";  break;
        default:   result += c;      break;
    }
}

return result;


}

// ============================================================
// Trim
// ============================================================

inline std::string trimJson(
std::string value)
{
std::size_t start =
    value.find_first_not_of(" \t\n\r");

std::size_t end =
    value.find_last_not_of(" \t\n\r");

if (start == std::string::npos)
{
    return std::string();
}

return value.substr(
    start,
    end - start + 1
);


}

// ============================================================
// Ajouter un package installé dans ./packages.json
//
// Format :
// [
//   {
//     "name": "...",
//     "url": "...",
//     "date": "..."
//   }
// ]
// ============================================================

inline void logInstalledPackage(
const std::string& name,
const std::string& url)
{
std::filesystem::path jsonPath =
    "./packages.json";

std::string content;


// --------------------------------------------------------
// Lire le fichier existant
// --------------------------------------------------------

if (std::filesystem::exists(jsonPath))
{
    std::ifstream inFile(jsonPath);

    std::stringstream buffer;

    buffer << inFile.rdbuf();

    content = buffer.str();
}


content = trimJson(content);


// --------------------------------------------------------
// Construire la nouvelle entrée
// --------------------------------------------------------

std::string entry =
    "  {\n"
    "    \"name\": \"" + escapeJson(name) + "\",\n"
    "    \"url\": \"" + escapeJson(url) + "\",\n"
    "    \"date\": \"" + escapeJson(getCurrentDateTime()) + "\"\n"
    "  }";


// --------------------------------------------------------
// Fichier vide / invalide -> nouveau tableau
// --------------------------------------------------------

if (content.empty() ||
    content.front() != '[' ||
    content.back() != ']')
{
    content =
        "[\n" + entry + "\n]";
}
else
{
    std::string inner =
        content.substr(1, content.size() - 2);

    inner = trimJson(inner);

    if (inner.empty())
    {
        content =
            "[\n" + entry + "\n]";
    }
    else
    {
        content =
            "[\n" + inner + ",\n" + entry + "\n]";
    }
}


// --------------------------------------------------------
// Écrire le fichier
// --------------------------------------------------------

std::ofstream outFile(
    jsonPath,
    std::ios::trunc
);

if (!outFile)
{
    std::cerr
        << Colors::Red
        << Colors::Bold
        << "ERROR"
        << Colors::Reset
        << ": Cannot write to packages.json\n";

    return;
}


outFile << content << "\n";

outFile.close();


std::cout
    << Colors::Dim
    << "-> Logged to packages.json"
    << Colors::Reset
    << "\n";


}

#endif
