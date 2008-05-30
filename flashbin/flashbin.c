// gcc -Wall -o flashbin flashbin.c -lpthread -D_REENTRANT
/* CrÃƒÂ©ÃƒÂ© le 15 mai 2008
 * TODO : 
 * -> Regarder les dossiers Ãƒ  surveiller : Ok
 * -> Savoir si les dossiers sont modifiÃƒÂ©s : Ok
 * -> Recopie des dossiers dans le fichier de log : Ok
 * -> Savoir la "route" : flashtodisk or disktoflash : Ok
 * -> Mettre en place le gestionnaire de signal : Ok
 * -> Log des erreurs avec sys_log : Ok
 * -> Les descripteurs : A faire
 * -> Autre : subsequement, A faire
 * */
#include "flashbin.h"

pthread_mutex_t	mutex_fichier_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	mutex_fct_log = PTHREAD_MUTEX_INITIALIZER;

char premier_dossier[100];
char deuxieme_dossier[100];
char troisieme_dossier[100];


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

	strcpy(l->debut,"#flashbin.log\n#Synchronisation\nsynchronized =");
	strcpy(l->synch,"yes");
	strcpy(l->path, "[paths]");
	// On crée par défaut le fichier de log avec les dossiers standart
	strcpy(l->bin,"/usr/bin =");
	strcpy(l->lib,"/usr/lib =");
	strcpy(l->var,"/var =");
	l->b_m=0;
	l->l_m=0;
	l->v_m=0;
	strcpy(l->path_end, "[/paths]");
	sprintf(fichier, "%s %s\n%s \n%s\n%s %i\n%s %i\n%s %i\n%s\n",l->debut, l->synch,l->way,l->path, l->bin, l->b_m, l->lib, l->l_m, l->var, l->v_m, l->path_end);
	
	taille=strlen(fichier);
	
	if((fichier_log=fopen("flashbin.log", "r"))==NULL)
	{
		fichier_log=fopen("flashbin.log", "w");
		if(fichier_log==NULL)
		{
			sys_log("Impossible de créer un fichier dans /var/log/");
		}
		else
		{
			ret=fwrite(fichier,1,taille ,fichier_log);
			fclose(fichier_log);
			fichier_configuration();
		}
	}
	else
	{
		fichier_configuration();
	}
	return 0;
}



void fichier_configuration()
{	
	FILE *fichier_conf;
	char buffer[BUFSIZ];
	char *temp=(char*)malloc(sizeof(char));
	char dossier[100];
//	char deuxieme_dossier[100];
//	char troisieme_dossier[100];
	int ret,i; 
	pthread_t pth;
	
	
	// On récupèrere le contenu du fichier dans un buffer
	fichier_conf=fopen("flashbin.conf", "r");
	if(fichier_conf==NULL)
		sys_log("Impossible d'ouvrir le fichier de configuration");
	else 
	{
		ret=fread(buffer,1,sizeof(buffer) ,fichier_conf);
		fclose(fichier_conf);
		
		temp=strstr(buffer, "[partitions]");
		
		// On recupére le nom de chaque dossier à "monitorer"
		temp=strstr(temp,"/");
		i=0;
		strcpy(&dossier[0], temp);
		while(dossier[i] != ' ')
		{
			premier_dossier[i]=dossier[i];
			i++;
		}
		
		temp=strstr(temp+strlen(premier_dossier),"/");
		i=0;
		strcpy(&dossier[0], temp);
		while(dossier[i] != ' ')
		{
			deuxieme_dossier[i]=dossier[i];
			i++;
		}

		temp=strstr(temp+strlen(deuxieme_dossier),"/");
		//coupe_nom(troisieme_dossier,temp);
		i=0;
		strcpy(&dossier[0], temp);
		while(dossier[i] != ' ')
		{
			troisieme_dossier[i]=dossier[i];
			i++;
		}

	
		printf("%s\n%s\n%s\n", premier_dossier, deuxieme_dossier, troisieme_dossier);

		// On met un thread pour chaque dossier
		pthread_create(&pth, NULL, (void *) verification, premier_dossier);
		pthread_create(&pth, NULL, (void *) verification, deuxieme_dossier);
		pthread_create(&pth, NULL, (void *) verification, troisieme_dossier);
		pthread_create(&pth, NULL, (void *) synchronisation , NULL);
		pthread_exit(NULL);
	}
}


