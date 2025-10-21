#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fonctions.h"

// Structure pour un message diffusé
typedef struct {
    char messageChiffre[4096];  // Message chiffré
    char hashMotDePasse[256];   // Hash du mot de passe
    time_t timestamp;           // Moment de l'envoi
    int tentativesRestantes;    // Nombre de tentatives avant auto-destruction
    int n_public;               // Clé publique n
    int e_public;               // Clé publique e
    char expediteur[256];       // Nom/IP de l'expéditeur
} MessageDiffuse;

// Structure pour stocker les informations d'un message
typedef struct {
    int n;              // Module RSA
    int e;              // Exposant public
    long long d;        // Exposant privé
    char destinataire[256]; // Adresse IP du destinataire
} ConfigurationMessagerie;

// Structure pour stocker un message avec sa durée de vie
typedef struct {
    char messageChiffre[4096];  // Message chiffré encodé
    time_t timestamp;           // Horodatage de création
    int ttl;                    // Durée de vie en secondes
    char expediteur[256];       // Adresse IP de l'expéditeur
} MessageAvecTTL;

// Fonction pour créer un message avec TTL
void creerMessageAvecTTL(MessageAvecTTL* msg, char* messageChiffre, int ttlSecondes, char* expediteur) {
    strcpy(msg->messageChiffre, messageChiffre);
    msg->timestamp = time(NULL);
    msg->ttl = ttlSecondes;
    strcpy(msg->expediteur, expediteur);
}

// Fonction pour vérifier si un message a expiré
int messageEstExpire(MessageAvecTTL* msg) {
    time_t maintenant = time(NULL);
    double difference = difftime(maintenant, msg->timestamp);
    return (int)difference >= msg->ttl;
}

// Fonction pour obtenir le temps restant
int obtenirTempsRestant(MessageAvecTTL* msg) {
    time_t maintenant = time(NULL);
    double difference = difftime(maintenant, msg->timestamp);
    int restant = msg->ttl - (int)difference;
    return restant > 0 ? restant : 0;
}

// Fonction pour formater le temps restant
void formaterTempsRestant(int secondes, char* buffer) {
    if (secondes >= 3600) {
        int heures = secondes / 3600;
        int minutes = (secondes % 3600) / 60;
        int sec = secondes % 60;
        sprintf(buffer, "%dh %dm %ds", heures, minutes, sec);
    } else if (secondes >= 60) {
        int minutes = secondes / 60;
        int sec = secondes % 60;
        sprintf(buffer, "%dm %ds", minutes, sec);
    } else {
        sprintf(buffer, "%ds", secondes);
    }
}

// Fonction pour sauvegarder un message avec TTL
int sauvegarderMessageAvecTTL(MessageAvecTTL* msg) {
    FILE* file = fopen("message_temp.txt", "w");
    if (file == NULL) {
        return 0;
    }
    
    fprintf(file, "%s\n", msg->messageChiffre);
    fprintf(file, "%ld\n", (long)msg->timestamp);
    fprintf(file, "%d\n", msg->ttl);
    fprintf(file, "%s\n", msg->expediteur);
    
    fclose(file);
    
    // Créer un fichier d'info lisible
    FILE* info = fopen("message_info.txt", "w");
    if (info != NULL) {
        fprintf(info, "========================================\n");
        fprintf(info, "   INFORMATIONS DU MESSAGE ENVOYE      \n");
        fprintf(info, "========================================\n\n");
        fprintf(info, "Expediteur : %s\n", msg->expediteur);
        fprintf(info, "Date/Heure : %s", ctime(&msg->timestamp));
        fprintf(info, "Duree de vie : %d secondes\n", msg->ttl);
        fprintf(info, "Expire le : %s", ctime(&(time_t){msg->timestamp + msg->ttl}));
        fprintf(info, "\nMessage chiffre :\n%s\n", msg->messageChiffre);
        fprintf(info, "========================================\n");
        fclose(info);
    }
    
    return 1;
}

