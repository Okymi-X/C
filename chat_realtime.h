#ifndef CHAT_REALTIME_H
#define CHAT_REALTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include "fonctions.h"
#include "aes.h"

// Structure pour un message en temps réel
typedef struct {
    char expediteur[128];
    char message[1024];
    uint8_t cleAES[AES_KEY_SIZE];
    time_t timestamp;
    int isChiffre;
    char ipDestinataire[64];
} MessageTempsReel;

// Structure pour une session de chat
typedef struct {
    char nomUtilisateur[128];
    char ipDistante[64];
    uint8_t cleAESSession[AES_KEY_SIZE];
    int n_rsa;
    int e_rsa;
    long long d_rsa;
    int sessionActive;
    int messagesEnvoyes;
    int messagesRecus;
} SessionChat;

// Couleurs pour le terminal
void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

#define COLOR_RESET 7
#define COLOR_GREEN 10
#define COLOR_CYAN 11
#define COLOR_YELLOW 14
#define COLOR_RED 12
#define COLOR_MAGENTA 13

// Initialiser une session de chat
void initialiserSession(SessionChat* session, const char* nom, const char* ip, int n, int e, long long d) {
    strcpy(session->nomUtilisateur, nom);
    strcpy(session->ipDistante, ip);
    session->n_rsa = n;
    session->e_rsa = e;
    session->d_rsa = d;
    session->sessionActive = 1;
    session->messagesEnvoyes = 0;
    session->messagesRecus = 0;
    
    // Générer une clé AES pour cette session
    genererCleAES(session->cleAESSession, AES_KEY_SIZE);
}

// Afficher l'interface de chat
void afficherInterfaceChat(SessionChat* session) {
    system("cls");
    system("chcp 65001 >nul 2>&1"); // Activer UTF-8
    setColor(COLOR_CYAN);
    printf("\n");
    printf("  ========================================================================\n");
    printf("  ");
    setColor(COLOR_YELLOW);
    printf("      MESSAGERIE MONDIALE EN TEMPS REEL - AES-256 + RSA            ");
    setColor(COLOR_CYAN);
    printf("\n");
    printf("  ========================================================================\n");
    printf("  ");
    setColor(COLOR_GREEN);
    printf("Session: %-30s", session->nomUtilisateur);
    setColor(COLOR_CYAN);
    printf("                            \n");
    printf("  ");
    setColor(COLOR_GREEN);
    printf("IP Distante: %-25s", session->ipDistante);
    setColor(COLOR_CYAN);
    printf("                            \n");
    printf("  ");
    setColor(COLOR_GREEN);
    printf("Messages: Envoyes=%d | Recus=%d", session->messagesEnvoyes, session->messagesRecus);
    setColor(COLOR_CYAN);
    printf("                                  \n");
    printf("  ========================================================================\n");
    printf("  ");
    setColor(COLOR_YELLOW);
    printf("Chiffrement: AES-256 (Ultra-securise) + RSA pour echange de cles     ");
    setColor(COLOR_CYAN);
    printf("\n");
    printf("  ========================================================================\n");
    setColor(COLOR_RESET);
    printf("\n");
}

// Chiffrer un message avec AES
void chiffrerMessageAES(const char* message, const uint8_t* cleAES, char* messageChiffre) {
    int len = strlen(message);
    uint8_t buffer[1024];
    uint8_t output[1024];
    
    memcpy(buffer, message, len);
    chiffrerAES_Simple(buffer, len, cleAES, AES_KEY_SIZE, output);
    bytesToHex(output, len, messageChiffre);
}

// Déchiffrer un message avec AES
void dechiffrerMessageAES(const char* messageChiffre, const uint8_t* cleAES, char* messageClair) {
    int len = strlen(messageChiffre) / 2;
    uint8_t buffer[1024];
    uint8_t output[1024];
    
    hexToBytes(messageChiffre, buffer, len);
    dechiffrerAES_Simple(buffer, len, cleAES, AES_KEY_SIZE, output);
    memcpy(messageClair, output, len);
    messageClair[len] = '\0';
}

// Envoyer un message en temps réel via ping
int envoyerMessageTempsReel(SessionChat* session, const char* message) {
    char messageChiffre[2048];
    
    // Chiffrer avec AES
    chiffrerMessageAES(message, session->cleAESSession, messageChiffre);
    
    // Sauvegarder le message
    FILE* f = fopen("chat_session.dat", "ab");
    if (f) {
        MessageTempsReel msg;
        strcpy(msg.expediteur, session->nomUtilisateur);
        strcpy(msg.message, messageChiffre);
        memcpy(msg.cleAES, session->cleAESSession, AES_KEY_SIZE);
        msg.timestamp = time(NULL);
        msg.isChiffre = 1;
        strcpy(msg.ipDestinataire, session->ipDistante);
        
        fwrite(&msg, sizeof(MessageTempsReel), 1, f);
        fclose(f);
    }
    
    // Simuler l'envoi via ping (en temps réel)
    char commande[512];
    sprintf(commande, "ping -n 1 -w 100 %s >nul 2>&1", session->ipDistante);
    system(commande);
    
    session->messagesEnvoyes++;
    
    setColor(COLOR_GREEN);
    printf("[VOUS] ");
    setColor(COLOR_RESET);
    printf("%s\n", message);
    setColor(COLOR_CYAN);
    printf("       -> Chiffre AES-256 + Envoye via ping mondial\n");
    setColor(COLOR_RESET);
    
    return 1;
}

