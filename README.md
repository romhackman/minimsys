# MiniMSYS (minimsys)

[![Logo]([https://packages.msys2.org/static/images/logo.svg](https://github.com/romhackman/minimsys/blob/main/logo_packet.png?raw=true)]

MiniMSYS est un gestionnaire simple de packages MSYS2 permettant de télécharger et extraire automatiquement des paquets `.pkg.tar.zst` depuis le dépôt MSYS2.

L’outil fonctionne via un exécutable (`minimsys.exe`) qui utilise un environnement Python embarqué (`venv`) pour gérer les dépendances et exécuter les scripts nécessaires.

---

## 🚀 Fonctionnalités

* Téléchargement de packages MSYS2
* Extraction automatique des archives `.pkg.tar.zst`
* Gestion des dépendances Python via environnement virtuel
* Interface simple en ligne de commande

---

## 📦 Installation

### 1. Télécharger le projet

Clone ou télécharge le dépôt :

```bash
git clone https://github.com/tonpseudo/minimsys.git
cd minimsys
```

---

### 2. Construire / utiliser l’exécutable

installation de la venv python :

```
python setup.py
```

Le programme principal est :

```
minimsys.exe
```

Il doit être placé dans le dossier du projet avec :

* `main.py`
* `.venv/` (créé automatiquement)
* `requirements.txt`

---

### 3. Ajouter au PATH (IMPORTANT)

Pour utiliser `minimsys` depuis n’importe quel terminal :

#### Windows

1. Copie le dossier contenant `minimsys.exe`
2. Ouvre :

   ```
   Paramètres > Système > Informations système > Paramètres avancés > Variables d’environnement
   ```
3. Dans **PATH**, ajoute le chemin du dossier
4. Redémarre ton terminal

---

## 🧪 Utilisation

### Télécharger un package MSYS2 :

```bash
minimsys -S <package1> <package2> 
```
⚠️ Le dossier de sortie sera le dossier où le programme est exécuté

### Exemple :

```bash
minimsys -S curl wget git
```

### Afficher le version de l'application :

```bash
minimsys -V 
```

---

Pour trouver facilement les paquets, allez sur le site de msys2:

[![MSYS2](https://packages.msys2.org/static/images/logo.svg)](https://packages.msys2.org/queue)



## 📁 Structure du projet

```
minimsys/
│
├── info.json           # Information application
├── setup.py            # Script de premaration
├── main.py             # Script principal Python
├── minimsys.exe        # Exécutable launcher
├── requirements.txt    # Dépendances Python
├── .venv/              # Environnement virtuel (auto-généré)
└── cache/              # Téléchargements temporaires
```

---

## 📚 Dépendances

Le projet utilise :

* `requests`
* `beautifulsoup4`
* `zstandard`

Installation automatique via le script de setup.

---

## ⚙️ Fonctionnement

1. `minimsys.exe` détecte les arguments `-S` `-V`
2. Lance `main.py` dans l’environnement virtuel
3. Récupère les packages depuis le site MSYS2
4. Télécharge les `.pkg.tar.zst`
5. Décompresse et extrait les fichiers

---

## 🧹 Nettoyage

Les fichiers temporaires (`.zst`, `.tar`) sont supprimés automatiquement après extraction.

---

## 📌 Notes

* Nécessite Python 3.10+ recommandé
* Internet requis pour télécharger les packages MSYS2
* Fonctionne principalement sous Windows (chemins `.venv/Scripts`)

---

## 🛠️ Améliorations possibles

* Cache intelligent des packages
* Résolution automatique des dépendances
* Support Linux/macOS
* Interface graphique

---

## 📄 Licence

Projet open-source
 
