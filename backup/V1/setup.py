import os
import subprocess
import sys
import venv

# Dossier où se trouve le script
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# Nom du dossier de l'environnement virtuel
VENV_DIR = os.path.join(BASE_DIR, ".venv")

def create_virtualenv():
    """Crée l'environnement virtuel s'il n'existe pas."""
    if not os.path.exists(VENV_DIR):
        print(f"Création de l'environnement virtuel dans {VENV_DIR}...")
        venv.create(VENV_DIR, with_pip=True)
    else:
        print(f"L'environnement virtuel {VENV_DIR} existe déjà.")

def install_requirements():
    """Installe les packages depuis requirements.txt dans l'environnement virtuel."""
    pip_executable = os.path.join(VENV_DIR, "Scripts" if os.name == "nt" else "bin", "pip")
    
    requirements_path = os.path.join(BASE_DIR, "requirements.txt")
    if not os.path.exists(requirements_path):
        print("Aucun fichier requirements.txt trouvé dans le dossier.")
        return
    
    print("Installation des dépendances depuis requirements.txt...")
    subprocess.check_call([pip_executable, "install", "-r", requirements_path])

def main():
    create_virtualenv()
    install_requirements()
    print("Setup terminé !")

if __name__ == "__main__":
    main()