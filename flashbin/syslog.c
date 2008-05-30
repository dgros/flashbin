#include <syslog.h>
#include "flashbin.h"
pthread_mutex_t	mutex_log = PTHREAD_MUTEX_INITIALIZER;

extern char premier_dossier[100];
extern char deuxieme_dossier[100];
extern char troisieme_dossier[100];
extern char way_save[100];

void sys_log(char *erreur)
{
	char	*log="Flashbin";
	
	openlog(log, LOG_CONS|LOG_PID, LOG_LOCAL7);
	syslog(LOG_ERR, erreur);
	closelog();
}

void gestionnaire(int numero)
{
	FILE * fichier_log;
	int ret, taille;
	char buffer[BUFSIZ];
	log_fic *l=(log_fic *)malloc(sizeof(log_fic));

	switch (numero){
		case SIGHUP : 
			fichier_configuration();
			break;
		case SIGUSR1 : 
	strcpy(l->debut,"#flashbin.log\n#Synchronisation\nsynchronized =");
	strcpy(l->synch,"yes");
	strcpy(l->way,way_save);
	strcpy(l->path, "[paths]");	
	strcpy(l->bin,premier_dossier);
	strcpy(l->lib,deuxieme_dossier);
	strcpy(l->var,troisieme_dossier);
	l->b_m=0;
	l->l_m=0;
	l->v_m=0;
	strcpy(l->path_end, "[/paths]");

	sprintf(buffer, "%s %s\n%s \n%s\n%s = %i\n%s = %i\n%s = %i\n%s\n",l->debut, l->synch,l->way,l->path, l->bin, l->b_m, l->lib, l->l_m, l->var, l->v_m, l->path_end);	

	taille=strlen(buffer);
	
	pthread_mutex_lock (& mutex_log);

	fichier_log=fopen("flashbin.log", "w");
	ret=fwrite(buffer,1,taille ,fichier_log);
	fclose(fichier_log);
	
	pthread_mutex_unlock (& mutex_log);

	break;
		}
}
/* Correspondance de la numÃ©rotation : A faire
 *
 *
 *
 *
 *
 *
 */

