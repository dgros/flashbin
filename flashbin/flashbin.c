// gcc -Wall -o flashbin flashbin.c -lpthread -D_REENTRANT
/* Créé le 15 mai 2008
 * TODO : 
 * -> Regarder les dossiers à surveiller : Ok
 * -> Savoir si les dossiers sont modifiés : Ok
 * -> Recopie des dossiers dans le fichier de log : Ok
 * -> Savoir la "route" : flashtodisk or disktoflash : A faire
 * -> Mettre en place le gestionnaire de signal : En cours
 * -> Log des erreurs avec sys_log : En cours
 * -> Les descripteurs : A faire
 * -> Autre : subsequement, A faire
 * */
/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

pthread_mutex_t	mutex_fichier_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	mutex_fct_log = PTHREAD_MUTEX_INITIALIZER;

void fichier_configuration();
void coupe_nom(char *dossier, char *buffer);
void *verification(char *dossier);
void ecrire_dans_log(char *dossier);

typedef struct {
	char debut[300];
	char synch[10];
	char way[100];
	char bin[30];
	int  b_m;
	char lib[30];
	int  l_m;
	char var[30];
	int v_m;
	
}log_fic;*/
#include "flashbin.h"

pthread_mutex_t	mutex_fichier_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	mutex_fct_log = PTHREAD_MUTEX_INITIALIZER;



int main(int argc, char *argv[])
{
	char fichier[BUFSIZ];
	FILE *fichier_log;
	int ret, taille;
	log_fic *l=(log_fic *)malloc(sizeof(log_fic));
	if(argv[1]== "flash")
	strcpy(l->way,"way = flashtodisk");
	else
	strcpy(l->way,"way = disktoflash");

	strcpy(l->debut,"#flashbin.log\n#Synchronisation\nsynchronisation =");
	strcpy(l->synch,"yes");
	//strcpy(l->way,"way = flashtodisk");
	strcpy(l->bin,"/usr/bin =");
	strcpy(l->lib,"/usr/lib =");
	strcpy(l->var,"/var =");
	l->b_m=0;
	l->l_m=0;
	l->v_m=0;
	sprintf(fichier, "%s %s\n%s \n%s %i\n%s %i\n%s %i\n",l->debut, l->synch,l->way, l->bin, l->b_m, l->lib, l->l_m, l->var, l->v_m);
	
	taille=strlen(fichier);
	fichier_log=fopen("flashbin.log", "w");
	
	if(fichier_log==NULL)
		sys_log("Impossible de créer un fichier dans /var/log/");
	else
	{
		ret=fwrite(fichier,1,taille ,fichier_log);
		fclose(fichier_log);
		fichier_configuration(argv[1]);
	}
	return 0;
}



void fichier_configuration()
{	
	FILE *fichier_conf;
	char buffer[BUFSIZ];
	char *temp;
	char premier_dossier[100];
	char deuxieme_dossier[100];
	char troisieme_dossier[100];
	int ret; 
	pthread_t pth;
	
	
	// On récupère le contenu du fichier dans un buffer
	fichier_conf=fopen("flashbin.conf", "r");
	if(fichier_conf==NULL)
		sys_log("Impossible d'ouvrir le fichier de configuration");
	else 
	{
		ret=fread(buffer,1,sizeof(buffer) ,fichier_conf);
		fclose(fichier_conf);

		// On recupère le nom de chaque dossier à "monitorer"
		temp=strstr(buffer,"/usr/bin");
		coupe_nom(premier_dossier,temp);
		
		temp=strstr(buffer,"/usr/lib");
		coupe_nom(deuxieme_dossier,temp);
	
		temp=strstr(buffer,"/var");
		coupe_nom(troisieme_dossier,temp);
	
		printf("%s\n%s\n%s\n", premier_dossier, deuxieme_dossier, troisieme_dossier);

		// On met un thread pour chaque dossier
		pthread_create(&pth, NULL, (void *) verification, premier_dossier);
		pthread_create(&pth, NULL, (void *) verification, deuxieme_dossier);
		pthread_create(&pth, NULL, (void *) verification, troisieme_dossier);
		pthread_exit(NULL);
	}
}

