/**
* @file sokoban.c
* @brief Programme qui fait tourner un jeu de sokoban
* @author Guillaume ANTOINES
* @version 1.0
* @date 09/11/2025
*
* Ce programme fait tourner un jeu de sokoban dont le but est de déplacer
* toutes les caisses sur des cibles pour gagner la partie.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Définition de la taille du tableau.
#define MAXLIG 12
typedef char t_plateau[MAXLIG][MAXLIG];

// Définition des caractères constantes.
const char CAISSE = '$';
const char MUR = '#';
const char JOUEUR = '@';
const char CIBLE = '.';
const char JOUEUR_CIBLE = '+';
const char CAISSE_CIBLE = '*';
const char CASE = ' ';

// Définition des touches de déplacement.
const char HAUT = 'z';
const char BAS = 's';
const char GAUCHE = 'q';
const char DROITE = 'd';
const char QUITTER = 'x';
const char RECOMMENCER = 'r';


// liste des procédures déclarés
void chargerPartie(t_plateau plateau, char fichier[]);
void enregistrerPartie(t_plateau plateau, char fichier[]);
void afficherEntete(int*nbDép, char fichier[]);
void afficherPlateau(t_plateau plateau);
void chercherJoueur(t_plateau plateau, int*posx, int*posy);
int kbhit();
void deplacer(t_plateau position, int*posx, int*posy, int*nbDép, char fichier[]);
void abandonnerPartie(t_plateau plateau, char fichier[]);
void recommencerPartie(t_plateau plateau, char fichier[], 
	int*nbDép, int*posx, int*posy);
bool gagner (t_plateau plateau);

int main(){
	int posx = 0; // position horizontale du joueur
	int posy = 0; // position verticale du joueur
	int nbDép = -1; // nombre de déplacements effectués
	char fichier[20]; // nom du fichier de sauvegarde
	t_plateau plateau; // déclaration du plateau de jeu


	// sélection du niveau
	printf("Quel niveau voulez vous charger ? (ex: niveau1.sok) : ");
	scanf("%s",fichier); 
	system("clear");

	chargerPartie(plateau, fichier);
	enregistrerPartie(plateau, fichier);
	afficherEntete(&nbDép, fichier);
	afficherPlateau(plateau);
	chercherJoueur(plateau, &posx, &posy);
	// tant qu'il y a des caisses à déplacer
	while (!gagner(plateau)) {
	deplacer(plateau, &posx, &posy, &nbDép, fichier);
	}
	// affichage des résultats
	printf("Vous avez gagné la partie avec %d déplacements !\n", nbDép);
	return EXIT_SUCCESS;
}

// charge une session a partir d'un fichier 
void chargerPartie(t_plateau plateau, char fichier[]) {
    FILE * f;
    char finDeLigne;
	int TAILLE = 12;

    f = fopen(fichier, "r");
    if (f==NULL) {
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne=0 ; ligne<TAILLE ; ligne++) {
            for (int colonne=0 ; colonne<TAILLE ; colonne++) {
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

// permet d'enregistrer une session dans un fichier
void enregistrerPartie(t_plateau plateau, char fichier[]) {
    FILE * f;
    char finDeLigne='\n';
	int TAILLE = 12;


    f = fopen(fichier, "w");
    for (int ligne=0 ; ligne<TAILLE ; ligne++) {
        for (int colonne=0 ; colonne<TAILLE ; colonne++) {
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

// affiche les commandes et le nombre de déplacements effectués
void afficherEntete(int*nbDép, char fichier[]) {
	system("clear");
	printf(" Nom de la partie de partie : %s\n\n", fichier);
	printf(" Haut : z\n Bas : s\n Gauche : q\n Droite : d\n");
	printf(" Pour abandonner la partie : x\n Pour continuer la partie : r\n\n");
	printf(" Nombre de déplacement : %d\n\n", *nbDép);
}

// affiche le plateau de jeu du fichier choisi
void afficherPlateau(t_plateau plateau) {
	char caractere;
	for (int lig=0; lig < MAXLIG; lig++) {
		for (int col=0; col < MAXLIG; col++) {
			caractere = plateau[lig][col];
			// pour afficher correctement le joueur et la caisse sur cible 
			if (caractere == JOUEUR_CIBLE) {
				printf("%c", JOUEUR);
			}
			else if (caractere == CAISSE_CIBLE) {
				printf("%c", CAISSE);
			}
			else{
				printf("%c", caractere);
			}
		}
		printf("\n");
	}
}

// pour chercher la position du joueur (caractère : @ ou +).
void chercherJoueur(t_plateau plateau, int*posx, int*posy) {
	for (int lig=0; lig < MAXLIG; lig++) {
		for (int col=0; col < MAXLIG; col++) {
			// si on trouve le joueur
			if ((plateau[lig][col] == JOUEUR) || (plateau[lig][col] == JOUEUR_CIBLE)){
				*posx = lig;
				*posy = col;
			}
		}
	}
}

int kbhit() {
	// la fonction retourne :
	// 1 si un caractere est present
	// 0 si pas de caractere présent
	int unCaractere=0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	// mettre le terminal en mode non bloquant
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
	ch = getchar();

	// restaurer le mode du terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
 
	if (ch != EOF) {
		ungetc(ch, stdin);
		unCaractere=1;
	} 
	return unCaractere;
}

// lorsque le joueur décide d'abandonner la partie
void abandonnerPartie(t_plateau plateau, char fichier[]) {
	char validation;
	printf("Souhaitez-vous sauvegarder votre progression ? \n y/n :");
	scanf("%c", &validation);
	if (validation == 'y'){
		printf("Nommez le fichier de sauvegarde : ");
		scanf("%s", fichier);
		enregistrerPartie(plateau, fichier);
		printf("Partie sauvegardée dans le fichier %s\n", fichier);
	}
	printf("Au revoir !\n");
	exit(0);
}

// lorsque le joueur décide de recommencer la partie
void recommencerPartie(t_plateau plateau, char fichier[], 
	int*nbDép, int*posx, int*posy) {

	char validation;
	printf("Recommencer la partie ? (y/n) ");
    		scanf(" %c", &validation);
		    if (validation == 'y') {
    		    chargerPartie(plateau, fichier);
    		    chercherJoueur(plateau, posx, posy);
				system("clear"); // Efface l'écran immédiatement.
				*nbDép = -2; // Réinitialise le nombre de déplacements
    		    afficherEntete(nbDép, fichier);
    		    afficherPlateau(plateau); // Affiche le plateau rechargé
				}
}

void deplacer(t_plateau plateau, int*posx, int*posy, int*nbDép, char fichier[]) {
	// depx et depy représentent la case de destination du joueur (déplacement)
	// depx et depy peuvent aussi représenter la case d'une caisse
	// casx et casy représentent la case de destination d'une caisse
	char touche;
	int depx, depy;
	int casx, casy;
	depx = *posx;
	depy = *posy;

	//définition de la valeur de touche
	touche = '\0';
	// lecture de la touche appuyée et déplacement du joueur
	if (kbhit()) {
		touche = getchar();
		switch(touche) {
			case HAUT:
				depx--;
				break;
			case BAS:
				depx++;
				break;
			case GAUCHE:
				depy--;
				break;
			case DROITE:
				depy++;
				break;
			case QUITTER:
				abandonnerPartie(plateau, fichier);
				break;
			case RECOMMENCER:
				recommencerPartie(plateau, fichier, nbDép, posx, posy);
				break;
			default:
				break;
			}
		// si la case de destination n'est pas un mur
		if (plateau[depx][depy] != MUR){

			if ((plateau[depx][depy] == CAISSE) || (plateau[depx][depy] == CAISSE_CIBLE)) {
				// calcul de la case de destination de la caisse
				casx = depx + (depx - *posx);
				casy = depy + (depy - *posy);
				// colision avec un mur ou une autre caisse
				if ((plateau[casx][casy] != MUR) && (plateau[casx][casy] != CAISSE)
					&& (plateau[casx][casy] != CAISSE_CIBLE)) {
					// si la caisse est déplacée depuis une cible
					if (plateau[depx][depy] == CAISSE_CIBLE) {
						plateau[depx][depy] = CIBLE;
						// si la caisse sur cible est déplacée sur une cible
						if (plateau[casx][casy] == CIBLE) {
							plateau[casx][casy] = CAISSE_CIBLE;
						}
						else {
						plateau[casx][casy] = CAISSE;
						}
					}
					// si la caisse est déplacée sur une cible
					else if (plateau[casx][casy] == CIBLE) {
						plateau[casx][casy] = CAISSE_CIBLE;
					}
					else {
						plateau[casx][casy] = CAISSE;
					}
					// si le joueur est déplacé depuis une cible
					if (plateau[*posx][*posy] == JOUEUR_CIBLE) {
						plateau[*posx][*posy] = CIBLE;
					}
					else {
						plateau[*posx][*posy] = CASE;
					}
					*posx = depx; // mise à jour de la position du joueur
					*posy = depy;
					// si le joueur est déplacé sur une cible
					if (plateau[depx][depy] == CIBLE) {
						plateau[*posx][*posy] = JOUEUR_CIBLE;
					}
					else {
						plateau[*posx][*posy] = JOUEUR;
					}
					(*nbDép)++;
				}
			}
			// Uniquement les déplacements du joueur
			else {
				if (plateau[*posx][*posy] == JOUEUR_CIBLE) {
					plateau[*posx][*posy] = CIBLE;
				}
				else {
					plateau[*posx][*posy] = CASE;
				}
				*posx = depx;
				*posy = depy;
				if (plateau[depx][depy] == CIBLE) {
					plateau[*posx][*posy] = JOUEUR_CIBLE;
				}
				else {
					plateau[*posx][*posy] = JOUEUR;
				}
				(*nbDép)++;
			}
		}
		afficherEntete(nbDép,fichier);
		afficherPlateau(plateau);
	}
}

// vérifie si il n'y a plus de caisses à déplacer sur les cibles
bool gagner (t_plateau plateau) {
	bool win = false;
	int nbCibles = 0;
	// compte le nombre de caisses restantes
	for (int lig=0; lig < MAXLIG; lig++) {
		for (int col=0; col < MAXLIG; col++) {
			if (plateau[lig][col] == CAISSE) {
				nbCibles++;
			}
		}
	}
	if (nbCibles == 0) {
		win = true; // toutes les caisses sont sur les cibles
	}
	return win;
}
