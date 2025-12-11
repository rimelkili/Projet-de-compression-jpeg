/**
 * @file cDecompression.cpp
 * @brief Implémentation de la classe cDecompression
 * @author EL KILI Rim
 * @date 2025
 * @version 1.0
 */

#include "cDecompression.h"
#include <fstream>
#include <iostream>
#include <cmath>

using namespace std;

cDecompression::cDecompression() : cCompression() {}

cDecompression::cDecompression(unsigned int largeur, unsigned int hauteur, unsigned int qualite)
    : cCompression(largeur, hauteur, qualite) {}

char** cDecompression::Decompression_JPEG(const char* Nom_Fichier_compresse) {
    ifstream fichier(Nom_Fichier_compresse, ios::binary);
    if (!fichier) {
        cerr << "Erreur: Impossible d'ouvrir le fichier compressé" << endl;
        return nullptr;
    }

    // Lire les métadonnées
    unsigned int largeur, hauteur, qualite;
    fichier.read(reinterpret_cast<char*>(&largeur), sizeof(largeur));
    fichier.read(reinterpret_cast<char*>(&hauteur), sizeof(hauteur));
    fichier.read(reinterpret_cast<char*>(&qualite), sizeof(qualite));

    // Définir les propriétés
    setLargeur(largeur);
    setHauteur(hauteur);
    setQualite(qualite);

    // Allouer le buffer via la fonction publique
    allocBuffer(largeur, hauteur);

    // Lire la longueur de la trame
    int longueurTrame;
    fichier.read(reinterpret_cast<char*>(&longueurTrame), sizeof(longueurTrame));

    // Lire la trame RLE
    int* trame = new int[longueurTrame];
    fichier.read(reinterpret_cast<char*>(trame), longueurTrame * sizeof(int));

    fichier.close();

    // Calculer le nombre de blocs
    int nbBlocsX = largeur / 8;
    int nbBlocsY = hauteur / 8;
    int totalBlocs = nbBlocsX * nbBlocsY;

    // Allouer les blocs (chaque bloc a 64 coefficients)
    int** blocs = new int*[totalBlocs];
    for (int i = 0; i < totalBlocs; i++) {
        blocs[i] = new int[64];
        for (int j = 0; j < 64; j++) {
            blocs[i][j] = 0;
        }
    }

    // Décodeur RLE simplifié
    int indexTrame = 0;
    int DC_precedent = 0;

    // Table de parcours zigzag inverse
    int zigzagInv[64][2] = {
        {0,0}, {0,1}, {1,0}, {2,0}, {1,1}, {0,2}, {0,3}, {1,2},
        {2,1}, {3,0}, {4,0}, {3,1}, {2,2}, {1,3}, {0,4}, {0,5},
        {1,4}, {2,3}, {3,2}, {4,1}, {5,0}, {6,0}, {5,1}, {4,2},
        {3,3}, {2,4}, {1,5}, {0,6}, {0,7}, {1,6}, {2,5}, {3,4},
        {4,3}, {5,2}, {6,1}, {7,0}, {7,1}, {6,2}, {5,3}, {4,4},
        {3,5}, {2,6}, {1,7}, {2,7}, {3,6}, {4,5}, {5,4}, {6,3},
        {7,2}, {7,3}, {6,4}, {5,5}, {4,6}, {3,7}, {4,7}, {5,6},
        {6,5}, {7,4}, {7,5}, {6,6}, {5,7}, {6,7}, {7,6}, {7,7}
    };

    for (int b = 0; b < totalBlocs; b++) {
        // 1. Lire le coefficient DC (différence)
        int DC_diff = trame[indexTrame++];
        int DC = DC_precedent + DC_diff;

        // Stocker le coefficient DC
        blocs[b][0] = DC;

        // Mettre à jour pour le prochain bloc
        DC_precedent = DC;

        // 2. Lire les coefficients AC
        int coeffIndex = 1; // Commencer après DC
        bool finBloc = false;

        while (!finBloc && coeffIndex < 64) {
            if (indexTrame >= longueurTrame) {
                break;
            }

            int runlength = trame[indexTrame++];
            int amplitude = trame[indexTrame++];

            if (runlength == 0 && amplitude == 0) {
                finBloc = true;
            } else {
                // Sauter les zéros
                coeffIndex += runlength;

                if (coeffIndex < 64) {
                    // Stocker l'amplitude à la position zigzag
                    blocs[b][coeffIndex] = amplitude;
                    coeffIndex++;
                }
            }
        }
    }

    // Allocations temporaires
    char* charPtrs[8];
    double* doublePtrs[8];
    int* intPtrs[8];

    char blocChar[8][8];
    double dct[8][8];
    int quant[8][8];

    for (int i = 0; i < 8; i++) {
        charPtrs[i] = blocChar[i];
        doublePtrs[i] = dct[i];
        intPtrs[i] = quant[i];
    }

    // Récupérer le buffer via la fonction publique
    unsigned char** buffer = getBuffer();

    // Traiter chaque bloc
    for (int by = 0; by < nbBlocsY; by++) {
        for (int bx = 0; bx < nbBlocsX; bx++) {
            int blocIndex = by * nbBlocsX + bx;

            // 1. Reconstruire la matrice quantifiée 8x8 à partir du tableau linéaire
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    // Conversion de l'indice linéaire
                    int linearIndex = i * 8 + j;
                    quant[i][j] = blocs[blocIndex][linearIndex];
                }
            }

            // 2. Déquantifier
            dequant_JPEG(intPtrs, doublePtrs);

            // 3. Appliquer IDCT
            Calcul_IDCT(doublePtrs, charPtrs);

            // 4. Convertir de -128..127 à 0..255 et écrire dans l'image
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    unsigned int x = bx * 8 + j;
                    unsigned int y = by * 8 + i;

                    if (x < largeur && y < hauteur) {
                        // Conversion et saturation
                        int valeur = blocChar[i][j] + 128;
                        if (valeur < 0) valeur = 0;
                        if (valeur > 255) valeur = 255;
                        buffer[y][x] = static_cast<unsigned char>(valeur);
                    }
                }
            }
        }
    }

    // Nettoyage
    for (int i = 0; i < totalBlocs; i++) {
        delete[] blocs[i];
    }
    delete[] blocs;
    delete[] trame;

    // Convertir en char** pour le retour
    char** result = new char*[hauteur];
    for (unsigned int i = 0; i < hauteur; i++) {
        result[i] = new char[largeur];
        for (unsigned int j = 0; j < largeur; j++) {
            result[i][j] = static_cast<char>(buffer[i][j]);
        }
    }

    cout << "Décompression terminée: " << largeur << "x" << hauteur
         << " qualité " << qualite << "%" << endl;

    return result;
}

