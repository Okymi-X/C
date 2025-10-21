#ifndef FONCTIONS_H
#define FONCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <locale.h>

// Déclarations des fonctions
int estPremier(int n);
int calculerPhi(int p, int q);  
long long expositionModulaire(long long base, long long exposant, long long modulo);
void genererParametres(int* n, int* phi);
int pgcd(int a, int b);
int genererExposantPublic(int phi);
long long calculerExposantPrive(int e, int phi);
void chiffrer(char* message, int e, int n, long long* messageChiffre, int* longueur);
void dechiffrer(long long* messageChiffre, long long d, int n, char* messageDechiffre, int longueur);

#endif

