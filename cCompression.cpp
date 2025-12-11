/**
 * @file cCompression.cpp
 * @brief Implémentation de la classe cCompression
 * @author EL KILI Rim
 * @date 2025
 * @version 1.0
 */

#define _USE_MATH_DEFINES  // Pour M_PI sous Windows
#include "cCompression.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

// Table de quantification JPEG standard pour la luminance
const int cCompression::TABLE_QUANT[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}
};

// Coefficients C(u) et C(v) pour la DCT
namespace {
    const double C[8] = {
        1.0 / sqrt(2.0), 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    };
}

// ========== CONSTRUCTEURS ET DESTRUCTEUR ==========

cCompression::cCompression()
    : mLargeur(0), mHauteur(0), mBuffer(nullptr), mQualite(50) {
}

cCompression::cCompression(unsigned int largeur, unsigned int hauteur, unsigned int qualite)
    : mLargeur(largeur), mHauteur(hauteur), mQualite(qualite), mBuffer(nullptr) {

    // Allocation du buffer
    allocBuffer(largeur, hauteur);
}

cCompression::~cCompression() {
    if (mBuffer) {
        for (unsigned int i = 0; i < mHauteur; i++) {
            delete[] mBuffer[i];
        }
        delete[] mBuffer;
    }
}

// ========== ACCESS ET MUTATEURS ==========

unsigned int cCompression::getLargeur() const { return mLargeur; }
unsigned int cCompression::getHauteur() const { return mHauteur; }
unsigned int cCompression::getQualite() const { return mQualite; }
unsigned char** cCompression::getBuffer() const { return mBuffer; }

void cCompression::setLargeur(unsigned int largeur) { mLargeur = largeur; }
void cCompression::setHauteur(unsigned int hauteur) { mHauteur = hauteur; }
void cCompression::setQualite(unsigned int qualite) {
    if (qualite <= 100) mQualite = qualite;
}

void cCompression::setBuffer(unsigned char** buffer) { mBuffer = buffer; }

void cCompression::allocBuffer(unsigned int largeur, unsigned int hauteur) {
    // Libérer l'ancien buffer si existe
    if (mBuffer) {
        for (unsigned int i = 0; i < mHauteur; i++) {
            delete[] mBuffer[i];
        }
        delete[] mBuffer;
    }

    // Allouer le nouveau buffer
    mLargeur = largeur;
    mHauteur = hauteur;
    mBuffer = new unsigned char*[mHauteur];
    for (unsigned int i = 0; i < mHauteur; i++) {
        mBuffer[i] = new unsigned char[mLargeur];
        memset(mBuffer[i], 0, mLargeur);
    }
}

// ========== FONCTIONS DE BASE DCT/IDCT ==========

void cCompression::Calcul_DCT_Block(char** Block8, double** DCT_Img) {
    // Initialisation de la matrice de sortie
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            DCT_Img[u][v] = 0.0;
        }
    }

    // Calcul de la DCT
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            double sum = 0.0;

            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    double cos1 = cos((2 * x + 1) * u * M_PI / 16.0);
                    double cos2 = cos((2 * y + 1) * v * M_PI / 16.0);
                    sum += Block8[x][y] * cos1 * cos2;
                }
            }

            DCT_Img[u][v] = 0.25 * C[u] * C[v] * sum;
        }
    }
}

void cCompression::Calcul_IDCT(double** DCT_Img, char** Block8) {
    // Initialisation du bloc de sortie
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Block8[x][y] = 0;
        }
    }

    // Calcul de la DCT inverse
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            double sum = 0.0;

            for (int u = 0; u < 8; u++) {
                for (int v = 0; v < 8; v++) {
                    double cos1 = cos((2 * x + 1) * u * M_PI / 16.0);
                    double cos2 = cos((2 * y + 1) * v * M_PI / 16.0);
                    sum += DCT_Img[u][v] * C[u] * C[v] * cos1 * cos2;
                }
            }

            Block8[x][y] = static_cast<char>(round(0.25 * sum));
        }
    }
}

// ========== FONCTIONS DE QUANTIFICATION ==========

void cCompression::calculerTableQuantification(int qualite, int table[8][8]) {
    double lambda;

    if (qualite < 50) {
        lambda = 5000.0 / qualite;
    } else {
        lambda = 200.0 - 2.0 * qualite;
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double valeur = (TABLE_QUANT[i][j] * lambda + 50.0) / 100.0;

            // Application des bornes
            if (valeur < 1.0) {
                table[i][j] = 1;
            } else if (valeur > 255.0) {
                table[i][j] = 255;
            } else {
                table[i][j] = static_cast<int>(round(valeur));
            }
        }
    }
}

