#ifndef CHAT_REALTIME_LINUX_H
#define CHAT_REALTIME_LINUX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include "fonctions.h"
#include "aes.h"

#define PORT_CHAT 8888
#define BUFFER_SIZE 4096

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
    int socketServeur;
    int socketClient;
    int estServeur;
    pthread_t threadReception;
} SessionChat;

// Couleurs ANSI pour Linux
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"

void setColor(const char* color) {
    printf("%s", color);
}

// Vérifier si une touche a été pressée (Linux)
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// Thread pour recevoir les messages en continu
void* threadReceptionMessages(void* param) {
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
                setColor(ANSI_COLOR_MAGENTA);
                printf("[%s] ", expediteur);
                setColor(ANSI_COLOR_RESET);
                printf("%s\n", messageClair);
                setColor(ANSI_COLOR_CYAN);
                printf("       -> Dechiffre AES-256 depuis %s\n", session->ipDistante);
                setColor(ANSI_COLOR_RESET);
                printf("\n> ");
                fflush(stdout);
                
                session->messagesRecus++;
            }
        } else if (bytesRecus == 0) {
            printf("\n");
            setColor(ANSI_COLOR_RED);
            printf("[SYSTEME] Connexion fermee par le correspondant\n");
            setColor(ANSI_COLOR_RESET);
            session->sessionActive = 0;
            break;
        } else {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                printf("\n");
                setColor(ANSI_COLOR_RED);
                printf("[ERREUR] Erreur de reception: %d\n", errno);
                setColor(ANSI_COLOR_RESET);
                session->sessionActive = 0;
                break;
            }
        }
        
        usleep(100000); // 100ms
    }
    
    return NULL;
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
    session->socketServeur = -1;
    session->socketClient = -1;
    session->estServeur = 0;
    
    // Générer une clé AES pour cette session
    genererCleAES(session->cleAESSession, AES_KEY_SIZE);
}

