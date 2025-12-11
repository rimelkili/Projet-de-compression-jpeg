/**
 * @file cCompression.h
 * @brief Déclaration de la classe cCompression pour la compression JPEG
 * @author EL KILI Rim
 * @date 2025
 * @version 1.0
 */

#ifndef CCOMPRESSION_H
#define CCOMPRESSION_H

#include <vector>

/**
 * @class cCompression
 * @brief Classe principale pour la compression JPEG d'images en niveaux de gris
 *
 * Cette classe implémente les étapes principales de la compression JPEG :
 * - Transformation DCT
 * - Quantification
 * - Encodage RLE
 * - Calcul des métriques de compression
 */
class cCompression {
private:
    unsigned int mLargeur;        ///< Largeur de l'image en pixels
    unsigned int mHauteur;        ///< Hauteur de l'image en pixels
    unsigned char** mBuffer;      ///< Buffer contenant les données de l'image
    unsigned int mQualite;        ///< Qualité de compression (0-100)

    /**
     * @brief Table de quantification JPEG standard pour la luminance
     */
    static const int TABLE_QUANT[8][8];

public:
    /**
     * @brief Constructeur par défaut
     */
    cCompression();

    /**
     * @brief Constructeur avec paramètres
     * @param largeur Largeur de l'image
     * @param hauteur Hauteur de l'image
     * @param qualite Qualité de compression (0-100, défaut=50)
     */
    cCompression(unsigned int largeur, unsigned int hauteur, unsigned int qualite = 50);

    /**
     * @brief Destructeur
     */
    ~cCompression();

    // Accesseurs et mutateurs
    unsigned int getLargeur() const;
    unsigned int getHauteur() const;
    unsigned int getQualite() const;
    unsigned char** getBuffer() const;  // NOUVEAU: Accès au buffer

    void setLargeur(unsigned int largeur);
    void setHauteur(unsigned int hauteur);
    void setQualite(unsigned int qualite);
    void setBuffer(unsigned char** buffer);

    /**
     * @brief Alloue le buffer pour l'image
     * @param largeur Largeur de l'image
     * @param hauteur Hauteur de l'image
     */
    void allocBuffer(unsigned int largeur, unsigned int hauteur);  // NOUVEAU

    /**
     * @brief Calcule la DCT d'un bloc 8x8
     * @param Block8 Bloc d'entrée 8x8 (valeurs entre -128 et 127)
     * @param DCT_Img Matrice de sortie pour les coefficients DCT
     */
    void Calcul_DCT_Block(char** Block8, double** DCT_Img);

    /**
     * @brief Calcule la DCT inverse d'un bloc 8x8
     * @param DCT_Img Coefficients DCT d'entrée
     * @param Block8 Bloc de sortie 8x8
     */
    void Calcul_IDCT(double** DCT_Img, char** Block8);

    /**
     * @brief Quantifie une matrice DCT selon la norme JPEG
     * @param img_DCT Matrice DCT d'entrée
     * @param Img_Quant Matrice quantifiée de sortie
     */
    void quant_JPEG(double** img_DCT, int** Img_Quant);

    /**
     * @brief Déquantifie une matrice selon la norme JPEG
     * @param Img_Quant Matrice quantifiée d'entrée
     * @param img_DCT Matrice déquantifiée de sortie
     */
    void dequant_JPEG(int** Img_Quant, double** img_DCT);

    /**
     * @brief Calcule l'écart quadratique moyen (EQM) d'un bloc
     * @param Bloc8x8 Bloc quantifié
     * @return L'EQM entre le bloc original et reconstruit
     */
    double EQM(int** Bloc8x8);

    /**
     * @brief Calcule le taux de compression d'un bloc
     * @param Bloc8x8 Bloc quantifié
     * @return Taux de compression en pourcentage
     */
    double Taux_Compression(int** Bloc8x8);

    /**
     * @brief Encode un bloc quantifié avec RLE
     * @param Img_Quant Bloc quantifié d'entrée
     * @param DC_precedent Valeur DC du bloc précédent
     * @param Trame Tableau de sortie pour la trame RLE
     * @return Longueur de la trame générée
     */
    int RLE_Block(int** Img_Quant, int DC_precedent, char* Trame);

    /**
     * @brief Encode toute l'image avec RLE
     * @param Trame Tableau de sortie pour la trame complète
     * @return Longueur totale de la trame
     */
    int RLE(int* Trame);

    /**
     * @brief Charge une image depuis un fichier
     * @param nomFichier Chemin du fichier image
     * @return true si chargement réussi, false sinon
     */
    bool chargerImage(const char* nomFichier);

    /**
     * @brief Charge une image depuis un fichier texte (comme lenna.img.txt)
     * @param nomFichier Chemin du fichier texte
     * @param largeur Largeur de l'image
     * @param hauteur Hauteur de l'image
     * @return true si chargement réussi
     */
    bool chargerImageTexte(const char* nomFichier, unsigned int largeur = 256, unsigned int hauteur = 256);  // NOUVEAU

    /**
     * @brief Sauvegarde l'image compressée
     * @param nomFichier Chemin du fichier de sortie
     * @return true si sauvegarde réussie, false sinon
     */
    bool sauvegarderImage(const char* nomFichier);

    /**
     * @brief Compresse l'image complète
     * @param nomFichierSortie Chemin du fichier compressé
     * @return true si compression réussie
     */
    bool compresser(const char* nomFichierSortie);

private:
    /**
     * @brief Calcule la table de quantification en fonction de la qualité
     * @param qualite Qualité souhaitée (0-100)
     * @param table Table de sortie 8x8
     */
    void calculerTableQuantification(int qualite, int table[8][8]);

    /**
     * @brief Effectue le décalage de niveau (0-255 vers -128-127)
     * @param bloc Bloc à traiter
     */
    void decalageNiveau(char** bloc);

    /**
     * @brief Effectue l'opération inverse du décalage de niveau
     * @param bloc Bloc à traiter
     */
    void decalageNiveauInverse(char** bloc);

    /**
     * @brief Lit un bloc 8x8 depuis l'image
     * @param x Position X du bloc
     * @param y Position Y du bloc
     * @param bloc Bloc de sortie
     */
    void lireBloc(unsigned int x, unsigned int y, unsigned char bloc[8][8]);

    /**
     * @brief Écrit un bloc 8x8 dans l'image
     * @param x Position X du bloc
     * @param y Position Y du bloc
     * @param bloc Bloc d'entrée
     */
    void ecrireBloc(unsigned int x, unsigned int y, unsigned char bloc[8][8]);
};

#endif // CCOMPRESSION_H