void cCompression::quant_JPEG(double** img_DCT, int** Img_Quant) {
    int table[8][8];
    calculerTableQuantification(mQualite, table);

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Img_Quant[i][j] = static_cast<int>(round(img_DCT[i][j] / table[i][j]));
        }
    }
}

void cCompression::dequant_JPEG(int** Img_Quant, double** img_DCT) {
    int table[8][8];
    calculerTableQuantification(mQualite, table);

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            img_DCT[i][j] = Img_Quant[i][j] * table[i][j];
        }
    }
}

// ========== FONCTIONS DE MÉTRIQUES ==========

double cCompression::EQM(int** Bloc8x8) {
    // Implémentation simplifiée
    return 0.0;
}

double cCompression::Taux_Compression(int** Bloc8x8) {
    int coefficientsNonNuls = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (Bloc8x8[i][j] != 0) {
                coefficientsNonNuls++;
            }
        }
    }

    double taux = (1.0 - static_cast<double>(coefficientsNonNuls) / 64.0) * 100.0;
    return taux;
}

// ========== FONCTIONS RLE ==========

int cCompression::RLE_Block(int** Img_Quant, int DC_precedent, char* Trame) {
    int index = 0;

    // 1. Encoder le coefficient DC (différence avec le précédent)
    int DC = Img_Quant[0][0];
    int DC_diff = DC - DC_precedent;
    Trame[index++] = static_cast<char>(DC_diff);

    // 2. Parcours en zigzag des coefficients AC
    int zigzag[64][2] = {
        {0,0}, {0,1}, {1,0}, {2,0}, {1,1}, {0,2}, {0,3}, {1,2},
        {2,1}, {3,0}, {4,0}, {3,1}, {2,2}, {1,3}, {0,4}, {0,5},
        {1,4}, {2,3}, {3,2}, {4,1}, {5,0}, {6,0}, {5,1}, {4,2},
        {3,3}, {2,4}, {1,5}, {0,6}, {0,7}, {1,6}, {2,5}, {3,4},
        {4,3}, {5,2}, {6,1}, {7,0}, {7,1}, {6,2}, {5,3}, {4,4},
        {3,5}, {2,6}, {1,7}, {2,7}, {3,6}, {4,5}, {5,4}, {6,3},
        {7,2}, {7,3}, {6,4}, {5,5}, {4,6}, {3,7}, {4,7}, {5,6},
        {6,5}, {7,4}, {7,5}, {6,6}, {5,7}, {6,7}, {7,6}, {7,7}
    };

    int runlength = 0;
    bool finBloc = false;

    // Parcourir du 2ème au 64ème coefficient
    for (int i = 1; i < 64; i++) {
        int x = zigzag[i][0];
        int y = zigzag[i][1];
        int coeff = Img_Quant[x][y];

        if (coeff == 0) {
            runlength++;
            // Si c'est le dernier coefficient
            if (i == 63) {
                Trame[index++] = static_cast<char>(runlength);
                Trame[index++] = 0;
                finBloc = true;
            }
        } else {
            Trame[index++] = static_cast<char>(runlength);
            Trame[index++] = static_cast<char>(coeff);
            runlength = 0;
        }
    }

    if (!finBloc) {
        Trame[index++] = 0; // runlength
        Trame[index++] = 0; // amplitude (fin de bloc)
    }

    return index;
}

int cCompression::RLE(int* Trame) {
    if (!mBuffer || mLargeur == 0 || mHauteur == 0) {
        return 0;
    }

    int totalBlocsX = mLargeur / 8;
    int totalBlocsY = mHauteur / 8;
    int indexTotal = 0;
    int DC_precedent = 0;

    // Allocation temporaire pour les blocs
    unsigned char blocData[8][8];
    char blocChar[8][8];
    double dct[8][8];
    int quant[8][8];
    char* charPtrs[8];
    double* doublePtrs[8];
    int* intPtrs[8];

    for (int i = 0; i < 8; i++) {
        charPtrs[i] = blocChar[i];
        doublePtrs[i] = dct[i];
        intPtrs[i] = quant[i];
    }

    char trameBloc[128];

    for (int by = 0; by < totalBlocsY; by++) {
        for (int bx = 0; bx < totalBlocsX; bx++) {
            // 1. Lire le bloc
            lireBloc(bx * 8, by * 8, blocData);

            // 2. Conversion en char (-128 à 127)
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    blocChar[i][j] = static_cast<char>(blocData[i][j] - 128);
                }
            }

            // 3. Calculer DCT
            Calcul_DCT_Block(charPtrs, doublePtrs);

            // 4. Quantifier
            quant_JPEG(doublePtrs, intPtrs);

            // 5. RLE sur le bloc
            int longueurBloc = RLE_Block(intPtrs, DC_precedent, trameBloc);

            // 6. Mettre à jour DC précédent
            DC_precedent = quant[0][0];

            // 7. Copier dans la trame totale
            for (int i = 0; i < longueurBloc; i++) {
                Trame[indexTotal++] = trameBloc[i];
            }
        }
    }

    return indexTotal;
}

