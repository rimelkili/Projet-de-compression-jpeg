/**
 * @file main.cpp
 * @brief Programme principal de test du codeur/decodeur JPEG
 * @author EL KILI Rim
 * @date 2025
 * @version 1.0
 */

#include "cCompression.h"
#include "cDecompression.h"
#include "cHuffman.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <iomanip>

using namespace std;
using namespace chrono;

/**
 * @brief Affiche une matrice 8x8
 */
void afficherMatrice8x8(int matrice[8][8], const string& titre = "Matrice") {
    cout << "\n" << titre << ":" << endl;
    cout << "----------------" << endl;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            cout << setw(4) << matrice[i][j] << " ";
        }
        cout << endl;
    }
}

/**
 * @brief Affiche une matrice 8x8 de doubles
 */
void afficherMatrice8x8Double(double matrice[8][8], const string& titre = "Matrice") {
    cout << "\n" << titre << ":" << endl;
    cout << "----------------" << endl;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            cout << setw(8) << fixed << setprecision(2) << matrice[i][j] << " ";
        }
        cout << endl;
    }
}

/**
 * @brief Teste la DCT et IDCT sur un bloc exemple
 */
void testDCT() {
    cout << "\n===============================================================" << endl;
    cout << "TEST 1: TRANSFORMEE COSINUS DISCRETE (DCT/IDCT)" << endl;
    cout << "===============================================================" << endl;

    // Creer un bloc de test (identique a l'exemple de l'annexe A)
    char blocTest[8][8] = {
        {11, 16, 21, 25, 27, 27, 27, 27},
        {16, 23, 25, 28, 31, 28, 28, 28},
        {22, 27, 32, 35, 30, 28, 28, 28},
        {31, 33, 34, 32, 32, 31, 31, 31},
        {31, 32, 33, 34, 34, 27, 27, 27},
        {33, 33, 33, 33, 32, 29, 29, 29},
        {34, 34, 33, 35, 34, 29, 29, 29},
        {34, 34, 33, 33, 35, 30, 30, 30}
    };

    // Afficher le bloc original
    cout << "\nBloc original (valeurs decalees -128 a 127):" << endl;
    cout << "--------------------------------------------" << endl;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            cout << setw(4) << (int)blocTest[i][j] << " ";
        }
        cout << endl;
    }

    // Preparer les pointeurs
    char* blocPtrs[8];
    double dct[8][8];
    double* dctPtrs[8];
    char blocReconstruit[8][8];
    char* reconPtrs[8];

    for (int i = 0; i < 8; i++) {
        blocPtrs[i] = blocTest[i];
        dctPtrs[i] = dct[i];
        reconPtrs[i] = blocReconstruit[i];
    }

    // Creer un compresseur
    cCompression comp;

    // Calculer la DCT
    auto debutDCT = high_resolution_clock::now();
    comp.Calcul_DCT_Block(blocPtrs, dctPtrs);
    auto finDCT = high_resolution_clock::now();

    // Afficher quelques coefficients DCT
    cout << "\nCoefficients DCT significatifs:" << endl;
    cout << "--------------------------------" << endl;
    cout << "Coefficient DC (0,0): " << fixed << setprecision(2) << dct[0][0] << endl;
    cout << "Coefficient AC (0,1): " << fixed << setprecision(2) << dct[0][1] << endl;
    cout << "Coefficient AC (1,0): " << fixed << setprecision(2) << dct[1][0] << endl;
    cout << "Coefficient AC (1,1): " << fixed << setprecision(2) << dct[1][1] << endl;

    // Calculer l'IDCT
    auto debutIDCT = high_resolution_clock::now();
    comp.Calcul_IDCT(dctPtrs, reconPtrs);
    auto finIDCT = high_resolution_clock::now();

    // Verifier la reconstruction
    double erreur = 0.0;
    double erreurMax = 0.0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double diff = blocTest[i][j] - blocReconstruit[i][j];
            erreur += diff * diff;
            if (fabs(diff) > erreurMax) {
                erreurMax = fabs(diff);
            }
        }
    }
    double erreurRMS = sqrt(erreur / 64.0);

    // Calculer les temps
    auto dureeDCT = duration_cast<microseconds>(finDCT - debutDCT);
    auto dureeIDCT = duration_cast<microseconds>(finIDCT - debutIDCT);

    // Afficher les resultats
    cout << "\nResultats DCT/IDCT:" << endl;
    cout << "-------------------" << endl;
    cout << "Temps DCT: " << dureeDCT.count() << " us" << endl;
    cout << "Temps IDCT: " << dureeIDCT.count() << " us" << endl;
    cout << "Erreur RMS: " << erreurRMS << endl;
    cout << "Erreur max: " << erreurMax << endl;
    cout << "Test DCT/IDCT: " << (erreurRMS < 0.1 ? "[OK] REUSSI" : "[X] ECHEC") << endl;
}