// Fonction pour charger un message avec TTL
int chargerMessageAvecTTL(MessageAvecTTL* msg) {
    FILE* file = fopen("message_temp.txt", "r");
    if (file == NULL) {
        return 0;
    }
    
    char buffer[4096];
    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        strcpy(msg->messageChiffre, buffer);
    } else {
        fclose(file);
        return 0;
    }
    
    long timestamp;
    if (fscanf(file, "%ld\n", &timestamp) == 1) {
        msg->timestamp = (time_t)timestamp;
    } else {
        fclose(file);
        return 0;
    }
    
    if (fscanf(file, "%d\n", &msg->ttl) != 1) {
        fclose(file);
        return 0;
    }
    
    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        strcpy(msg->expediteur, buffer);
    }
    
    fclose(file);
    return 1;
}

// Fonction simple de hachage pour le mot de passe
unsigned long hashMotDePasse(char* motDePasse) {
    unsigned long hash = 5381;
    int c;
    while ((c = *motDePasse++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Fonction pour obtenir l'adresse de broadcast du réseau local
void obtenirAdresseBroadcast(char* adresseBroadcast) {
    // Par défaut, utiliser l'adresse de broadcast locale
    // En pratique, on devrait détecter automatiquement le réseau
    strcpy(adresseBroadcast, "255.255.255.255");
}

// Fonction pour sauvegarder un message diffusé
void sauvegarderMessageDiffuse(MessageDiffuse* msg) {
    FILE* fichier = fopen("messages_diffuses.dat", "ab");
    if (fichier == NULL) {
        printf("[ERREUR] Impossible de sauvegarder le message\n");
        return;
    }
    fwrite(msg, sizeof(MessageDiffuse), 1, fichier);
    fclose(fichier);
    printf("[OK] Message sauvegarde pour diffusion\n");
}

// Fonction pour charger tous les messages diffusés
int chargerMessagesDiffuses(MessageDiffuse* messages, int maxMessages) {
    FILE* fichier = fopen("messages_diffuses.dat", "rb");
    if (fichier == NULL) {
        return 0; // Aucun message
    }
    
    int count = 0;
    while (count < maxMessages && fread(&messages[count], sizeof(MessageDiffuse), 1, fichier) == 1) {
        count++;
    }
    fclose(fichier);
    return count;
}

// Fonction pour mettre à jour un message (après tentative échouée)
void mettreAJourMessage(MessageDiffuse* messages, int count) {
    FILE* fichier = fopen("messages_diffuses.dat", "wb");
    if (fichier == NULL) {
        printf("[ERREUR] Impossible de mettre a jour les messages\n");
        return;
    }
    
    for (int i = 0; i < count; i++) {
        if (messages[i].tentativesRestantes > 0) {
            fwrite(&messages[i], sizeof(MessageDiffuse), 1, fichier);
        }
    }
    fclose(fichier);
}

// Fonction pour convertir un message chiffré en chaîne de caractères pour transmission
void messageChiffreVersChaine(long long* messageChiffre, int longueur, char* buffer) {
    buffer[0] = '\0';
    char temp[32];
    for (int i = 0; i < longueur; i++) {
        sprintf(temp, "%lld", messageChiffre[i]);
        strcat(buffer, temp);
        if (i < longueur - 1) {
            strcat(buffer, ",");
        }
    }
}

// Fonction pour convertir une chaîne reçue en message chiffré
int chaineVersMessageChiffre(char* buffer, long long* messageChiffre) {
    int longueur = 0;
    char* token = strtok(buffer, ",");
    while (token != NULL) {
        messageChiffre[longueur] = atoll(token);
        longueur++;
        token = strtok(NULL, ",");
    }
    return longueur;
}

// Fonction pour diffuser un message à tout le réseau avec mot de passe
void diffuserMessageSecurise(ConfigurationMessagerie* config, char* message, char* motDePasse, char* nomExpediteur) {
    printf("\n========================================\n");
    printf("   PUBLICATION MONDIALE DE MESSAGE      \n");
    printf("========================================\n\n");
    
    // Chiffrement du message avec RSA
    long long messageChiffre[1024];
    int longueur;
    chiffrer(message, config->e, config->n, messageChiffre, &longueur);
    
    printf("MESSAGE ORIGINAL:\n");
    printf("  \"%s\"\n\n", message);
    
    // Conversion en chaîne
    char messageEncode[4096];
    messageChiffreVersChaine(messageChiffre, longueur, messageEncode);
    
    // Créer le message diffusé
    MessageDiffuse msgDiffuse;
    strcpy(msgDiffuse.messageChiffre, messageEncode);
    sprintf(msgDiffuse.hashMotDePasse, "%lu", hashMotDePasse(motDePasse));
    msgDiffuse.timestamp = time(NULL);
    msgDiffuse.tentativesRestantes = 3; // 3 tentatives avant auto-destruction
    msgDiffuse.n_public = config->n;
    msgDiffuse.e_public = config->e;
    strcpy(msgDiffuse.expediteur, nomExpediteur);
    
    printf("SECURITE:\n");
    printf("  Protection par mot de passe: OUI\n");
    printf("  Tentatives autorisees: %d\n", msgDiffuse.tentativesRestantes);
    printf("  Hash du mot de passe: %s\n\n", msgDiffuse.hashMotDePasse);
    
    printf("CHIFFREMENT RSA:\n");
    printf("  Cle publique: (n=%d, e=%d)\n", config->n, config->e);
    printf("  Taille du message: %d octets\n\n", (int)strlen(messageEncode));
    
    // Sauvegarder le message
    sauvegarderMessageDiffuse(&msgDiffuse);
    
    printf("DIFFUSION MONDIALE:\n");
    printf("  Mode: PUBLICATION INTERNET\n");
    printf("  Portee: Monde entier\n");
    printf("  Expediteur: %s\n", nomExpediteur);
    printf("  Timestamp: %s\n", ctime(&msgDiffuse.timestamp));
    
    // Simuler la publication mondiale (en réalité envoi vers serveur cloud)
    printf("  [Simulation] Envoi vers serveurs mondiaux...\n");
    printf("  [Simulation] DNS: messagerie-mondiale.cloud\n");
    printf("  [Simulation] Serveurs: USA, Europe, Asie, Afrique\n");
    
    printf("\n[SUCCES] Message publie et accessible mondialement!\n");
    printf("[INFO] Toute personne avec votre cle publique peut le recevoir\n");
    printf("[ALERTE] Le message s'auto-detruira apres 3 tentatives incorrectes!\n");
    printf("\n========================================\n");
}

// Fonction pour lire un message diffusé avec vérification du mot de passe
void lireMessageDiffuse(long long d_prive, int n_prive) {
    printf("\n========================================\n");
    printf("   LECTURE DES MESSAGES MONDIAUX        \n");
    printf("========================================\n\n");
    
    // Charger tous les messages
    MessageDiffuse messages[100];
    int count = chargerMessagesDiffuses(messages, 100);
    
    if (count == 0) {
        printf("[INFO] Aucun message publie disponible\n");
        printf("[ASTUCE] Les messages apparaissent apres publication mondiale\n\n");
        printf("========================================\n");
        return;
    }
    
    printf("MESSAGES DISPONIBLES: %d\n\n", count);
    
    // Afficher la liste des messages
    for (int i = 0; i < count; i++) {
        printf("%d. De: %s\n", i + 1, messages[i].expediteur);
        printf("   Date: %s", ctime(&messages[i].timestamp));
        printf("   Tentatives restantes: %d\n", messages[i].tentativesRestantes);
        printf("   Cle publique: (n=%d, e=%d)\n\n", messages[i].n_public, messages[i].e_public);
    }
    
    printf("========================================\n");
    printf("Choisir un message (1-%d) ou 0 pour annuler: ", count);
    int choix;
    scanf("%d", &choix);
    getchar(); // Consommer le '\n'
    
    if (choix < 1 || choix > count) {
        printf("\n[ANNULE] Retour au menu\n");
        return;
    }
    
    MessageDiffuse* msgChoisi = &messages[choix - 1];
    
    printf("\n========================================\n");
    printf("   DECHIFFREMENT DU MESSAGE            \n");
    printf("========================================\n\n");
    
    printf("MESSAGE SELECTIONNE:\n");
    printf("  Expediteur: %s\n", msgChoisi->expediteur);
    printf("  Date: %s", ctime(&msgChoisi->timestamp));
    printf("  Tentatives restantes: %d\n\n", msgChoisi->tentativesRestantes);
    
    // Vérifier la compatibilité des clés
    if (msgChoisi->n_public != n_prive) {
        printf("[ERREUR] Cles incompatibles!\n");
        printf("         Message n=%d, Votre n=%d\n", msgChoisi->n_public, n_prive);
        printf("[INFO] Vous ne pouvez pas dechiffrer ce message\n\n");
        printf("========================================\n");
        return;
    }
    
    // Demander le mot de passe
    printf("Entrez le mot de passe: ");
    char motDePasse[256];
    fgets(motDePasse, sizeof(motDePasse), stdin);
    motDePasse[strcspn(motDePasse, "\n")] = '\0';
    
    // Vérifier le mot de passe
    char hashSaisi[256];
    sprintf(hashSaisi, "%lu", hashMotDePasse(motDePasse));
    
    if (strcmp(hashSaisi, msgChoisi->hashMotDePasse) != 0) {
        printf("\n[ERREUR] Mot de passe incorrect!\n");
        msgChoisi->tentativesRestantes--;
        
        if (msgChoisi->tentativesRestantes <= 0) {
            printf("[ALERTE] Message AUTO-DETRUIT!\n");
            printf("[INFO] Le message a ete supprime definitivement\n");
        } else {
            printf("[AVERTISSEMENT] Tentatives restantes: %d\n", msgChoisi->tentativesRestantes);
        }
        
        // Mettre à jour le fichier
        mettreAJourMessage(messages, count);
        printf("\n========================================\n");
        return;
    }
    
    // Mot de passe correct - Déchiffrer le message
    printf("\n[OK] Mot de passe correct!\n\n");
    
    // Décoder le message chiffré
    long long messageChiffre[1024];
    char bufferCopie[4096];
    strcpy(bufferCopie, msgChoisi->messageChiffre);
    int longueur = chaineVersMessageChiffre(bufferCopie, messageChiffre);
    
    printf("DECHIFFREMENT:\n");
    printf("  Utilisation de la cle privee: (n=%d, d=%lld)\n", n_prive, d_prive);
    printf("  Nombres a dechiffrer: %d\n\n", longueur);
    
    // Déchiffrer
    char messageDechiffre[1024];
    dechiffrer(messageChiffre, d_prive, n_prive, messageDechiffre, longueur);
    
    printf("========================================\n");
    printf("  MESSAGE DECHIFFRE:                   \n");
    printf("========================================\n\n");
    printf("  De: %s\n", msgChoisi->expediteur);
    printf("  Date: %s\n", ctime(&msgChoisi->timestamp));
    printf("  +--------------------------------------+\n");
    printf("  | %-36s |\n", messageDechiffre);
    printf("  +--------------------------------------+\n\n");
    printf("========================================\n");
    printf("[SUCCES] Message lu avec succes!\n");
    printf("========================================\n");
}

// Fonction pour envoyer un message via ping (simulation)
int envoyerMessageParPing(char* destinataire, char* messageEncode, int tailleMessage) {
    printf("\n[PING] Envoi de donnees vers %s...\n", destinataire);
    
    // Création d'un fichier temporaire avec le message
    FILE* temp = fopen("message_temp.txt", "w");
    if (temp == NULL) {
        printf("[ERREUR] Impossible de creer le fichier temporaire\n");
        return 0;
    }
    fprintf(temp, "%s", messageEncode);
    fclose(temp);
    
    // Construction de la commande ping
    // On envoie plusieurs pings pour "transmettre" le message
    char commande[512];
    int nbPaquets = (tailleMessage / 32) + 1; // Diviser en paquets de 32 octets
    if (nbPaquets > 10) nbPaquets = 10; // Limiter à 10 paquets
    
    sprintf(commande, "ping -n %d -l %d %s > ping_result.txt", nbPaquets, tailleMessage, destinataire);
    
    printf("[PING] Execution de: ping -n %d -l %d %s\n", nbPaquets, tailleMessage, destinataire);
    printf("[PING] Taille des donnees simulees: %d octets\n", tailleMessage);
    printf("[PING] Nombre de paquets: %d\n", nbPaquets);
    
    int resultat = system(commande);
    
    // Vérifier le résultat
    FILE* resultFile = fopen("ping_result.txt", "r");
    if (resultFile != NULL) {
        char ligne[256];
        int succes = 0;
        while (fgets(ligne, sizeof(ligne), resultFile)) {
            if (strstr(ligne, "TTL=") != NULL || strstr(ligne, "temps=") != NULL) {
                succes = 1;
                printf("[PING] Reponse recue: %s", ligne);
            }
        }
        fclose(resultFile);
        
        if (succes) {
            printf("\n[SUCCES] Ping reussi! Le message a ete 'transmis'\n");
            printf("[INFO] En realite, le message chiffre serait encode dans les paquets ICMP\n");
            return 1;
        }
    }
    
    printf("\n[AVERTISSEMENT] Pas de reponse, mais le message chiffre est pret\n");
    printf("[INFO] Message sauvegarde dans: message_temp.txt\n");
    return 1;
}

// Fonction principale d'envoi de message chiffré avec TTL
void envoyerMessageChiffreAvecTTL(ConfigurationMessagerie* config, char* message, int ttlSecondes) {
    printf("\n========================================\n");
    printf("   ENVOI DE MESSAGE VIA INTERNET       \n");
    printf("========================================\n\n");
    
    // Chiffrement du message
    long long messageChiffre[1024];
    int longueur;
    chiffrer(message, config->e, config->n, messageChiffre, &longueur);
    
    printf("MESSAGE ORIGINAL:\n");
    printf("  \"%s\"\n\n", message);
    
    printf("CHIFFREMENT RSA:\n");
    printf("  Cle publique: (n=%d, e=%d)\n", config->n, config->e);
    printf("  Message chiffre: ");
    for (int i = 0; i < longueur && i < 10; i++) {
        printf("%lld ", messageChiffre[i]);
    }
    if (longueur > 10) printf("...");
    printf("\n\n");
    
    // Conversion en chaîne pour transmission
    char messageEncode[4096];
    messageChiffreVersChaine(messageChiffre, longueur, messageEncode);
    
    // Créer le message avec TTL
    MessageAvecTTL msgTTL;
    creerMessageAvecTTL(&msgTTL, messageEncode, ttlSecondes, "localhost");
    
    // Afficher les informations de durée de vie
    char tempsFormate[50];
    formaterTempsRestant(ttlSecondes, tempsFormate);
    
    printf("DUREE DE VIE DU MESSAGE:\n");
    printf("  TTL (Time To Live): %s\n", tempsFormate);
    printf("  Date de creation: %s", ctime(&msgTTL.timestamp));
    time_t expiration = msgTTL.timestamp + ttlSecondes;
    printf("  Date d'expiration: %s\n", ctime(&expiration));
    
    printf("ENCODAGE POUR TRANSMISSION:\n");
    printf("  Format: %.*s%s\n", 
           strlen(messageEncode) > 80 ? 80 : (int)strlen(messageEncode), 
           messageEncode,
           strlen(messageEncode) > 80 ? "..." : "");
    printf("  Taille totale: %d octets\n\n", (int)strlen(messageEncode));
    
    printf("TRANSMISSION INTERNET:\n");
    printf("  Destinataire: %s\n", config->destinataire);
    printf("  Protocole: TCP/IP via Internet\n");
    printf("  [Simulation] Routage global active\n");
    
    // Sauvegarder le message avec TTL
    sauvegarderMessageAvecTTL(&msgTTL);
    
    // Envoi via ping (simulation de connexion Internet)
    if (envoyerMessageParPing(config->destinataire, messageEncode, strlen(messageEncode))) {
        printf("\n[SUCCES] Message envoye via Internet et chiffre avec RSA!\n");
        printf("[INFO] Le message chiffre complet est:\n");
        printf("       %s\n", messageEncode);
        printf("\n[IMPORTANT] Ce message expirera dans %s\n", tempsFormate);
        printf("[INFO] Informations sauvegardees dans: message_info.txt\n");
    } else {
        printf("\n[ECHEC] Impossible d'envoyer le message\n");
    }
    
    printf("\n========================================\n");
}

// Fonction d'envoi de message chiffré (sans TTL - version simple)
void envoyerMessageChiffre(ConfigurationMessagerie* config, char* message) {
    // Version par défaut avec TTL de 1 heure
    envoyerMessageChiffreAvecTTL(config, message, 3600);
}

// Fonction pour recevoir et déchiffrer un message
void recevoirMessageChiffre(ConfigurationMessagerie* config, char* messageEncode) {
    printf("\n========================================\n");
    printf("  RECEPTION DE MESSAGE CHIFFRE PAR PING\n");
    printf("========================================\n\n");
    
    printf("MESSAGE RECU (ENCODE):\n");
    printf("  %.*s%s\n\n", 
           strlen(messageEncode) > 80 ? 80 : (int)strlen(messageEncode),
           messageEncode,
           strlen(messageEncode) > 80 ? "..." : "");
    
    // Conversion de la chaîne en tableau de nombres
    long long messageChiffre[1024];
    char bufferCopie[4096];
    strcpy(bufferCopie, messageEncode);
    int longueur = chaineVersMessageChiffre(bufferCopie, messageChiffre);
    
    printf("MESSAGE CHIFFRE (DECODE):\n");
    printf("  ");
    for (int i = 0; i < longueur && i < 10; i++) {
        printf("%lld ", messageChiffre[i]);
    }
    if (longueur > 10) printf("...");
    printf("\n  (%d nombres au total)\n\n", longueur);
    
    // Déchiffrement
    char messageDechiffre[1024];
    dechiffrer(messageChiffre, config->d, config->n, messageDechiffre, longueur);
    
    printf("DECHIFFREMENT RSA:\n");
    printf("  Cle privee: (n=%d, d=%lld)\n", config->n, config->d);
    printf("  Message dechiffre: \"%s\"\n\n", messageDechiffre);
    
    printf("========================================\n");
}

// Fonction d'initialisation de la configuration
void initialiserMessagerie(ConfigurationMessagerie* config, int n, int e, long long d, char* destinataire) {
    config->n = n;
    config->e = e;
    config->d = d;
    strcpy(config->destinataire, destinataire);
}

// Fonction pour attendre et recevoir un message via ping avec vérification TTL
int attendreReceptionPingAvecTTL(char* adresseEnvoyeur, MessageAvecTTL* msgTTL, int timeout) {
    printf("\n========================================\n");
    printf("   ATTENTE DE RECEPTION DE MESSAGE     \n");
    printf("========================================\n\n");
    printf("[INFO] En ecoute des pings depuis: %s\n", adresseEnvoyeur);
    printf("[INFO] Timeout: %d secondes\n\n", timeout);
    
    // Créer un fichier de log pour capturer les pings
    char commande[512];
    sprintf(commande, "ping -n %d -w 1000 %s > reception_temp.txt", timeout, adresseEnvoyeur);
    
    printf("[PING] Tentative de ping vers l'envoyeur pour verifier la connexion...\n");
    printf("[PING] Commande: ping -n %d %s\n\n", timeout, adresseEnvoyeur);
    
    system(commande);
    
    // Vérifier si un fichier de message a été créé par l'envoyeur
    printf("[INFO] Verification de la presence d'un message...\n");
    
    // Charger le message avec TTL
    if (chargerMessageAvecTTL(msgTTL)) {
        printf("[SUCCES] Message trouve!\n\n");
        
        // Vérifier si le message a expiré
        if (messageEstExpire(msgTTL)) {
            int tempsDepasse = (int)difftime(time(NULL), msgTTL->timestamp) - msgTTL->ttl;
            printf("[ERREUR] ❌ MESSAGE EXPIRE!\n");
            printf("         Le message a depasse sa duree de vie de %d secondes\n", tempsDepasse);
            printf("         Date d'envoi: %s", ctime(&msgTTL->timestamp));
            time_t expiration = msgTTL->timestamp + msgTTL->ttl;
            printf("         A expire le: %s", ctime(&expiration));
            printf("         Pour des raisons de securite, le message ne peut pas etre lu.\n\n");
            return 0;
        }
        
        // Afficher le temps restant
        int tempsRestant = obtenirTempsRestant(msgTTL);
        char tempsFormate[50];
        formaterTempsRestant(tempsRestant, tempsFormate);
        
        printf("[OK] Message valide!\n");
        printf("     Temps restant avant expiration: %s\n", tempsFormate);
        printf("     Date d'envoi: %s", ctime(&msgTTL->timestamp));
        printf("\n");
        
        return 1;
    }
    
    printf("[INFO] Aucun message recu de %s\n", adresseEnvoyeur);
    printf("[NOTE] En mode simulation, assurez-vous qu'un message a ete envoye\n");
    printf("       et se trouve dans message_temp.txt\n\n");
    
    return 0;
}

// Version simple (sans TTL) pour compatibilité
int attendreReceptionPing(char* adresseEnvoyeur, char* messageRecu, int timeout) {
    MessageAvecTTL msgTTL;
    if (attendreReceptionPingAvecTTL(adresseEnvoyeur, &msgTTL, timeout)) {
        strcpy(messageRecu, msgTTL.messageChiffre);
        return 1;
    }
    return 0;
}

// Fonction complète de réception avec déchiffrement automatique
void recevoirMessageAutomatique(char* adresseEnvoyeur, int n_public, int e_public, long long d_prive, int n_prive) {
    printf("\n========================================\n");
    printf("  RECEPTEUR DE MESSAGES SECURISES      \n");
    printf("========================================\n\n");
    
    printf("CONFIGURATION DU RECEPTEUR:\n");
    printf("  Adresse de l'envoyeur: %s\n", adresseEnvoyeur);
    printf("  Cle publique envoyeur: (n=%d, e=%d)\n", n_public, e_public);
    printf("  Votre cle privee: (n=%d, d=%lld)\n", n_prive, d_prive);
    printf("\n");
    
    // Vérifier que les clés correspondent
    if (n_public != n_prive) {
        printf("[ERREUR] Les parametres 'n' ne correspondent pas!\n");
        printf("         n_public=%d, n_prive=%d\n", n_public, n_prive);
        printf("[INFO] Vous devez avoir le meme 'n' que l'envoyeur pour dechiffrer\n\n");
        printf("========================================\n");
        return;
    }
    
    printf("[OK] Les parametres sont compatibles\n\n");
    
    // Attendre la réception
    char messageChiffreRecu[4096];
    if (attendreReceptionPing(adresseEnvoyeur, messageChiffreRecu, 3)) {
        printf("========================================\n");
        printf("  MESSAGE RECU - DECHIFFREMENT         \n");
        printf("========================================\n\n");
        
        printf("MESSAGE CHIFFRE RECU:\n");
        printf("  %.*s%s\n\n", 
               strlen(messageChiffreRecu) > 80 ? 80 : (int)strlen(messageChiffreRecu),
               messageChiffreRecu,
               strlen(messageChiffreRecu) > 80 ? "..." : "");
        
        // Décoder le message
        long long messageChiffre[1024];
        char bufferCopie[4096];
        strcpy(bufferCopie, messageChiffreRecu);
        int longueur = chaineVersMessageChiffre(bufferCopie, messageChiffre);
        
        printf("DECODAGE:\n");
        printf("  Nombres extraits: ");
        for (int i = 0; i < longueur && i < 10; i++) {
            printf("%lld ", messageChiffre[i]);
        }
        if (longueur > 10) printf("...");
        printf("\n  Total: %d nombres\n\n", longueur);
        
        // Déchiffrer
        char messageDechiffre[1024];
        dechiffrer(messageChiffre, d_prive, n_prive, messageDechiffre, longueur);
        
        printf("DECHIFFREMENT RSA:\n");
        printf("  Cle privee utilisee: (n=%d, d=%lld)\n", n_prive, d_prive);
        printf("\n");
        printf("  +--------------------------------------+\n");
        printf("  | MESSAGE DECHIFFRE:                   |\n");
        printf("  | %-36s |\n", messageDechiffre);
        printf("  +--------------------------------------+\n");
        printf("\n");
        
        printf("========================================\n");
        printf("[SUCCES] Message recu et dechiffre!\n");
        printf("========================================\n");
    } else {
        printf("========================================\n");
        printf("[ECHEC] Aucun message recu\n");
        printf("========================================\n");
    }
}

// Fonction pour sauvegarder les clés RSA
int sauvegarderClesRSA(int n, int e, long long d, int phi) {
    FILE* fichier = fopen("cles_rsa.dat", "wb");
    if (fichier == NULL) {
        return 0;
    }
    fwrite(&n, sizeof(int), 1, fichier);
    fwrite(&e, sizeof(int), 1, fichier);
    fwrite(&d, sizeof(long long), 1, fichier);
    fwrite(&phi, sizeof(int), 1, fichier);
    fclose(fichier);
    return 1;
}

// Fonction pour charger les clés RSA
int chargerClesRSA(int* n, int* e, long long* d, int* phi) {
    FILE* fichier = fopen("cles_rsa.dat", "rb");
    if (fichier == NULL) {
        return 0; // Fichier n'existe pas
    }
    
    int resultat = 1;
    if (fread(n, sizeof(int), 1, fichier) != 1) resultat = 0;
    if (fread(e, sizeof(int), 1, fichier) != 1) resultat = 0;
    if (fread(d, sizeof(long long), 1, fichier) != 1) resultat = 0;
    if (fread(phi, sizeof(int), 1, fichier) != 1) resultat = 0;
    
    fclose(fichier);
    return resultat;
}

// Fonction pour supprimer tous les messages diffusés
void supprimerTousLesMessages() {
    if (remove("messages_diffuses.dat") == 0) {
        printf("[OK] Tous les messages ont ete supprimes\n");
    } else {
        printf("[INFO] Aucun message a supprimer\n");
    }
}

// Fonction de nettoyage
void nettoyerMessagerie() {
    // Nettoyer les fichiers temporaires
    remove("message_temp.txt");
    remove("ping_result.txt");
    remove("reception_temp.txt");
}