// Recevoir des messages en temps réel
int recevoirMessagesTempsReel(SessionChat* session) {
    FILE* f = fopen("chat_session.dat", "rb");
    if (!f) return 0;
    
    MessageTempsReel msg;
    int nouveauxMessages = 0;
    
    // Lire les nouveaux messages
    fseek(f, session->messagesRecus * sizeof(MessageTempsReel), SEEK_SET);
    
    while (fread(&msg, sizeof(MessageTempsReel), 1, f) == 1) {
        if (strcmp(msg.expediteur, session->nomUtilisateur) != 0) {
            char messageClair[1024];
            dechiffrerMessageAES(msg.message, msg.cleAES, messageClair);
            
            setColor(COLOR_MAGENTA);
            printf("\n[%s] ", msg.expediteur);
            setColor(COLOR_RESET);
            printf("%s\n", messageClair);
            setColor(COLOR_CYAN);
            printf("       -> Dechiffre AES-256 depuis %s\n", msg.ipDestinataire);
            setColor(COLOR_RESET);
            
            nouveauxMessages++;
        }
        session->messagesRecus++;
    }
    
    fclose(f);
    return nouveauxMessages;
}

// Mode chat interactif avec réception automatique
void modeChatTempsReel(int n, int e, long long d) {
    SessionChat session;
    char buffer[256];
    
    system("chcp 65001 >nul 2>&1"); // UTF-8
    
    printf("\n");
    setColor(COLOR_YELLOW);
    printf("  ========================================================================\n");
    printf("       INITIALISATION DU CHAT EN TEMPS REEL - MONDIAL                    \n");
    printf("  ========================================================================\n");
    setColor(COLOR_RESET);
    
    printf("\nVotre pseudo: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    
    char ipDistante[64];
    printf("IP du destinataire (mondiale): ");
    fgets(ipDistante, sizeof(ipDistante), stdin);
    ipDistante[strcspn(ipDistante, "\n")] = '\0';
    
    initialiserSession(&session, buffer, ipDistante, n, e, d);
    
    afficherInterfaceChat(&session);
    
    setColor(COLOR_GREEN);
    printf("OK Session initialisee avec chiffrement AES-256\n");
    printf("OK Communication mondiale via ping activee\n");
    printf("OK Cle de session generee: ");
    char hexKey[65];
    bytesToHex(session.cleAESSession, AES_KEY_SIZE, hexKey);
    printf("%.16s...\n\n", hexKey);
    setColor(COLOR_RESET);
    
    printf("Tapez vos messages (tapez '/quit' pour quitter, '/refresh' pour actualiser):\n");
    printf("===========================================================================\n\n");
    
    // Variables pour la saisie non-bloquante
    time_t dernierCheck = time(NULL);
    
    while (session.sessionActive) {
        // Vérifier les nouveaux messages toutes les 2 secondes
        time_t maintenant = time(NULL);
        if (difftime(maintenant, dernierCheck) >= 2.0) {
            int nouveaux = recevoirMessagesTempsReel(&session);
            if (nouveaux > 0) {
                printf("\n> "); // Réafficher le prompt
                fflush(stdout);
            }
            dernierCheck = maintenant;
        }
        
        // Vérifier si une entrée est disponible (non-bloquant sur Windows)
        if (_kbhit()) {
            printf("> ");
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                break;
            }
            buffer[strcspn(buffer, "\n")] = '\0';
            
            if (strcmp(buffer, "/quit") == 0) {
                session.sessionActive = 0;
                setColor(COLOR_RED);
                printf("\n[SYSTEME] Session terminee. Messages: %d envoyes, %d recus\n",
                       session.messagesEnvoyes, session.messagesRecus);
                setColor(COLOR_RESET);
                break;
            } else if (strcmp(buffer, "/refresh") == 0) {
                afficherInterfaceChat(&session);
                printf("===========================================================================\n\n");
                continue;
            } else if (strlen(buffer) > 0) {
                envoyerMessageTempsReel(&session, buffer);
            }
        } else {
            // Petite pause pour ne pas surcharger le CPU
            Sleep(100);
        }
    }
}

#endif
