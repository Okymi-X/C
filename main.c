#include "fonctions.h"
#include "messageries.h"
#include "chat_realtime.h"

void afficherMenu() {
    printf("\n========================================\n");
    printf("    MESSAGERIE MONDIALE SECURISEE      \n");
    printf("========================================\n");
    printf("1. Afficher mes cles RSA actuelles\n");
    printf("2. Publier un message (MONDIAL)\n");
    printf("3. Lire les messages publies\n");
    printf("4. Envoyer a une IP specifique\n");
    printf("5. Chat en temps reel (AES-256)\n");
    printf("6. Gestion des cles et messages\n");
    printf("7. Quitter\n");
    printf("========================================\n");
    printf("Votre choix: ");
}


void testRSA(int n, int e, long long d) {
    printf("\n========================================\n");
    printf("     ALGORITHME DE CHIFFREMENT RSA     \n");
    printf("========================================\n\n");
    
    // Affichage des clés
    printf("GENERATION DES CLES:\n");
    printf("-------------------\n");
    printf("  n (module)       : %d\n", n);
    printf("  e (exposant pub) : %d\n", e);
    printf("  d (exposant priv): %lld\n", d);
    printf("  Cle Publique     : (n=%d, e=%d)\n", n, e);
    printf("  Cle Privee       : (n=%d, d=%lld)\n\n", n, d);
    
    // Message original
    char message[100];
    printf("Veuillez entrer un message: ");
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0';
    
    printf("\nMESSAGE ORIGINAL:\n");
    printf("----------------\n");
    printf("  Texte: %s\n", message);
    printf("  Codes ASCII/UTF-8: ");
    for (int i = 0; i < strlen(message); i++){
        printf("%d ", (unsigned char)message[i]);
    }
    printf("\n\n");
    
    // Chiffrement
    long long messageChiffre[100];
    int longueur;
    chiffrer(message, e, n, messageChiffre, &longueur);
    
    printf("MESSAGE CHIFFRE:\n");
    printf("---------------\n");
    printf("  Nombres: ");
    for (int i = 0; i < longueur; i++){
        printf("%lld ", messageChiffre[i]);
    }
    printf("\n\n");
    
    // Déchiffrement
    char messageDechiffre[100];
    dechiffrer(messageChiffre, d, n, messageDechiffre, longueur);
    
    printf("MESSAGE DECHIFFRE:\n");
    printf("-----------------\n");
    printf("  Texte: %s\n", messageDechiffre);
    printf("  Codes ASCII/UTF-8: ");
    for (int i = 0; i < longueur; i++){
        printf("%d ", (unsigned char)messageDechiffre[i]);
    }
    printf("\n\n");
    
    printf("========================================\n");
    printf("  Verification: %s\n", strcmp(message, messageDechiffre) == 0 ? "REUSSI [OK]" : "ECHEC [X]");
    printf("========================================\n");
}


