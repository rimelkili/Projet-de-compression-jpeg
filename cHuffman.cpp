/**
 * @file cHuffman.cpp
 * @brief Implémentation de la classe cHuffman
 * @author EL KILI Rim
 * @date 2025
 * @version 1.0
 */

#include "cHuffman.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

// ========== CONSTRUCTEURS ET DESTRUCTEUR ==========

cHuffman::cHuffman()
    : mRacine(nullptr), mTrame(nullptr), mLongueur(0) {
}

cHuffman::cHuffman(char* trame, unsigned int longueur)
    : mTrame(trame), mLongueur(longueur), mRacine(nullptr) {
}

cHuffman::~cHuffman() {
    supprimerArbre(mRacine);
}

// ========== ACCESS ET MUTATEURS ==========

sNoeud* cHuffman::getRacine() const { return mRacine; }
char* cHuffman::getTrame() const { return mTrame; }
unsigned int cHuffman::getLongueur() const { return mLongueur; }

// ========== FONCTIONS HUFFMAN ==========

void cHuffman::HuffmanCodes(char* donnees, double* frequences, unsigned int taille) {
    // Créer une file de priorité min
    priority_queue<sNoeud*, vector<sNoeud*>, compare> file;

    // Créer les nœuds initiaux
    for (unsigned int i = 0; i < taille; i++) {
        file.push(new sNoeud(donnees[i], frequences[i]));
    }

    // Construire l'arbre de Huffman
    while (file.size() > 1) {
        // Extraire les deux nœuds avec les plus petites fréquences
        sNoeud* gauche = file.top();
        file.pop();

        sNoeud* droit = file.top();
        file.pop();

        // Créer un nouveau nœud interne
        double sommeFreq = gauche->mfreq + droit->mfreq;
        sNoeud* nouveau = new sNoeud(sommeFreq, gauche, droit);

        // Ajouter le nouveau nœud à la file
        file.push(nouveau);
    }

    // La racine est le dernier nœud
    if (!file.empty()) {
        mRacine = file.top();
        file.pop();
    }

    // Générer les codes
    genererCodes(mRacine, "");
}

void cHuffman::genererCodes(sNoeud* racine, std::string code) {
    if (!racine) return;

    // Si c'est une feuille, stocker le code
    if (!racine->mgauche && !racine->mdroit) {
        if (racine->mdonnee != 0) {
            mCodes[racine->mdonnee] = code;
        }
    } else {
        // Parcours récursif
        genererCodes(racine->mgauche, code + "0");
        genererCodes(racine->mdroit, code + "1");
    }
}

void cHuffman::AfficherHuffman(sNoeud* racine) {
    if (!racine) racine = mRacine;
    if (!racine) return;

    cout << "Codes Huffman:" << endl;
    cout << "==============" << endl;

    for (const auto& paire : mCodes) {
        cout << "  '" << paire.first << "' : " << paire.second << endl;
    }
    cout << endl;
}

unsigned int cHuffman::Histogramme(char* trame, unsigned int longueur,
                                   char* donnees, double* frequences) {
    if (!trame || longueur == 0) return 0;

    // Compter les occurrences
    map<char, int> comptage;
    for (unsigned int i = 0; i < longueur; i++) {
        comptage[trame[i]]++;
    }

    // Copier dans les tableaux de sortie
    unsigned int index = 0;
    for (const auto& paire : comptage) {
        donnees[index] = paire.first;
        frequences[index] = static_cast<double>(paire.second) / longueur;
        index++;
    }

    return index;
}

unsigned int cHuffman::encoder(char* trameEntree, unsigned int longueur,
                               vector<bool>& trameSortie) {
    if (!trameEntree || longueur == 0 || mCodes.empty()) return 0;

    trameSortie.clear();

    // Pour chaque caractère, ajouter son code
    for (unsigned int i = 0; i < longueur; i++) {
        char c = trameEntree[i];
        auto it = mCodes.find(c);
        if (it != mCodes.end()) {
            const string& code = it->second;
            for (char bit : code) {
                trameSortie.push_back(bit == '1');
            }
        }
    }

    return trameSortie.size();
}

unsigned int cHuffman::decoder(vector<bool>& trameEntree, unsigned int longueurBits,
                               char* trameSortie) {
    if (!mRacine || trameEntree.empty()) return 0;

    unsigned int indexSortie = 0;
    sNoeud* courant = mRacine;

    for (unsigned int i = 0; i < longueurBits; i++) {
        // Suivre le chemin dans l'arbre
        if (trameEntree[i]) {
            courant = courant->mdroit;
        } else {
            courant = courant->mgauche;
        }

        // Si on arrive à une feuille
        if (!courant->mgauche && !courant->mdroit) {
            trameSortie[indexSortie++] = courant->mdonnee;
            courant = mRacine; // Retour à la racine
        }
    }

    return indexSortie;
}

// ========== FONCTIONS AUXILIAIRES ==========

void cHuffman::supprimerArbre(sNoeud* noeud) {
    if (!noeud) return;

    supprimerArbre(noeud->mgauche);
    supprimerArbre(noeud->mdroit);
    delete noeud;
}

sNoeud* cHuffman::copierArbre(sNoeud* original) {
    if (!original) return nullptr;

    sNoeud* copie = new sNoeud(original->mdonnee, original->mfreq);
    copie->mgauche = copierArbre(original->mgauche);
    copie->mdroit = copierArbre(original->mdroit);

    return copie;
}