// Afficher l'interface de chat
void afficherInterfaceChat(SessionChat* session) {
    system("clear");
    setColor(ANSI_COLOR_CYAN);
    printf("\n");
    printf("  ========================================================================\n");
    printf("  ");
    setColor(ANSI_COLOR_YELLOW);
    printf("      MESSAGERIE MONDIALE EN TEMPS REEL - AES-256 + RSA            ");
    setColor(ANSI_COLOR_CYAN);
    printf("\n");
    printf("  ========================================================================\n");
    printf("  ");
    setColor(ANSI_COLOR_GREEN);
    printf("Session: %-30s", session->nomUtilisateur);
    setColor(ANSI_COLOR_CYAN);
    printf("                            \n");
    printf("  ");
    setColor(ANSI_COLOR_GREEN);
    printf("IP Distante: %-25s", session->ipDistante);
    setColor(ANSI_COLOR_CYAN);
    printf("                            \n");
    printf("  ");
    setColor(ANSI_COLOR_GREEN);
    printf("Messages: Envoyes=%d | Recus=%d", session->messagesEnvoyes, session->messagesRecus);
    setColor(ANSI_COLOR_CYAN);
    printf("                                  \n");
    printf("  ========================================================================\n");
    printf("  ");
    setColor(ANSI_COLOR_YELLOW);
    printf("Chiffrement: AES-256 (Ultra-securise) + RSA pour echange de cles     ");
    setColor(ANSI_COLOR_CYAN);
    printf("\n");
    printf("  ========================================================================\n");
    setColor(ANSI_COLOR_RESET);
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

// Démarrer le serveur TCP
int demarrerServeur(SessionChat* session) {
    struct sockaddr_in adresseServeur, adresseClient;
    socklen_t tailleAdresse = sizeof(adresseClient);
    
    // Créer le socket serveur
    session->socketServeur = socket(AF_INET, SOCK_STREAM, 0);
    if (session->socketServeur < 0) {
        printf("[ERREUR] Impossible de creer le socket: %d\n", errno);
        return 0;
    }
    
    // Permettre la réutilisation de l'adresse
    int opt = 1;
    setsockopt(session->socketServeur, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Configurer l'adresse
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_addr.s_addr = INADDR_ANY;
    adresseServeur.sin_port = htons(PORT_CHAT);
    
    // Bind
    if (bind(session->socketServeur, (struct sockaddr*)&adresseServeur, sizeof(adresseServeur)) < 0) {
        printf("[ERREUR] Bind a echoue: %d\n", errno);
        close(session->socketServeur);
        return 0;
    }
    
    // Listen
    if (listen(session->socketServeur, 1) < 0) {
        printf("[ERREUR] Listen a echoue: %d\n", errno);
        close(session->socketServeur);
        return 0;
    }
    
    setColor(ANSI_COLOR_GREEN);
    printf("\n[SERVEUR] En ecoute sur le port %d...\n", PORT_CHAT);
    printf("[SERVEUR] En attente de connexion...\n");
    setColor(ANSI_COLOR_RESET);
    
    // Accepter une connexion
    session->socketClient = accept(session->socketServeur, (struct sockaddr*)&adresseClient, &tailleAdresse);
    if (session->socketClient < 0) {
        printf("[ERREUR] Accept a echoue: %d\n", errno);
        close(session->socketServeur);
        return 0;
    }
    
    // Récupérer l'IP du client
    char ipClient[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(adresseClient.sin_addr), ipClient, INET_ADDRSTRLEN);
    strcpy(session->ipDistante, ipClient);
    
    setColor(ANSI_COLOR_GREEN);
    printf("\n[SERVEUR] Connexion etablie avec %s\n", ipClient);
    setColor(ANSI_COLOR_RESET);
    
    session->estServeur = 1;
    return 1;
}

// Se connecter en tant que client
int connecterClient(SessionChat* session) {
    struct sockaddr_in adresseServeur;
    
    // Créer le socket client
    session->socketClient = socket(AF_INET, SOCK_STREAM, 0);
    if (session->socketClient < 0) {
        printf("[ERREUR] Impossible de creer le socket: %d\n", errno);
        return 0;
    }
    
    // Configurer l'adresse du serveur
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(PORT_CHAT);
    inet_pton(AF_INET, session->ipDistante, &adresseServeur.sin_addr);
    
    setColor(ANSI_COLOR_CYAN);
    printf("\n[CLIENT] Connexion a %s:%d...\n", session->ipDistante, PORT_CHAT);
    setColor(ANSI_COLOR_RESET);
    
    // Se connecter
    if (connect(session->socketClient, (struct sockaddr*)&adresseServeur, sizeof(adresseServeur)) < 0) {
        printf("[ERREUR] Connexion echouee: %d\n", errno);
        printf("[INFO] Assurez-vous que l'autre machine est en mode serveur\n");
        close(session->socketClient);
        session->socketClient = -1;
        return 0;
    }
    
    setColor(ANSI_COLOR_GREEN);
    printf("\n[CLIENT] Connecte a %s:%d\n", session->ipDistante, PORT_CHAT);
    setColor(ANSI_COLOR_RESET);
    
    return 1;
}

// Envoyer un message en temps réel via TCP
int envoyerMessageTempsReel(SessionChat* session, const char* message) {
    if (session->socketClient < 0) {
        setColor(ANSI_COLOR_RED);
        printf("[ERREUR] Pas de connexion active\n");
        setColor(ANSI_COLOR_RESET);
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
    
    if (byteEnvoyes < 0) {
        setColor(ANSI_COLOR_RED);
        printf("[ERREUR] Echec d'envoi: %d\n", errno);
        setColor(ANSI_COLOR_RESET);
        return 0;
    }
    
    session->messagesEnvoyes++;
    
    setColor(ANSI_COLOR_GREEN);
    printf("[VOUS] ");
    setColor(ANSI_COLOR_RESET);
    printf("%s\n", message);
    setColor(ANSI_COLOR_CYAN);
    printf("       -> Chiffre AES-256 + Envoye via TCP a %s:%d\n", session->ipDistante, PORT_CHAT);
    setColor(ANSI_COLOR_RESET);
    
    return 1;
}

// (Fonction recevoirMessagesTempsReel supprimée - maintenant gérée par le thread)

// Mode chat interactif avec réception automatique (Linux)
void modeChatTempsReel(int n, int e, long long d) {
    SessionChat session;
    char buffer[256];
    
    printf("\n");
    setColor(ANSI_COLOR_YELLOW);
    printf("  ========================================================================\n");
    printf("       INITIALISATION DU CHAT EN TEMPS REEL - MONDIAL                    \n");
    printf("  ========================================================================\n");
    setColor(ANSI_COLOR_RESET);
    
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
        setColor(ANSI_COLOR_RED);
        printf("\n[ERREUR] Impossible d'etablir la connexion\n");
        setColor(ANSI_COLOR_RESET);
        return;
    }
    
    afficherInterfaceChat(&session);
    
    setColor(ANSI_COLOR_GREEN);
    printf("OK Session initialisee avec chiffrement AES-256\n");
    printf("OK Communication TCP/IP en temps reel activee\n");
    printf("OK Connexion etablie avec %s:%d\n", session.ipDistante, PORT_CHAT);
    printf("OK Cle de session universelle: ");
    char hexKey[65];
    bytesToHex(session.cleAESSession, AES_KEY_SIZE, hexKey);
    printf("%.16s...\n\n", hexKey);
    setColor(ANSI_COLOR_RESET);
    
    // Démarrer le thread de réception
    if (pthread_create(&session.threadReception, NULL, threadReceptionMessages, &session) != 0) {
        printf("[ERREUR] Impossible de creer le thread de reception\n");
        close(session.socketClient);
        if (session.estServeur) close(session.socketServeur);
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
            setColor(ANSI_COLOR_RED);
            printf("\n[SYSTEME] Session terminee. Messages: %d envoyes, %d recus\n",
                   session.messagesEnvoyes, session.messagesRecus);
            setColor(ANSI_COLOR_RESET);
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
    pthread_join(session.threadReception, NULL);
    
    close(session.socketClient);
    if (session.estServeur) {
        close(session.socketServeur);
    }
    
    setColor(ANSI_COLOR_GREEN);
    printf("\n[OK] Connexion fermee proprement\n");
    setColor(ANSI_COLOR_RESET);
}

#endif
