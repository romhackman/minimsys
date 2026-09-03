# minimsys

Un petit outil en ligne de commande (C++) pour rechercher, inspecter et télécharger des packages depuis [packages.msys2.org](https://packages.msys2.org), sans avoir besoin de `pacman` ou d'un environnement MSYS2 complet.

## Fonctionnalités

- 🔍 **Recherche de packages** par mot-clé (`-Ss`)
- 📦 **Informations détaillées** sur un package : dépendances, lien de téléchargement (`-S`)
- ⬇️ **Téléchargement** du package dans un dossier `./cache`, avec ou sans confirmation
- 🔗 **Téléchargement automatique des dépendances** (mode `-y`)
- 🧭 **Fallback intelligent** : si un package n'existe pas, recherche automatique de son "package de base" (ex: `mingw-w64-x86_64-SDL2_ttf` → `mingw-w64-x86_64-SDL2`)
- 📝 **Journalisation** des packages téléchargés dans `packages.json`
- 🎨 Sortie colorée dans le terminal (ANSI)

## Prérequis

- Un compilateur C++17 (ou supérieur)
- [libcurl](https://curl.se/libcurl/) (headers + bibliothèque)

## Compilation

```bash
g++ -std=c++17 main.cpp -o minimsys -lcurl
```

> Sous MSYS2/MinGW, assurez-vous d'avoir installé le package `curl` (ex: `mingw-w64-x86_64-curl`) avant de compiler.

## Utilisation

```
minimsys [option] [package]
```

### Options

| Option                     | Description                                                              |
|----------------------------|----------------------------------------------------------------------------|
| `-S <package>`             | Afficher les informations d'un package                                   |
| `-S <package> -y`          | Télécharger le package + toutes ses dépendances (sans confirmation)      |
| `-S <package> -n`          | Télécharger uniquement le package, sans les dépendances (sans confirmation) |
| `-Ss <query>`               | Rechercher des packages                                                  |
| `-h`, `--help`             | Afficher l'aide                                                          |
| `--version`                | Afficher la version                                                      |

### Exemples

```bash
# Afficher les informations d'un package (avec confirmation de téléchargement)
minimsys -S mingw-w64-x86_64-gcc

# Télécharger un package et toutes ses dépendances, sans confirmation
minimsys -S mingw-w64-ucrt-x86_64-curl -y

# Télécharger uniquement le package, sans ses dépendances
minimsys -S mingw-w64-ucrt-x86_64-curl -n

# Rechercher des packages contenant "gcc"
minimsys -Ss gcc

# Aide
minimsys -h

# Version
minimsys --version
```

## Structure du projet

| Fichier              | Rôle                                                                                   |
|-----------------------|-----------------------------------------------------------------------------------------|
| `main.cpp`            | Point d'entrée : parsing des arguments et dispatch des commandes                       |
| `search.h`            | Logique principale de `-S` : récupération de la page package, dépendances, téléchargement, gestion des dépendances |
| `base_search.h`       | Fallback quand un package est introuvable (`404`) : recherche du package de base et liste ses variantes binaires |
| `search_packages.h`   | Logique de `-Ss` : recherche de packages par mot-clé et affichage des résultats        |
| `download.h`          | Téléchargement d'un fichier `.pkg.tar.zst` vers `./cache`, avec confirmation interactive |
| `packages_json.h`     | Journalisation des packages téléchargés dans `packages.json`                            |
| `colors.h`             | Constantes de couleurs ANSI pour la sortie terminal                                     |
| `help.h`               | Affichage de l'aide (`-h` / `--help`)                                                   |
| `version.h`             | Nom et numéro de version du programme                                                   |

## Fonctionnement interne

1. **`-S <package>`** interroge `https://packages.msys2.org/packages/<package>`.
   - Si le package est trouvé (`HTTP 200`), les dépendances et le lien de téléchargement `.pkg.tar.zst` sont extraits par expressions régulières depuis le HTML de la page.
   - Si le package est introuvable (`HTTP 404`), `searchBasePackage` est appelé pour chercher le package "de base" (partie avant le dernier `_`) et lister ses variantes disponibles par environnement (mingw64, ucrt64, clang64, etc.).
2. **`-Ss <query>`** interroge `https://packages.msys2.org/search?q=<query>` et parse le tableau de résultats HTML (nom, version, description).
3. Le téléchargement se fait via `libcurl` vers le dossier `./cache` (créé automatiquement s'il n'existe pas). Si le fichier existe déjà en cache, il n'est pas re-téléchargé.
4. Chaque téléchargement réussi est ajouté à `./packages.json`, avec le nom du fichier, l'URL et la date/heure.

## Fichiers générés

- `./cache/` — contient les fichiers `.pkg.tar.zst` téléchargés
- `./packages.json` — historique des packages téléchargés (nom, URL, date)

## Limitations connues

- Le parsing HTML repose sur des expressions régulières adaptées à la structure actuelle du site `packages.msys2.org` ; toute modification de cette structure peut casser l'extraction.
- Pas de vérification de checksum/signature après téléchargement.
- Le mode `-y` télécharge les dépendances de premier niveau uniquement (pas de résolution récursive des sous-dépendances).

## Licence

Non spécifiée.