void coupe_nom(char *dossier, char *temp)
{
	char temp1[100];
	int i;
		strcpy(&temp1[0], temp);
		i=0;
		// On ne recopie que jusqu'au premier espace pour avoir le nom complet
		while(temp1[i] != ' ')
		{
	      dossier[i]=temp1[i];
		  i++;
		}
	dossier[i]='\0';

}

void * verification(char *dossier)
{
  struct stat statbuf_1;
  char *time_1;
  char temporaire[100];
  
  struct sigaction sig;

  sig.sa_flags = 0;
  sig.sa_handler = gestionnaire;

  sigaction(SIGUSR1	, &sig, NULL); 
  sigaction(SIGUSR2	, &sig, NULL); 
  
  // On lit dans la structure stat la date de modification des dossiers
  stat(dossier, &statbuf_1);
  time_1=ctime(&statbuf_1.st_mtime);
  strcpy(&temporaire[0], time_1);

	while(1)
   		{
		 stat(dossier, &statbuf_1);
    	 time_1=ctime(&statbuf_1.st_mtime);
   		 printf("%s", time_1);
	  		
			//Si le dossier est modifié
			if(strcmp(time_1, temporaire) != 0)
			{
			ecrire_dans_log(dossier);
			}
   
  		 sleep(5);
   		}

  return 0;
}


void ecrire_dans_log(char *dossier)
{
	FILE * fichier_log;
	int ret, taille;
	char buffer[BUFSIZ];
	char *nombre=(char *)malloc(sizeof(char));
	char buffer_temp[BUFSIZ];
	char *dos_bin;
	char *way_t=(char *)malloc(sizeof(char));
	log_fic *l=(log_fic *)malloc(sizeof(log_fic));
	
	//On récupére le fichier dans un buffer pour traitement
	strcpy(buffer_temp, "");
	strcpy(buffer, "");
	
	pthread_mutex_lock (& mutex_fichier_log);
	fichier_log=fopen("flashbin.log", "r");
	
	if(fichier_log==NULL)
	{
		sys_log("Impossible de créer un fichier dans /var/log/");
		pthread_mutex_lock (& mutex_fichier_log);
	}
	
	else 
	{
		ret=fread(buffer,1,sizeof(buffer) ,fichier_log);
		fclose(fichier_log);
		pthread_mutex_unlock (& mutex_fichier_log);
		
		way_t=strstr(buffer,"way");
		strncpy(l->way,way_t,17);

		dos_bin=NULL;
		taille=strlen(buffer); // On stocke la première taille du buffer
		dos_bin=strstr(buffer, "/usr/bin")+11;
		strncpy(nombre, dos_bin,1);


		if(strcmp(dossier, "/usr/bin")==0 || atoi(nombre)==1 )
		{
		l->b_m=1;
		}

		dos_bin=NULL;
		dos_bin=strstr(buffer, "/usr/lib")+11;
		strncpy(nombre, dos_bin,1);


		if(strcmp(dossier, "/usr/lib")==0 || atoi(nombre)==1 )
		{
		l->l_m=1;
		}
	
		dos_bin=NULL;
		dos_bin=strstr(buffer, "/var")+7;
		strncpy(nombre, dos_bin,1);

		if(strcmp(dossier, "/var")==0 || atoi(nombre)==1 )
		{
		l->v_m=1;
		}
	
		strcpy(l->debut,"#flashbin.log\n#Synchronisation\nsynchronisation =");
		strcpy(l->synch,"no");
		strcpy(l->bin,"/usr/bin =");
		strcpy(l->lib,"/usr/lib =");
		strcpy(l->var,"/var =");
		sprintf(buffer, "%s %s\n%s \n%s %i\n%s %i\n%s %i\n",l->debut, l->synch,l->way, l->bin, l->b_m, l->lib, l->l_m, l->var, l->v_m);
	
		taille=strlen(buffer);
		//		pthread_mutex_lock (& mutex_fichier_log);
	    fichier_log=fopen("flashbin.log", "w");
		ret=fwrite(buffer,1,taille ,fichier_log);
		fclose(fichier_log);
		//		pthread_mutex_unlock (& mutex_fichier_log);
		
		//free(way_t);
		//free(dos_bin);
		free(l);
		free(nombre);
	}
}	

