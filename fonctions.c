#include "fonctions.h"

// Fonction pour vérifier si un nombre est premier
int estPremier(int n){
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (int i = 5; i * i <= n; i += 6){
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;
    }
    return 1;
}

// Fonction pour calculer phi(n) = (p-1) * (q-1)
int calculerPhi(int p, int q){
    return (p - 1) * (q - 1);
}

// Fonction pour calculer l'exponentiation modulaire: (base^exposant) mod modulo
long long expositionModulaire(long long base, long long exposant, long long modulo){
    long long resultat = 1;
    base = base % modulo;
    while (exposant > 0){
        if (exposant % 2 == 1){
            resultat = (resultat * base) % modulo;
        }
        exposant = exposant >> 1;
        base = (base * base) % modulo;
    }
    return resultat;
}

// Fonction pour générer n et phi(n)
void genererParametres(int* n, int* phi){
    // Liste de nombres premiers plus grands pour gérer tous les caractères UTF-8 (0-255)
    int premiers[] = {151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257};
    int tailleListe = sizeof(premiers) / sizeof(premiers[0]);
    
    // Choisir deux nombres premiers différents aléatoirement
    int index1 = rand() % tailleListe;
    int index2 = rand() % tailleListe;
    while (index1 == index2){
        index2 = rand() % tailleListe;
    }
    
    int p = premiers[index1];
    int q = premiers[index2];
    
    *n = p * q;
    *phi = calculerPhi(p, q);
}

// Fonction PGCD (Plus Grand Commun Diviseur)
int pgcd(int a, int b){
    if (b == 0){
        return a;
    } else {
        return pgcd(b, a % b);
    }
}

// Fonction pour générer l'exposant public e
int genererExposantPublic(int phi){
    int e = 3;
    while (e < phi){
        if (pgcd(e, phi) == 1){
            break;
        } else {
            e += 2;
        }
    }
    return e;
}

// Fonction pour calculer l'exposant privé d
long long calculerExposantPrive(int e, int phi){
    long long d = 1;
    while ((d * e) % phi != 1){
        d++;
        if (d > phi) return -1; // éviter les boucles infinies
    }
    return d;
}

// Fonction pour chiffrer un message
void chiffrer(char* message, int e, int n, long long* messageChiffre, int* longueur){
    *longueur = strlen(message);
    for (int i = 0; i < *longueur; i++){
        // Utiliser la valeur ASCII/UTF-8 du caractère directement
        int m = (unsigned char)message[i];
        // Chiffrer: C = M^e mod n
        messageChiffre[i] = expositionModulaire(m, e, n);
    }
}

// Fonction pour déchiffrer un message
void dechiffrer(long long* messageChiffre, long long d, int n, char* messageDechiffre, int longueur){
    for (int i = 0; i < longueur; i++){
        // Déchiffrer: M = C^d mod n
        long long m = expositionModulaire(messageChiffre[i], d, n);
        // Reconvertir en caractère directement
        messageDechiffre[i] = (char)m;
    }
    messageDechiffre[longueur] = '\0';
}
