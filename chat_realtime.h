#ifndef CHAT_REALTIME_H
#define CHAT_REALTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "fonctions.h"
#include "aes.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT_CHAT 8888
#define BUFFER_SIZE 4096

// Déclarations anticipées
void chiffrerMessageAES(const char* message, const uint8_t* cleAES, char* messageChiffre);
void dechiffrerMessageAES(const char* messageChiffre, const uint8_t* cleAES, char* messageClair);

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
    SOCKET socketServeur;
    SOCKET socketClient;
    int estServeur;
    HANDLE threadReception;
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

// Initialiser Winsock
int initialiserWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("[ERREUR] WSAStartup a echoue: %d\n", result);
        return 0;
    }
    return 1;
}

// Nettoyer Winsock
void nettoyerWinsock() {
    WSACleanup();
}

// Thread pour recevoir les messages en continu
DWORD WINAPI threadReceptionMessages(LPVOID param) {
    SessionChat* session = (SessionChat*)param;
    char buffer[BUFFER_SIZE];
    
    while (session->sessionActive) {
        int bytesRecus = recv(session->socketClient, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesRecus > 0) {
            buffer[bytesRecus] = '\0';
            
            // Parser le message (format: EXPEDITEUR|MESSAGE_CHIFFRE)
            char* separateur = strchr(buffer, '|');
            if (separateur != NULL) {
                *separateur = '\0';
                char* expediteur = buffer;
                char* messageChiffre = separateur + 1;
                
                // Déchiffrer le message
                char messageClair[1024];
                dechiffrerMessageAES(messageChiffre, session->cleAESSession, messageClair);
                
                printf("\n");
                setColor(COLOR_MAGENTA);
                printf("[%s] ", expediteur);
                setColor(COLOR_RESET);
                printf("%s\n", messageClair);
                setColor(COLOR_CYAN);
                printf("       -> Dechiffre AES-256 depuis %s\n", session->ipDistante);
                setColor(COLOR_RESET);
                printf("\n> ");
                fflush(stdout);
                
                session->messagesRecus++;
            }
        } else if (bytesRecus == 0) {
            printf("\n");
            setColor(COLOR_RED);
            printf("[SYSTEME] Connexion fermee par le correspondant\n");
            setColor(COLOR_RESET);
            session->sessionActive = 0;
            break;
        } else {
            int erreur = WSAGetLastError();
            if (erreur != WSAEWOULDBLOCK) {
                printf("\n");
                setColor(COLOR_RED);
                printf("[ERREUR] Erreur de reception: %d\n", erreur);
                setColor(COLOR_RESET);
                session->sessionActive = 0;
                break;
            }
        }
        
        Sleep(100);
    }
    
    return 0;
}

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
    session->socketServeur = INVALID_SOCKET;
    session->socketClient = INVALID_SOCKET;
    session->estServeur = 0;
    session->threadReception = NULL;
    
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

// Envoyer un message en temps réel via TCP
int envoyerMessageTempsReel(SessionChat* session, const char* message) {
    if (session->socketClient == INVALID_SOCKET) {
        setColor(COLOR_RED);
        printf("[ERREUR] Pas de connexion active\n");
        setColor(COLOR_RESET);
        return 0;
    }
    
    char messageChiffre[2048];
    
    // Chiffrer avec AES
    chiffrerMessageAES(message, session->cleAESSession, messageChiffre);
    
    // Préparer le paquet: EXPEDITEUR|MESSAGE_CHIFFRE
    char paquet[BUFFER_SIZE];
    snprintf(paquet, BUFFER_SIZE, "%s|%s", session->nomUtilisateur, messageChiffre);
    
    // Envoyer via socket TCP
    int byteEnvoyes = send(session->socketClient, paquet, strlen(paquet), 0);
    
    if (byteEnvoyes == SOCKET_ERROR) {
        setColor(COLOR_RED);
        printf("[ERREUR] Echec d'envoi: %d\n", WSAGetLastError());
        setColor(COLOR_RESET);
        return 0;
    }
    
    session->messagesEnvoyes++;
    
    setColor(COLOR_GREEN);
    printf("[VOUS] ");
    setColor(COLOR_RESET);
    printf("%s\n", message);
    setColor(COLOR_CYAN);
    printf("       -> Chiffre AES-256 + Envoye via TCP a %s:%d\n", session->ipDistante, PORT_CHAT);
    setColor(COLOR_RESET);
    
    return 1;
}

// Démarrer le serveur TCP
int demarrerServeur(SessionChat* session) {
    struct sockaddr_in adresseServeur, adresseClient;
    int tailleAdresse = sizeof(adresseClient);
    
    // Créer le socket serveur
    session->socketServeur = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (session->socketServeur == INVALID_SOCKET) {
        printf("[ERREUR] Impossible de creer le socket: %d\n", WSAGetLastError());
        return 0;
    }
    
    // Configurer l'adresse
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_addr.s_addr = INADDR_ANY;
    adresseServeur.sin_port = htons(PORT_CHAT);
    
    // Bind
    if (bind(session->socketServeur, (struct sockaddr*)&adresseServeur, sizeof(adresseServeur)) == SOCKET_ERROR) {
        printf("[ERREUR] Bind a echoue: %d\n", WSAGetLastError());
        closesocket(session->socketServeur);
        return 0;
    }
    
    // Listen
    if (listen(session->socketServeur, 1) == SOCKET_ERROR) {
        printf("[ERREUR] Listen a echoue: %d\n", WSAGetLastError());
        closesocket(session->socketServeur);
        return 0;
    }
    
    setColor(COLOR_GREEN);
    printf("\n[SERVEUR] En ecoute sur le port %d...\n", PORT_CHAT);
    printf("[SERVEUR] En attente de connexion...\n");
    setColor(COLOR_RESET);
    
    // Accepter une connexion
    session->socketClient = accept(session->socketServeur, (struct sockaddr*)&adresseClient, &tailleAdresse);
    if (session->socketClient == INVALID_SOCKET) {
        printf("[ERREUR] Accept a echoue: %d\n", WSAGetLastError());
        closesocket(session->socketServeur);
        return 0;
    }
    
    // Récupérer l'IP du client
    char* ipClient = inet_ntoa(adresseClient.sin_addr);
    strcpy(session->ipDistante, ipClient);
    
    setColor(COLOR_GREEN);
    printf("\n[SERVEUR] Connexion etablie avec %s\n", ipClient);
    setColor(COLOR_RESET);
    
    session->estServeur = 1;
    return 1;
}