void cDecompression::decoder_RLE(int* Trame, int longueur, int nbBlocsX, int nbBlocsY, int blocs[][64]) {
    // Cette fonction est déjà intégrée dans Decompression_JPEG
    cout << "Décodage RLE: " << nbBlocsX << "x" << nbBlocsY << " blocs" << endl;
}

void cDecompression::reconstruireImage(int blocs[][64], int nbBlocsX, int nbBlocsY) {
    // Cette fonction est déjà intégrée dans Decompression_JPEG
    cout << "Reconstruction image: " << nbBlocsX << "x" << nbBlocsY << " blocs" << endl;
}

bool cDecompression::testerCompressionDecompression(const char* fichierEntree, int qualite) {
    cout << "\n=== Test Compression/Décompression ===" << endl;
    cout << "Fichier: " << fichierEntree << endl;
    cout << "Qualité: " << qualite << "%" << endl;

    // 1. Charger l'image originale
    cCompression compresseur;

    if (!compresseur.chargerImageTexte(fichierEntree, 256, 256)) {
        cerr << "Erreur de chargement de l'image" << endl;
        return false;
    }

    compresseur.setQualite(qualite);

    // 2. Compresser
    string nomFichierSortie = string(fichierEntree) + "_compresse_q" + to_string(qualite) + ".dat";
    if (!compresseur.compresser(nomFichierSortie.c_str())) {
        cerr << "Erreur de compression" << endl;
        return false;
    }

    // 3. Décompresser
    cDecompression decompresseur;
    char** imageDecompressee = decompresseur.Decompression_JPEG(nomFichierSortie.c_str());

    if (!imageDecompressee) {
        cerr << "Erreur de décompression" << endl;
        return false;
    }

    // 4. Calculer des statistiques
    unsigned int largeur = compresseur.getLargeur();
    unsigned int hauteur = compresseur.getHauteur();

    // Récupérer l'image originale
    unsigned char** original = compresseur.getBuffer();

    // Calculer l'erreur quadratique moyenne
    double erreurTotale = 0.0;
    int pixels = largeur * hauteur;

    for (unsigned int i = 0; i < hauteur; i++) {
        for (unsigned int j = 0; j < largeur; j++) {
            int diff = static_cast<int>(original[i][j]) - static_cast<int>(imageDecompressee[i][j]);
            erreurTotale += diff * diff;
        }
    }

    double eqm = sqrt(erreurTotale / pixels);
    double psnr = (eqm > 0) ? 20 * log10(255.0 / eqm) : 99.99;

    cout << "Resultats:" << endl;
    cout << "  - EQM: " << eqm << endl;
    cout << "  - PSNR: " << psnr << " dB" << endl;
    cout << "  - Taille originale: " << pixels << " octets" << endl;

    // Nettoyage
    for (unsigned int i = 0; i < hauteur; i++) {
        delete[] imageDecompressee[i];
    }
    delete[] imageDecompressee;

    cout << "Test reussi!" << endl;
    return true;
}