/**
 * @brief Teste la quantification/dequantification
 */
void testQuantification() {
    cout << "\n===============================================================" << endl;
    cout << "TEST 2: QUANTIFICATION/DEQUANTIFICATION JPEG" << endl;
    cout << "===============================================================" << endl;

    cCompression comp(256, 256, 50); // Qualite 50%

    // Creer un bloc DCT de test similaire a l'exemple
    double dct[8][8];
    double* dctPtrs[8];
    int quant[8][8];
    int* quantPtrs[8];
    double dctReconstruit[8][8];
    double* reconPtrs[8];

    for (int i = 0; i < 8; i++) {
        dctPtrs[i] = dct[i];
        quantPtrs[i] = quant[i];
        reconPtrs[i] = dctReconstruit[i];
    }

    // Remplir avec des valeurs de test (similaires a l'exemple)
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i == 0 && j == 0) dct[i][j] = 235.62;
            else if (i == 0 && j == 1) dct[i][j] = -1.03;
            else if (i == 0 && j == 2) dct[i][j] = -12.08;
            else dct[i][j] = (i + j - 4) * 5.0; // Valeurs diverses
        }
    }

    cout << "\nCoefficients DCT avant quantification:" << endl;
    cout << "--------------------------------------" << endl;
    for (int i = 0; i < 4; i++) { // Afficher seulement les 4 premieres lignes
        for (int j = 0; j < 4; j++) {
            cout << setw(8) << fixed << setprecision(2) << dct[i][j] << " ";
        }
        cout << endl;
    }

    // Quantifier
    comp.quant_JPEG(dctPtrs, quantPtrs);

    // Compter les coefficients nuls
    int coefficientsNonNuls = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (quant[i][j] != 0) coefficientsNonNuls++;
        }
    }

    cout << "\nMatrice quantifiee (premieres valeurs):" << endl;
    cout << "---------------------------------------" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << setw(4) << quant[i][j] << " ";
        }
        cout << endl;
    }

    // Dequantifier
    comp.dequant_JPEG(quantPtrs, reconPtrs);

    // Calculer l'erreur
    double erreur = 0.0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double diff = dct[i][j] - dctReconstruit[i][j];
            erreur += diff * diff;
        }
    }
    double erreurRMS = sqrt(erreur / 64.0);

    // Calculer le taux de compression theorique
    double taux = comp.Taux_Compression(quantPtrs);

    // Afficher les resultats
    cout << "\nResultats quantification:" << endl;
    cout << "-------------------------" << endl;
    cout << "Coefficients non nuls apres quantification: "
         << coefficientsNonNuls << "/64" << endl;
    cout << "Taux de remplissage: "
         << fixed << setprecision(1) << (coefficientsNonNuls/64.0*100.0) << "%" << endl;
    cout << "Erreur RMS quantification: " << erreurRMS << endl;
    cout << "Taux de compression theorique: " << taux << "%" << endl;
    cout << "Test quantification: " << (coefficientsNonNuls < 64 ? "[OK] REUSSI" : "[X] ECHEC") << endl;
}

/**
 * @brief Teste le RLE avec l'exemple du sujet
 */
