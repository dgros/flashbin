#include <syslog.h>
#include "flashbin.h"

void sys_log(char *erreur)
{
	char	*log="Flashbin";
	
	openlog(log, LOG_CONS|LOG_PID, LOG_LOCAL7);
	syslog(LOG_ERR, erreur);
	closelog();
}

void gestionnaire(int numero)
{
	switch (numero){
		case SIGHUP : 
			fichier_configuration();
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

