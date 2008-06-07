/* Deamon for flashbin project
 * Created 15, MAY 2008
 * Author : Gabriel SALLES-LOUSTAU & Damien GROS
 * To run this deamon as an program, execute with root rights
 * */
#include "flashbin.h"

pthread_mutex_t	mutex_fichier_log = PTHREAD_MUTEX_INITIALIZER;

char premier_dossier[100];
char deuxieme_dossier[100];
char troisieme_dossier[100];
char way_save[100];

int main(int argc, char *argv[])
{
	char fichier[BUFSIZ];
	FILE *fichier_log;
	int ret, taille;
	log_fic *l=(log_fic *)malloc(sizeof(log_fic));

	if( (strcmp(argv[1], "flash")) == 0 )
		strcpy(l->way,"way = flashtodisk");
	else
		strcpy(l->way,"way = disktoflash");

    strcpy(way_save,l->way);
	strcpy(l->debut,"#flashbin.log\n#Synchronisation\nsynchronized =");
	strcpy(l->synch,"yes");
	strcpy(l->path, "[paths]");
	// Create flashbin.log if it doesn't exist.
	strcpy(l->bin,"/usr/bin =");
	strcpy(l->lib,"/usr/lib =");
	strcpy(l->var,"/var/lib =");
	l->b_m=0;
	l->l_m=0;
	l->v_m=0;
	strcpy(l->path_end, "[/paths]");
	sprintf(fichier, "%s %s\n%s \n%s\n%s %i\n%s %i\n%s %i\n%s\n",l->debut, l->synch,l->way,l->path, l->bin, l->b_m, l->lib, l->l_m, l->var, l->v_m, l->path_end);
	
	taille=strlen(fichier);
	
	if((fichier_log=fopen("/var/log/flashbin.log", "r"))==NULL)
	{
		fichier_log=fopen("/var/log/flashbin.log", "w");
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
	int ret,i; 
	pthread_t pth;
	
	
	// Save the fileconf in a buffer 
	fichier_conf=fopen("/etc/flashbin.conf", "r");
	if(fichier_conf==NULL)
		sys_log("Impossible d'ouvrir le fichier de configuration");
	else 
	{
		ret=fread(buffer,1,sizeof(buffer) ,fichier_conf);
		fclose(fichier_conf);
		
		temp=strstr(buffer, "[partitions]");
		
		// Read the buffer to know the folder to audit
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
		i=0;
		strcpy(&dossier[0], temp);
		while(dossier[i] != ' ')
		{
			troisieme_dossier[i]=dossier[i];
			i++;
		}

	
		printf("%s\n%s\n%s\n", premier_dossier, deuxieme_dossier, troisieme_dossier);

		// Create a thread for each folders, and un specific thread for synchronization
		pthread_create(&pth, NULL, (void *) verification, premier_dossier);
		pthread_detach(pth);
		pthread_create(&pth, NULL, (void *) verification, deuxieme_dossier);
		pthread_detach(pth);
		pthread_create(&pth, NULL, (void *) verification, troisieme_dossier);
		pthread_detach(pth);
		pthread_create(&pth, NULL, (void *) synchronisation , NULL);
		pthread_detach(pth);
//		pthread_exit(NULL);
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
  sigaction(SIGUSR1, &sig, NULL); 
  
  // Check a first time in the structure stat to have a time reference
  stat(dossier, &statbuf_1);
  time_1=ctime(&statbuf_1.st_mtime);
  strcpy(&temporaire[0], time_1);

	while(1)
   		{
                // Call modification.sh to know if the sub-folder have been changed
		 sprintf(modification, "/usr/local/bin/modification.sh %s", dossier);
                 	if(system(modification)==1) sys_log("Impossible de lancer le script de modification");
                 stat(dossier, &statbuf_1);
    	         time_1=ctime(&statbuf_1.st_mtime);
   		 printf("%s %s",dossier, time_1);
	  		
			// If the folder has been changed
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
	
	// Stock the file log in a buffer
	strcpy(buffer_temp, "");
	strcpy(buffer, "");
	
	pthread_mutex_lock (& mutex_fichier_log);
	fichier_log=fopen("/var/log/flashbin.log", "r");
	
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
	    fichier_log=fopen("/var/log/flashbin.log", "w");
		ret=fwrite(buffer,1,taille ,fichier_log);
		fclose(fichier_log);
				pthread_mutex_unlock (& mutex_fichier_log);
		
		free(l);
		free(nombre);
	}
}	


void *synchronisation()
{
    int ret;
	FILE * fichier_syn;
    FILE * fichier_log;
    char buffer[BUFSIZ];

	while(1)
	{
		sleep(100);
        
            pthread_mutex_lock (& mutex_fichier_log);
        	fichier_log=fopen("/var/log/flashbin.log", "r");
	        ret=fread(buffer,1,sizeof(buffer) ,fichier_log);
        	fclose(fichier_log);
	        pthread_mutex_unlock (& mutex_fichier_log);
		
		        if(strstr(buffer, "no") != NULL)
                {
                         if( (fichier_syn=fopen("/var/run/synchronisation.run", "r")) == NULL)
		                 {	
	                        fichier_syn=fopen("/var/run/synchronisation.run", "w");
		                    fclose(fichier_syn);
	                    	system("/usr/local/bin/flashbin.sh synchronize");
		                 }
                }
	}
}