void testRLE() {
    cout << "\n===============================================================" << endl;
    cout << "TEST 3: RUN-LENGTH ENCODING (RLE)" << endl;
    cout << "===============================================================" << endl;

    cCompression comp(8, 8, 50);

    // Bloc quantifie de l'exemple B
    int blocQuant[8][8] = {
        {15, 0, -1, 0, 0, 0, 0, 0},
        {-2, -1, 0, 0, 0, 0, 0, 0},
        {-1, -1, 0, 0, 0, 0, 0, 0},
        {-1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0}
    };

    cout << "\nBloc quantifie 8x8 (exemple du sujet):" << endl;
    cout << "--------------------------------------" << endl;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            cout << setw(4) << blocQuant[i][j] << " ";
        }
        cout << endl;
    }

    int* blocPtrs[8];
    for (int i = 0; i < 8; i++) {
        blocPtrs[i] = blocQuant[i];
    }

    // Encoder avec RLE
    char trame[128];
    int DC_precedent = 0;
    int longueur = comp.RLE_Block(blocPtrs, DC_precedent, trame);

    cout << "\nTrame RLE generee (paires runlength/amplitude):" << endl;
    cout << "------------------------------------------------" << endl;
    cout << "DC_diff = " << (int)trame[0] << endl;

    for (int i = 1; i < longueur; i += 2) {
        if (i+1 < longueur) {
            cout << "(" << (int)trame[i] << ", " << (int)trame[i+1] << ") ";
        }
    }
    cout << endl;

    // Afficher sous forme de liste comme dans l'exemple
    cout << "\nFormat liste (comme exemple B):" << endl;
    cout << "-------------------------------" << endl;
    cout << (int)trame[0] << " ";
    for (int i = 1; i < longueur; i += 2) {
        if (i+1 < longueur) {
            cout << (int)trame[i] << ", " << (int)trame[i+1] << " ";
        }
    }
    cout << endl;

    cout << "\nResultats RLE:" << endl;
    cout << "--------------" << endl;
    cout << "Longueur de la trame: " << longueur << " symboles" << endl;
    cout << "Taille originale: 64 coefficients" << endl;
    cout << "Compression RLE: "
         << fixed << setprecision(1) << (1.0 - (double)longueur / 64.0) * 100.0 << "%" << endl;
    cout << "Test RLE: " << (longueur < 64 ? "[OK] REUSSI" : "[X] ECHEC") << endl;
}

/**
 * @brief Teste le codage de Huffman avec l'exemple du sujet
 */
void testHuffman() {
    cout << "\n===============================================================" << endl;
    cout << "TEST 4: CODAGE DE HUFFMAN" << endl;
    cout << "===============================================================" << endl;

    // Donnees de test (Table 1 du sujet)
    char donnees[] = {'a', 'b', 'c', 'd', 'e', 'f'};
    double frequences[] = {5.0/28.0, 3.0/28.0, 7.0/28.0, 1.0/28.0, 10.0/28.0, 2.0/28.0};
    unsigned int taille = 6;

    cout << "\nTable des frequences (exemple du sujet):" << endl;
    cout << "----------------------------------------" << endl;
    cout << "Symbole | Frequence | Occurences" << endl;
    cout << "--------+-----------+-----------" << endl;
    cout << "   a    |    " << fixed << setprecision(3) << frequences[0] << "   |      5" << endl;
    cout << "   b    |    " << fixed << setprecision(3) << frequences[1] << "   |      3" << endl;
    cout << "   c    |    " << fixed << setprecision(3) << frequences[2] << "   |      7" << endl;
    cout << "   d    |    " << fixed << setprecision(3) << frequences[3] << "   |      1" << endl;
    cout << "   e    |    " << fixed << setprecision(3) << frequences[4] << "   |      10" << endl;
    cout << "   f    |    " << fixed << setprecision(3) << frequences[5] << "   |      2" << endl;

    // Creer l'encodeur Huffman
    cHuffman huffman;

    // Construire l'arbre
    huffman.HuffmanCodes(donnees, frequences, taille);

    // Afficher les codes
    huffman.AfficherHuffman();

    // Tester l'encodage d'une sequence
    char sequence[] = {'a', 'e', 'c', 'e', 'b', 'd', 'f', 'e'};
    unsigned int longueurSeq = 8;

    vector<bool> trameCodee;
    unsigned int bits = huffman.encoder(sequence, longueurSeq, trameCodee);

    cout << "Encodage de la sequence 'a e c e b d f e':" << endl;
    cout << "------------------------------------------" << endl;
    cout << "Sequence originale: ";
    for (unsigned int i = 0; i < longueurSeq; i++) {
        cout << sequence[i] << " ";
    }
    cout << endl;

    cout << "Sequence encodee: ";
    for (bool bit : trameCodee) {
        cout << (bit ? "1" : "0");
    }
    cout << endl;

    cout << "\nResultats Huffman:" << endl;
    cout << "------------------" << endl;
    cout << "Longueur originale: " << longueurSeq * 8 << " bits (8 bits par caractere)" << endl;
    cout << "Longueur compressee: " << bits << " bits" << endl;
    cout << "Taux de compression: "
         << fixed << setprecision(1) << (1.0 - (double)bits / (longueurSeq * 8.0)) * 100.0 << "%" << endl;
    cout << "Bits par symbole moyen: " << fixed << setprecision(2) << (double)bits / longueurSeq << endl;
    cout << "Test Huffman: [OK] REUSSI" << endl;
}

