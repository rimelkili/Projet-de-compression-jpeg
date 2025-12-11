/**
 * @file cHuffman.h
 * @brief Déclaration de la classe cHuffman pour le codage de Huffman
 * @author EL KILI Rim
 * @date 2025
 * @version 1.0
 */

#ifndef CHUFFMAN_H
#define CHUFFMAN_H

#include <queue>
#include <vector>
#include <string>
#include <map>

/**
 * @struct sNoeud
 * @brief Structure représentant un nœud dans l'arbre de Huffman
 */
struct sNoeud {
    char mdonnee;           ///< Caractère stocké (ou 0 pour les nœuds internes)
    double mfreq;           ///< Fréquence d'apparition
    sNoeud* mgauche;        ///< Pointeur vers le fils gauche
    sNoeud* mdroit;         ///< Pointeur vers le fils droit

    /**
     * @brief Constructeur
     * @param d Caractère
     * @param f Fréquence
     */
    sNoeud(char d, double f)
        : mdonnee(d), mfreq(f), mgauche(nullptr), mdroit(nullptr) {}

    /**
     * @brief Constructeur pour nœud interne
     * @param f Fréquence totale
     * @param g Fils gauche
     * @param d Fils droit
     */
    sNoeud(double f, sNoeud* g, sNoeud* d)
        : mdonnee(0), mfreq(f), mgauche(g), mdroit(d) {}
};

/**
 * @struct compare
 * @brief Foncteur de comparaison pour la file de priorité
 */
struct compare {
    /**
     * @brief Compare deux nœuds par leur fréquence
     * @param gauche Premier nœud
     * @param droit Second nœud
     * @return true si gauche > droit (pour une file min)
     */
    bool operator()(sNoeud* gauche, sNoeud* droit) {
        return gauche->mfreq > droit->mfreq;
    }
};

/**
 * @class cHuffman
 * @brief Classe pour le codage de Huffman
 *
 * Implémente la construction de l'arbre de Huffman et le codage/décodage
 */
class cHuffman {
private:
    sNoeud* mRacine;                ///< Racine de l'arbre de Huffman
    char* mTrame;                   ///< Trame à encoder/décoder
    unsigned int mLongueur;         ///< Longueur de la trame
    std::map<char, std::string> mCodes;  ///< Table des codes Huffman

public:
    /**
     * @brief Constructeur par défaut
     */
    cHuffman();

    /**
     * @brief Constructeur avec initialisation
     * @param trame Trame à encoder
     * @param longueur Longueur de la trame
     */
    cHuffman(char* trame, unsigned int longueur);

    /**
     * @brief Destructeur
     */
    ~cHuffman();

    // Accesseurs
    sNoeud* getRacine() const;
    char* getTrame() const;
    unsigned int getLongueur() const;

    /**
     * @brief Construit l'arbre de Huffman à partir de données
     * @param donnees Tableau de caractères
     * @param frequences Tableau de fréquences
     * @param taille Nombre d'éléments
     */
    void HuffmanCodes(char* donnees, double* frequences, unsigned int taille);

    /**
     * @brief Affiche les codes Huffman
     * @param racine Racine de l'arbre (par défaut: mRacine)
     */
    void AfficherHuffman(sNoeud* racine = nullptr);

    /**
     * @brief Calcule l'histogramme d'une trame
     * @param trame Trame d'entrée
     * @param longueur Longueur de la trame
     * @param donnees Tableau de sortie pour les caractères uniques
     * @param frequences Tableau de sortie pour les fréquences
     * @return Nombre d'éléments uniques
     */
    static unsigned int Histogramme(char* trame, unsigned int longueur,
                                    char* donnees, double* frequences);

    /**
     * @brief Encode une trame avec Huffman
     * @param trameEntree Trame d'entrée
     * @param longueur Longueur de la trame
     * @param trameSortie Trame encodée (sortie)
     * @return Longueur de la trame encodée en bits
     */
    unsigned int encoder(char* trameEntree, unsigned int longueur,
                         std::vector<bool>& trameSortie);

    /**
     * @brief Décode une trame avec Huffman
     * @param trameEntree Trame encodée
     * @param longueurBits Longueur en bits
     * @param trameSortie Trame décodée (sortie)
     * @return Longueur de la trame décodée
     */
    unsigned int decoder(std::vector<bool>& trameEntree, unsigned int longueurBits,
                         char* trameSortie);

private:
    /**
     * @brief Génère les codes Huffman récursivement
     * @param racine Nœud courant
     * @param code Code accumulé
     */
    void genererCodes(sNoeud* racine, std::string code);

    /**
     * @brief Supprime récursivement l'arbre
     * @param noeud Nœud à supprimer
     */
    void supprimerArbre(sNoeud* noeud);

    /**
     * @brief Copie récursivement un arbre
     * @param original Arbre original
     * @return Copie de l'arbre
     */
    sNoeud* copierArbre(sNoeud* original);
};

#endif // CHUFFMAN_H
