#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

//pthread_mutex_t	mutex_fichier_log = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t	mutex_fct_log = PTHREAD_MUTEX_INITIALIZER;

void fichier_configuration();
void coupe_nom(char *dossier, char *buffer);
void *verification(char *dossier);
void ecrire_dans_log(char *dossier);
void sys_log(char *erreur);
void gestionnaire(int numero);

typedef struct {
	char debut[300];
	char synch[10];
	char way[100];
	char path[10];
	char bin[30];
	int  b_m;
	char lib[30];
	int  l_m;
	char var[30];
	int v_m;
	char path_end[10];
	
}log_fic;


