import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin
import os
import tarfile
import zstandard as zstd
import argparse
import json

cache_dir = "cache"
os.makedirs(cache_dir, exist_ok=True)

def version():
    with open("info.json", "r") as f:
        data = json.load(f)

    print("the version of the software is V :", data.get("version"))


def extract_zst_tar(file_path, output_dir):
    os.makedirs(output_dir, exist_ok=True)

    tar_path = file_path.replace(".zst", "")

    try:
        # Décompression .zst -> .tar
        with open(file_path, "rb") as compressed:
            dctx = zstd.ZstdDecompressor()
            with open(tar_path, "wb") as decompressed:
                dctx.copy_stream(compressed, decompressed)

        # Extraction tar
        with tarfile.open(tar_path, "r:") as tar:
            tar.extractall(path=output_dir)

        print("Décompression terminée :", output_dir)

    finally:
        # Nettoyage fichiers intermédiaires
        if os.path.exists(tar_path):
            os.remove(tar_path)

        if os.path.exists(file_path):
            os.remove(file_path)

        print("Fichiers temporaires supprimés")


def download_and_extract(pkg, dep_dir):
    url = "https://packages.msys2.org/packages/" + pkg

    response = requests.get(url)
    response.raise_for_status()

    soup = BeautifulSoup(response.text, "html.parser")
    dds = soup.find_all("dd", class_="col-sm-9")

    downloaded_files = []

    for dd in dds:
        links = dd.find_all("a", href=True)

        for a in links:
            href = a["href"]

            if href.endswith(".pkg.tar.zst"):
                file_url = urljoin(url, href)
                filename = os.path.join(cache_dir, os.path.basename(file_url))

                print("Téléchargement :", file_url)

                r = requests.get(file_url, stream=True)
                r.raise_for_status()

                with open(filename, "wb") as f:
                    for chunk in r.iter_content(chunk_size=8192):
                        if chunk:
                            f.write(chunk)

                print("Fichier enregistré :", filename)
                downloaded_files.append(filename)

    if not downloaded_files:
        print("Aucun fichier .pkg.tar.zst trouvé.")
        return

    for file in downloaded_files:
        extract_zst_tar(file, dep_dir)


def main():
    parser = argparse.ArgumentParser(description="Mini gestionnaire MSYS2")

    parser.add_argument(
        "-S",
        dest="packages",
        nargs="+",
        help="Noms des packages MSYS2 à télécharger"
    )

    parser.add_argument(
        "-v",
        "--version",
        action="store_true",
        help="show version"
    )

    parser.add_argument(
        "-o",
        dest="output",
        default="cache/extracted",
        help="Dossier d'extraction (par défaut: cache/extracted)"
    )

    args = parser.parse_args()

    # AJOUTER ÇA
    if args.version:
        version()
        return

    if not args.packages:
        print("Usage: python main.py -S <package1> <package2> ... -o <output_dir>")
        return

    dep_dir = os.path.join(args.output, "dep")
    os.makedirs(dep_dir, exist_ok=True)

    print(f"[minimsys] extraction dir = {dep_dir}")

    for pkg in args.packages:
        print(f"\n=== Traitement de {pkg} ===")
        download_and_extract(pkg, dep_dir)


if __name__ == "__main__":
    main()