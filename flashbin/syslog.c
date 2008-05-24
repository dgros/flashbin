#include <syslog.h>
#include "flashbin.h"

void sys_log(char *erreur)
{
	char	*log="Flashbin";
	
	openlog(log, LOG_CONS|LOG_PID, LOG_LOCAL7);
	syslog(LOG_DEBUG, erreur);
	closelog();
}

void gestionnaire(int numero)
{
	switch (numero){
		case SIGUSR1 : 
			printf("%d recu\n");
			break;
		case SIGUSR2 : 
			printf("%d recu\n");
			break;

	}
}
/* Correspondance de la numérotation : A faire
 *
 *
 *
 *
 *
 *
 */