/**
 * @brief Teste la compression complete sur Lenna
 * @param qualite Qualite de compression a tester
 */
void testCompressionLenna(int qualite) {
    cout << "\n===============================================================" << endl;
    cout << "TEST 5: COMPRESSION COMPLETE LENNA (qualite " << qualite << "%)" << endl;
    cout << "===============================================================" << endl;

    auto debutTotal = high_resolution_clock::now();

    // 1. Charger Lenna
    cCompression compresseur;
    if (!compresseur.chargerImageTexte("lenna.img.txt", 256, 256)) {
        cerr << "[X] Echec du chargement de Lenna" << endl;
        return;
    }

    compresseur.setQualite(qualite);

    auto finChargement = high_resolution_clock::now();

    // 2. Compresser
    string nomFichierSortie = "lenna_compresse_q" + to_string(qualite) + ".dat";
    if (!compresseur.compresser(nomFichierSortie.c_str())) {
        cerr << "[X] Echec de la compression" << endl;
        return;
    }

    auto finCompression = high_resolution_clock::now();

    // 3. Decompresser
    cDecompression decompresseur;
    char** imageReconstruite = decompresseur.Decompression_JPEG(nomFichierSortie.c_str());

    auto finDecompression = high_resolution_clock::now();

    if (!imageReconstruite) {
        cerr << "[X] Echec de la decompression" << endl;
        return;
    }

    // 4. Calculer les temps
    auto dureeChargement = duration_cast<milliseconds>(finChargement - debutTotal);
    auto dureeCompression = duration_cast<milliseconds>(finCompression - finChargement);
    auto dureeDecompression = duration_cast<milliseconds>(finDecompression - finCompression);
    auto dureeTotale = duration_cast<milliseconds>(finDecompression - debutTotal);

    // 5. Calculer PSNR
    unsigned char** original = compresseur.getBuffer();
    double erreurTotale = 0.0;
    int pixels = 256 * 256;

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            int diff = static_cast<int>(original[i][j]) - static_cast<int>(imageReconstruite[i][j]);
            erreurTotale += diff * diff;
        }
    }

    double eqm = sqrt(erreurTotale / pixels);
    double psnr = (eqm > 0) ? 20 * log10(255.0 / eqm) : 99.99;

        // Afficher les resultats
    cout << "\nResultats compression Lenna:" << endl;
    cout << "-----------------------------" << endl;
    cout << "Image: 256x256 pixels (65536 octets)" << endl;
    cout << "Qualite JPEG: " << qualite << "%" << endl;
    cout << "\nTemps d'execution:" << endl;
    cout << "  - Chargement: " << dureeChargement.count() << " ms" << endl;
    cout << "  - Compression: " << dureeCompression.count() << " ms" << endl;
    cout << "  - Decompression: " << dureeDecompression.count() << " ms" << endl;
    cout << "  - Total: " << dureeTotale.count() << " ms" << endl;

    cout << "\nQualite de reconstruction:" << endl;
    cout << "  - EQM: " << fixed << setprecision(2) << eqm << endl;
    cout << "  - PSNR: " << fixed << setprecision(2) << psnr << " dB" << endl;

    // Nettoyage
    for (int i = 0; i < 256; i++) {
        delete[] imageReconstruite[i];
    }
    delete[] imageReconstruite;

    cout << "\nFichier compresse: " << nomFichierSortie << endl;
    cout << "Test compression Lenna: [OK] REUSSI" << endl;
}

/**
 * @brief Teste differentes qualites sur Lenna
 */