void * verification(char *dossier)
{
  struct stat statbuf_1;
  char *time_1;
  char modification[100];
  char temporaire[100];
  
struct sigaction sig;

  sig.sa_flags = 0;
  sig.sa_handler = gestionnaire;

  sigaction(SIGHUP, &sig, NULL); 
  
  // On lit dans la structure stat la date de modification des dossiers
  stat(dossier, &statbuf_1);
  time_1=ctime(&statbuf_1.st_mtime);
  strcpy(&temporaire[0], time_1);

	while(1)
   		{
		 sprintf(modification, "./modification.sh %s", dossier);
                 	if(system(modification)==1) sys_log("Impossible de lancer le script de modification");
                 stat(dossier, &statbuf_1);
    	         time_1=ctime(&statbuf_1.st_mtime);
   		 printf("%s %s",dossier, time_1);
	  		
			//Si le dossier est modifié
			if(strcmp(time_1, temporaire) != 0)
			{
			ecrire_dans_log(dossier);
			}
   
  		 sleep(100);
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
	
	//On récupèree le fichier dans un buffer pour traitement
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
		taille=strlen(buffer); // On stocke la premiÃƒÂ¨re taille du buffer
		dos_bin=strstr(buffer, premier_dossier)+strlen(premier_dossier);
		strncpy(nombre, dos_bin,1);


		if(strcmp(dossier, premier_dossier)==0 || atoi(nombre)==1 )
		{
		l->b_m=1;
		}

		dos_bin=NULL;
		dos_bin=strstr(buffer, deuxieme_dossier)+strlen(deuxieme_dossier);
		strncpy(nombre, dos_bin,1);


		if(strcmp(dossier, deuxieme_dossier)==0 || atoi(nombre)==1 )
		{
		l->l_m=1;
		}
	
		dos_bin=NULL;
		dos_bin=strstr(buffer, troisieme_dossier)+strlen(troisieme_dossier);
		strncpy(nombre, dos_bin,1);

		if(strcmp(dossier, troisieme_dossier)==0 || atoi(nombre)==1 )
		{
		l->v_m=1;
		}
	
		strcpy(l->debut,"#flashbin.log\n#Synchronisation\nsynchronized =");
		strcpy(l->synch,"no");
		strcpy(l->path, "[paths]");
		strcpy(l->path_end, "[/path]");
		strcpy(l->bin,premier_dossier);
		strcpy(l->lib,deuxieme_dossier);
		strcpy(l->var,troisieme_dossier);
		sprintf(buffer, "%s %s\n%s \n%s\n%s = %i\n%s = %i\n%s = %i\n%s\n",l->debut, l->synch,l->way,l->path, l->bin, l->b_m, l->lib, l->l_m, l->var, l->v_m, l->path_end);
	
		taille=strlen(buffer);
				pthread_mutex_lock (& mutex_fichier_log);
	    fichier_log=fopen("flashbin.log", "w");
		ret=fwrite(buffer,1,taille ,fichier_log);
		fclose(fichier_log);
				pthread_mutex_unlock (& mutex_fichier_log);
		
		//free(way_t);
		//free(dos_bin);
		free(l);
		free(nombre);
	}
}	


void *synchronisation()
{
	FILE * fichier_syn;

	while(1)
	{
		sleep(30);
		if( (fichier_syn=fopen("synchronisation.run", "r")) == NULL)
		{	
		fichier_syn=fopen("synchronisation.run", "w");
		fclose(fichier_syn);
		system("./flashbin.sh synchronize && rm synchronisation.run");
		}
	}
}
