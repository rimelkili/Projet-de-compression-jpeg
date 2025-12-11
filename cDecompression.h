/**
 * @file cDecompression.h
 * @brief Déclaration de la classe cDecompression pour la décompression JPEG
 * @author EL KILI Rim
 * @date 2025
 * @version 1.0
 */

#ifndef CDECOMPRESSION_H
#define CDECOMPRESSION_H

#include "cCompression.h"

/**
 * @class cDecompression
 * @brief Classe pour la décompression d'images JPEG
 *
 * Cette classe hérite de cCompression et implémente les fonctionnalités
 * de décompression JPEG.
 */
class cDecompression : public cCompression {
public:
    /**
     * @brief Constructeur par défaut
     */
    cDecompression();

    /**
     * @brief Constructeur avec paramètres
     * @param largeur Largeur de l'image
     * @param hauteur Hauteur de l'image
     * @param qualite Qualité de compression
     */
    cDecompression(unsigned int largeur, unsigned int hauteur, unsigned int qualite = 50);

    /**
     * @brief Décompresse une image à partir d'un fichier
     * @param Nom_Fichier_compresse Chemin du fichier compressé
     * @return Image décompressée
     */
    char** Decompression_JPEG(const char* Nom_Fichier_compresse);

    /**
     * @brief Décode une trame RLE
     * @param Trame Trame RLE d'entrée
     * @param longueur Longueur de la trame
     * @param nbBlocsX Nombre de blocs en largeur
     * @param nbBlocsY Nombre de blocs en hauteur
     * @param blocs Tableau de sortie pour les blocs
     */
    void decoder_RLE(int* Trame, int longueur, int nbBlocsX, int nbBlocsY, int blocs[][64]);

    /**
     * @brief Reconstruit l'image à partir des blocs décompressés
     * @param blocs Blocs décompressés
     * @param nbBlocsX Nombre de blocs en largeur
     * @param nbBlocsY Nombre de blocs en hauteur
     */
    void reconstruireImage(int blocs[][64], int nbBlocsX, int nbBlocsY);

    /**
     * @brief Teste le cycle compression/décompression
     * @param fichierEntree Fichier image d'entrée
     * @param qualite Qualité de compression
     * @return true si le test est réussi
     */
    bool testerCompressionDecompression(const char* fichierEntree, int qualite);
};

#endif // CDECOMPRESSION_H