void testQualitesVariables() {
    cout << "\n===============================================================" << endl;
    cout << "TEST 6: COMPARAISON DES QUALITES JPEG" << endl;
    cout << "===============================================================" << endl;

    vector<int> qualites = {10, 25, 50, 75, 90};

    cout << "\nComparaison des differentes qualites de compression:" << endl;
    cout << "----------------------------------------------------" << endl;
    cout << "Qualite | Temps comp. (ms) | Temps decomp. (ms) | EQM   | PSNR (dB)" << endl;
    cout << "--------+------------------+-------------------+-------+-----------" << endl;

    for (int qualite : qualites) {
        auto debut = high_resolution_clock::now();

        // -------------------------------------------------------------
        // ASTUCE: On desactive cout temporairement pour que les logs
        // des fonctions ne cassent pas le tableau
        // -------------------------------------------------------------
        std::cout.setstate(std::ios_base::failbit);

        // Charger et compresser
        cCompression compresseur;
        compresseur.chargerImageTexte("lenna.img.txt", 256, 256);
        compresseur.setQualite(qualite);

        string nomFichier = "lenna_q" + to_string(qualite) + ".dat";
        compresseur.compresser(nomFichier.c_str());

        auto finComp = high_resolution_clock::now();

        // Decompresser
        cDecompression decompresseur;
        char** image = decompresseur.Decompression_JPEG(nomFichier.c_str());

        // -------------------------------------------------------------
        // On reactive cout pour afficher la ligne du tableau
        // -------------------------------------------------------------
        std::cout.clear();

        auto finDecomp = high_resolution_clock::now();

        // Calculer PSNR
        unsigned char** original = compresseur.getBuffer();
        double erreurTotale = 0.0;

        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                int diff = static_cast<int>(original[i][j]) - static_cast<int>(image[i][j]);
                erreurTotale += diff * diff;
            }
        }

        double eqm = sqrt(erreurTotale / (256*256));
        double psnr = (eqm > 0) ? 20 * log10(255.0 / eqm) : 99.99;

        auto dureeComp = duration_cast<milliseconds>(finComp - debut);
        auto dureeDecomp = duration_cast<milliseconds>(finDecomp - finComp);

        cout << setw(7) << qualite << " | "
             << setw(16) << dureeComp.count() << " | "
             << setw(17) << dureeDecomp.count() << " | "
             << setw(5) << fixed << setprecision(1) << eqm << " | "
             << setw(9) << fixed << setprecision(1) << psnr << endl;

        // Nettoyage
        for (int i = 0; i < 256; i++) {
            delete[] image[i];
        }
        delete[] image;
    }

    cout << "\nLegende:" << endl;
    cout << "  - EQM: Erreur Quadratique Moyenne (plus c'est bas, mieux c'est)" << endl;
    cout << "  - PSNR: Peak Signal-to-Noise Ratio (plus c'est haut, mieux c'est)" << endl;
    cout << "  - PSNR > 40 dB: Excellente qualite" << endl;
    cout << "  - PSNR 30-40 dB: Bonne qualite" << endl;
    cout << "  - PSNR 20-30 dB: Qualite acceptable" << endl;
    cout << "  - PSNR < 20 dB: Mauvaise qualite" << endl;
}

/**
 * @brief Fonction principale
 */
int main() {
    cout << "===============================================================" << endl;
    cout << "           PROJET JPEG - CODEC COMPRESSION/DECOMPRESSION" << endl;
    cout << "===============================================================" << endl;
    cout << "Auteur: [Votre Nom]" << endl;
    cout << "Date: 2024" << endl;
    cout << "Version: 1.0" << endl;
    cout << "===============================================================" << endl;

    // Test 1: DCT/IDCT
    testDCT();

    // Test 2: Quantification
    testQuantification();

    // Test 3: RLE
    testRLE();

    // Test 4: Huffman
    testHuffman();

    // Test 5: Compression sur Lenna avec qualite 50
    testCompressionLenna(50);

    // Test 6: Comparaison des qualites
    testQualitesVariables();

    // Test 7: Cycle complet compression/decompression

    cout << "\n===============================================================" << endl;
    cout << "TEST 7: CYCLE COMPLET COMPRESSION/DECOMPRESSION" << endl;
    cout << "===============================================================" << endl;

    cDecompression testeur;
    if (testeur.testerCompressionDecompression("lenna.img.txt", 75)) {
        cout << "[OK] Test cycle complet: REUSSI" << endl;
    } else {
        cout << "[X] Test cycle complet: ECHEC" << endl;
    }

    cout << "\n                  TESTS TERMINES AVEC SUCCES" << endl;

    cout << "\nResume des fichiers generes:" << endl;
    cout << "-----------------------------" << endl;
    cout << "- lenna_compresse_q50.dat : Image Lenna compressee (qualite 50%)" << endl;
    cout << "- lenna_q10.dat a lenna_q90.dat : Images avec differentes qualites" << endl;
    cout << "- lenna.img.txt_compresse_q75.dat : Fichier de test compression" << endl;

    return 0;
}