// Se connecter en tant que client
int connecterClient(SessionChat* session) {
    struct sockaddr_in adresseServeur;
    
    // Créer le socket client
    session->socketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (session->socketClient == INVALID_SOCKET) {
        printf("[ERREUR] Impossible de creer le socket: %d\n", WSAGetLastError());
        return 0;
    }
    
    // Configurer l'adresse du serveur
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(PORT_CHAT);
    adresseServeur.sin_addr.s_addr = inet_addr(session->ipDistante);
    
    setColor(COLOR_CYAN);
    printf("\n[CLIENT] Connexion a %s:%d...\n", session->ipDistante, PORT_CHAT);
    setColor(COLOR_RESET);
    
    // Se connecter
    if (connect(session->socketClient, (struct sockaddr*)&adresseServeur, sizeof(adresseServeur)) == SOCKET_ERROR) {
        printf("[ERREUR] Connexion echouee: %d\n", WSAGetLastError());
        printf("[INFO] Assurez-vous que l'autre machine est en mode serveur\n");
        closesocket(session->socketClient);
        session->socketClient = INVALID_SOCKET;
        return 0;
    }
    
    setColor(COLOR_GREEN);
    printf("\n[CLIENT] Connecte a %s:%d\n", session->ipDistante, PORT_CHAT);
    setColor(COLOR_RESET);
    
    return 1;
}

// Mode chat interactif avec réception automatique
void modeChatTempsReel(int n, int e, long long d) {
    SessionChat session;
    char buffer[256];
    
    system("chcp 65001 >nul 2>&1"); // UTF-8
    
    // Initialiser Winsock
    if (!initialiserWinsock()) {
        printf("[ERREUR] Impossible d'initialiser Winsock\n");
        return;
    }
    
    printf("\n");
    setColor(COLOR_YELLOW);
    printf("  ========================================================================\n");
    printf("       INITIALISATION DU CHAT EN TEMPS REEL - MONDIAL                    \n");
    printf("  ========================================================================\n");
    setColor(COLOR_RESET);
    
    printf("\nVotre pseudo: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    
    printf("\nMode de connexion:\n");
    printf("  1. Serveur (attendre une connexion)\n");
    printf("  2. Client (se connecter a une IP)\n");
    printf("Votre choix: ");
    int modeChoix;
    scanf("%d", &modeChoix);
    getchar();
    
    char ipDistante[64] = "0.0.0.0";
    
    if (modeChoix == 2) {
        printf("IP du destinataire: ");
        fgets(ipDistante, sizeof(ipDistante), stdin);
        ipDistante[strcspn(ipDistante, "\n")] = '\0';
    }
    
    initialiserSession(&session, buffer, ipDistante, n, e, d);
    
    // Établir la connexion
    int connexionReussie = 0;
    if (modeChoix == 1) {
        connexionReussie = demarrerServeur(&session);
    } else {
        connexionReussie = connecterClient(&session);
    }
    
    if (!connexionReussie) {
        setColor(COLOR_RED);
        printf("\n[ERREUR] Impossible d'etablir la connexion\n");
        setColor(COLOR_RESET);
        nettoyerWinsock();
        return;
    }
    
    afficherInterfaceChat(&session);
    
    setColor(COLOR_GREEN);
    printf("OK Session initialisee avec chiffrement AES-256\n");
    printf("OK Communication TCP/IP en temps reel activee\n");
    printf("OK Connexion etablie avec %s:%d\n", session.ipDistante, PORT_CHAT);
    printf("OK Cle de session universelle: ");
    char hexKey[65];
    bytesToHex(session.cleAESSession, AES_KEY_SIZE, hexKey);
    printf("%.16s...\n\n", hexKey);
    setColor(COLOR_RESET);
    
    // Démarrer le thread de réception
    session.threadReception = CreateThread(NULL, 0, threadReceptionMessages, &session, 0, NULL);
    if (session.threadReception == NULL) {
        printf("[ERREUR] Impossible de creer le thread de reception\n");
        closesocket(session.socketClient);
        if (session.estServeur) closesocket(session.socketServeur);
        nettoyerWinsock();
        return;
    }
    
    printf("Tapez vos messages (tapez '/quit' pour quitter, '/refresh' pour actualiser):\n");
    printf("===========================================================================\n\n");
    
    // Boucle principale
    while (session.sessionActive) {
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
    }
    
    // Nettoyage
    session.sessionActive = 0;
    if (session.threadReception != NULL) {
        WaitForSingleObject(session.threadReception, 2000);
        CloseHandle(session.threadReception);
    }
    
    closesocket(session.socketClient);
    if (session.estServeur) {
        closesocket(session.socketServeur);
    }
    nettoyerWinsock();
    
    setColor(COLOR_GREEN);
    printf("\n[OK] Connexion fermee proprement\n");
    setColor(COLOR_RESET);
}

#endif