int main(){
    setlocale(LC_ALL, "");
    system("chcp 65001 >nul 2>&1"); // Activer UTF-8
    srand(time(NULL));
    
    // Variables pour les clés RSA
    int n, phi, e;
    long long d;
    
    // Essayer de charger les clés existantes
    if (chargerClesRSA(&n, &e, &d, &phi)) {
        printf("\n========================================\n");
        printf("   CLES RSA CHARGEES DEPUIS LE FICHIER  \n");
        printf("========================================\n");
        printf("  n = %d\n", n);
        printf("  e = %d\n", e);
        printf("  d = %lld\n", d);
        printf("  [OK] Vous pouvez lire vos anciens messages!\n");
        printf("========================================\n");
    } else {
        // Générer de nouvelles clés si aucune n'existe
        printf("\n========================================\n");
        printf("   GENERATION DE NOUVELLES CLES RSA     \n");
        printf("========================================\n");
        genererParametres(&n, &phi);
        e = genererExposantPublic(phi);
        d = calculerExposantPrive(e, phi);
        
        // Sauvegarder les nouvelles clés
        if (sauvegarderClesRSA(n, e, d, phi)) {
            printf("  [OK] Cles RSA generees et sauvegardees!\n");
            printf("  n = %d\n", n);
            printf("  e = %d\n", e);
            printf("  d = %lld\n", d);
        } else {
            printf("  [AVERTISSEMENT] Impossible de sauvegarder les cles\n");
        }
        printf("========================================\n");
    }
    
    // Configuration de la messagerie
    ConfigurationMessagerie config;
    
    int choix;
    char buffer[256];
    
    printf("\n");
    printf("   ========================================\n");
    printf("   |  MESSAGERIE SECURISEE RSA - MONDIALE |\n");
    printf("   ========================================\n");
    printf("   |  Mode: Communication Internet        |\n");
    printf("   |  Portee: Monde entier (IP publique)  |\n");
    printf("   |  Protection: Mot de passe + RSA      |\n");
    printf("   |  Auto-destruction: 3 tentatives      |\n");
    printf("   ========================================\n");
    
    while (1) {
        afficherMenu();
        scanf("%d", &choix);
        getchar(); // Consommer le '\n'
        
        switch (choix) {
            case 1:
                printf("\n========================================\n");
                printf("      VOS CLES RSA ACTUELLES           \n");
                printf("========================================\n\n");
                printf("CLE PUBLIQUE (partagee sur le reseau):\n");
                printf("  n = %d\n", n);
                printf("  e = %d\n\n", e);
                printf("CLE PRIVEE (gardee secrete):\n");
                printf("  n = %d\n", n);
                printf("  d = %lld\n\n", d);
                printf("IMPORTANT:\n");
                printf("  - Tous sur le reseau doivent avoir le meme 'n'\n");
                printf("  - Le mot de passe ajoute une protection supplementaire\n");
                printf("  - 3 tentatives incorrectes = message detruit!\n");
                printf("========================================\n");
                break;
                
            case 2: {
                printf("\n========================================\n");
                printf("   PUBLICATION DE MESSAGE MONDIAL      \n");
                printf("========================================\n\n");
                
                printf("Votre nom/pseudo: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                char nomExpediteur[256];
                strcpy(nomExpediteur, buffer);
                
                printf("Message a publier (monde entier): ");
                char message[256];
                fgets(message, sizeof(message), stdin);
                message[strcspn(message, "\n")] = '\0';
                
                printf("Mot de passe (requis pour lire): ");
                char motDePasse[256];
                fgets(motDePasse, sizeof(motDePasse), stdin);
                motDePasse[strcspn(motDePasse, "\n")] = '\0';
                
                if (strlen(motDePasse) < 4) {
                    printf("\n[ERREUR] Le mot de passe doit contenir au moins 4 caracteres!\n");
                    break;
                }
                
                printf("\n[INFO] Mode de diffusion mondiale active\n");
                printf("[INFO] Le message sera accessible depuis n'importe ou dans le monde\n\n");
                
                // Utiliser une adresse publique symbolique (en réalité sauvegardé localement/serveur)
                initialiserMessagerie(&config, n, e, d, "0.0.0.0");
                diffuserMessageSecurise(&config, message, motDePasse, nomExpediteur);
                nettoyerMessagerie();
                break;
            }
            
            case 3:
                lireMessageDiffuse(d, n);
                break;
                
            case 4: {
                printf("\n========================================\n");
                printf("   ENVOI DIRECT VERS UNE IP PUBLIQUE   \n");
                printf("========================================\n\n");
                
                printf("Adresse IP du destinataire (publique ou privee): ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                char ipDestinataire[256];
                strcpy(ipDestinataire, buffer);
                
                printf("Message a envoyer: ");
                char message[256];
                fgets(message, sizeof(message), stdin);
                message[strcspn(message, "\n")] = '\0';
                
                printf("Duree de vie (TTL en minutes, 0 pour illimite): ");
                int ttlMinutes;
                scanf("%d", &ttlMinutes);
                getchar();
                
                int ttlSecondes = ttlMinutes > 0 ? ttlMinutes * 60 : 31536000; // 1 an si illimité
                
                printf("\n[INFO] Envoi direct vers %s\n", ipDestinataire);
                printf("[INFO] Le message sera transmis via Internet\n\n");
                
                initialiserMessagerie(&config, n, e, d, ipDestinataire);
                envoyerMessageChiffreAvecTTL(&config, message, ttlSecondes);
                nettoyerMessagerie();
                break;
            }
                
            case 5: {
                printf("\n");
                system("chcp 65001 >nul 2>&1");
                setColor(COLOR_CYAN);
                printf("  ========================================================================\n");
                printf("        MODE CHAT EN TEMPS REEL - COMMUNICATION MONDIALE               \n");
                printf("  ========================================================================\n");
                printf("  ");
                setColor(COLOR_GREEN);
                printf("Chiffrement: AES-256 (Ultra-securise)");
                setColor(COLOR_CYAN);
                printf("                              \n");
                printf("  ");
                setColor(COLOR_GREEN);
                printf("Echange de cles: RSA");
                setColor(COLOR_CYAN);
                printf("                                                \n");
                printf("  ");
                setColor(COLOR_GREEN);
                printf("Communication: Ping mondial en temps reel");
                setColor(COLOR_CYAN);
                printf("                           \n");
                printf("  ");
                setColor(COLOR_GREEN);
                printf("Portee: Monde entier via Internet");
                setColor(COLOR_CYAN);
                printf("                                   \n");
                printf("  ========================================================================\n");
                setColor(COLOR_RESET);
                
                modeChatTempsReel(n, e, d);
                break;
            }
                
            case 6: {
                printf("\n========================================\n");
                printf("    GESTION DES CLES ET MESSAGES        \n");
                printf("========================================\n");
                printf("1. Regenerer de nouvelles cles RSA\n");
                printf("2. Supprimer tous les messages\n");
                printf("3. Afficher les informations systeme\n");
                printf("4. Retour au menu principal\n");
                printf("========================================\n");
                printf("Votre choix: ");
                
                int sousChoix;
                scanf("%d", &sousChoix);
                getchar();
                
                switch(sousChoix) {
                    case 1:
                        printf("\n[ATTENTION] Regenerer les cles supprimera l'acces aux anciens messages!\n");
                        printf("Etes-vous sur ? (o/n): ");
                        char confirmation;
                        scanf("%c", &confirmation);
                        getchar();
                        
                        if (confirmation == 'o' || confirmation == 'O') {
                            genererParametres(&n, &phi);
                            e = genererExposantPublic(phi);
                            d = calculerExposantPrive(e, phi);
                            
                            if (sauvegarderClesRSA(n, e, d, phi)) {
                                printf("\n[OK] Nouvelles cles generees et sauvegardees!\n");
                                printf("  n = %d\n", n);
                                printf("  e = %d\n", e);
                                printf("  d = %lld\n", d);
                            } else {
                                printf("\n[ERREUR] Impossible de sauvegarder les cles\n");
                            }
                        } else {
                            printf("\n[ANNULE] Conservation des cles actuelles\n");
                        }
                        break;
                        
                    case 2:
                        printf("\n[ATTENTION] Supprimer tous les messages est irreversible!\n");
                        printf("Etes-vous sur ? (o/n): ");
                        scanf("%c", &confirmation);
                        getchar();
                        
                        if (confirmation == 'o' || confirmation == 'O') {
                            supprimerTousLesMessages();
                        } else {
                            printf("\n[ANNULE] Messages conserves\n");
                        }
                        break;
                        
                    case 3:
                        printf("\n========================================\n");
                        printf("   INFORMATIONS SYSTEME                \n");
                        printf("========================================\n");
                        printf("Fichier de cles: cles_rsa.dat\n");
                        printf("Fichier de messages: messages_diffuses.dat\n");
                        printf("Cles actuelles:\n");
                        printf("  n = %d\n", n);
                        printf("  e = %d\n", e);
                        printf("  d = %lld\n", d);
                        printf("  phi = %d\n", phi);
                        
                        // Compter les messages
                        MessageDiffuse messages[100];
                        int nbMessages = chargerMessagesDiffuses(messages, 100);
                        printf("\nNombre de messages stockes: %d\n", nbMessages);
                        printf("========================================\n");
                        break;
                        
                    case 4:
                        printf("\n[OK] Retour au menu principal\n");
                        break;
                        
                    default:
                        printf("\n[ERREUR] Choix invalide!\n");
                }
                break;
            }
                
            case 7:
                printf("\n========================================\n");
                printf("         Fermeture securisee            \n");
                printf("========================================\n");
                printf("Au revoir!\n\n");
                nettoyerMessagerie();
                return 0;
                
            default:
                printf("\n[ERREUR] Choix invalide!\n");
        }
    }

    return 0;
}