// ========== FONCTIONS AUXILIAIRES ==========

void cCompression::decalageNiveau(char** bloc) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            bloc[i][j] = static_cast<char>(bloc[i][j] - 128);
        }
    }
}

void cCompression::decalageNiveauInverse(char** bloc) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            bloc[i][j] = static_cast<char>(bloc[i][j] + 128);
        }
    }
}

void cCompression::lireBloc(unsigned int x, unsigned int y, unsigned char bloc[8][8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned int px = x + j;
            unsigned int py = y + i;

            if (px < mLargeur && py < mHauteur && mBuffer) {
                bloc[i][j] = mBuffer[py][px];
            } else {
                bloc[i][j] = 0;
            }
        }
    }
}

void cCompression::ecrireBloc(unsigned int x, unsigned int y, unsigned char bloc[8][8]) {
    if (!mBuffer) return;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned int px = x + j;
            unsigned int py = y + i;

            if (px < mLargeur && py < mHauteur) {
                mBuffer[py][px] = bloc[i][j];
            }
        }
    }
}

bool cCompression::chargerImage(const char* nomFichier) {
    ifstream fichier(nomFichier, ios::binary);
    if (!fichier) {
        cerr << "Erreur: Impossible d'ouvrir le fichier " << nomFichier << endl;
        return false;
    }

    // Pour l'exemple, on suppose une image 256x256
    mLargeur = 256;
    mHauteur = 256;
    allocBuffer(mLargeur, mHauteur);

    for (unsigned int i = 0; i < mHauteur; i++) {
        fichier.read(reinterpret_cast<char*>(mBuffer[i]), mLargeur);
    }

    fichier.close();
    return true;
}

bool cCompression::chargerImageTexte(const char* nomFichier, unsigned int largeur, unsigned int hauteur) {
    ifstream fichier(nomFichier);
    if (!fichier) {
        cerr << "Erreur: Impossible d'ouvrir " << nomFichier << endl;
        return false;
    }

    // Allouer le buffer
    allocBuffer(largeur, hauteur);

    // Lecture des valeurs
    for (unsigned int i = 0; i < hauteur; i++) {
        for (unsigned int j = 0; j < largeur; j++) {
            int valeur;
            fichier >> valeur;

            // Saturation
            if (valeur < 0) valeur = 0;
            if (valeur > 255) valeur = 255;

            mBuffer[i][j] = static_cast<unsigned char>(valeur);
        }
    }

    fichier.close();

    cout << "Image texte chargée: " << nomFichier << " ("
         << largeur << "x" << hauteur << ")" << endl;

    return true;
}

bool cCompression::compresser(const char* nomFichierSortie) {
    if (!mBuffer) {
        cerr << "Erreur: Aucune image chargée" << endl;
        return false;
    }

    // 1. Préparer la trame RLE
    int tailleMaxTrame = (mLargeur / 8) * (mHauteur / 8) * 128;
    int* trame = new int[tailleMaxTrame];

    // 2. Calculer la trame RLE
    int longueurTrame = RLE(trame);

    // 3. Sauvegarder
    ofstream fichier(nomFichierSortie, ios::binary);
    if (!fichier) {
        delete[] trame;
        return false;
    }

    // Écrire l'en-tête simplifié
    fichier.write(reinterpret_cast<const char*>(&mLargeur), sizeof(mLargeur));
    fichier.write(reinterpret_cast<const char*>(&mHauteur), sizeof(mHauteur));
    fichier.write(reinterpret_cast<const char*>(&mQualite), sizeof(mQualite));
    fichier.write(reinterpret_cast<const char*>(&longueurTrame), sizeof(longueurTrame));

    // Écrire la trame
    fichier.write(reinterpret_cast<const char*>(trame), longueurTrame * sizeof(int));

    fichier.close();
    delete[] trame;

    cout << "Compression terminée: " << nomFichierSortie << endl;
    cout << "Taille trame: " << longueurTrame << " entiers" << endl;

    return true;
}

bool cCompression::sauvegarderImage(const char* nomFichier) {
    if (!mBuffer) {
        return false;
    }

    ofstream fichier(nomFichier, ios::binary);
    if (!fichier) {
        return false;
    }

    for (unsigned int i = 0; i < mHauteur; i++) {
        fichier.write(reinterpret_cast<const char*>(mBuffer[i]), mLargeur);
    }

    fichier.close();
    return true;
}
