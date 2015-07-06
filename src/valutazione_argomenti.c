/** \file valutazione_argomenti.c
  \author Alessandro Pagiaro
Si dichiara che il contenuto di questo file e' in ogni sua parte opera
originale dell' autore.  */

#include "valutazione_argomenti.h"
#include <limits.h>

wat_proc_conf* valutazione_argomenti(int argc, char** argv, int nwork_def, int chr_def){
	char* avanzo;
	int option;
	wat_proc_conf* a;

	if(argc<2){
		errno=EINVAL;
		perror("Mancano argomenti");
		return NULL;
	}
	avanzo = NULL;

	a = (wat_proc_conf*) malloc(sizeof(wat_proc_conf));
	if(a == NULL){
		perror("Problemi di allocazione memoria");
		return NULL;
	}
	a->nwork = 0;
	a->chron = 0;
	a->dumpfile = NULL;
	a->file = NULL;

	while ( (option = getopt(argc, argv,"::n:v:f:")) != -1) {
		switch (option) {
			 case 'n' :{
					if(a->nwork){
					 	errno = E2BIG;
					 	perror("Opzione inserita due volte");
					 	if(a->dumpfile != NULL)
					 		free(a->dumpfile);
					 	free(a);
					 	return NULL;
					}
					else {
						(a)->nwork=strtol(optarg, &avanzo, 10);
						if (strcmp(avanzo, "")){
						 	errno = EINVAL;
						 	perror("Errore opz -n, param non era un numero");
						 	if(a->dumpfile != NULL)
						 		free(a->dumpfile);
						 	free(a);
						 	return NULL;
						}
						if(a->nwork <= 0){
							errno=EINVAL;
						 	perror("Parametro opzione -n non valido");
						 	if(a->dumpfile != NULL)
						 		free(a->dumpfile);
						 	free(a);
						 	return NULL;
						}
				 	}
				}
				 break;
			 case 'v' : {
				 if(a->chron){
					errno = E2BIG;
					perror("Opzione inserita due volte");
					if(a->dumpfile != NULL)
						free(a->dumpfile);
					free(a);
					return NULL;
				 }
				 else{
					a->chron = strtol(optarg, &avanzo, 10);
					if(a->chron > INT_MAX){
						errno=EINVAL;
						if(a->dumpfile != NULL)
							free(a->dumpfile);
						free(a);
						perror("Non ho tutto quello spazio per salvare tutti quei chronon per la stampa");
						return NULL;
					}
					if (strcmp(avanzo, "")){
					 	errno = EINVAL;
					 	perror("Errore opz -n, param non era un numero");
					 	if(a->dumpfile != NULL)
					 		free(a->dumpfile);
					 	free(a);
					 	return NULL;
					}
					if( a->chron <= 0 ){
						errno=EINVAL;
					 	perror("Parametro opzione -v non valido");
					 	free(a);
					 	return NULL;
					}
					}
			 	}
				break;
			 case 'f' : {
					a->dumpfile = (char*) malloc(sizeof(char)*strlen(optarg)+1);
					if(a->dumpfile == NULL){
						perror("Errore allocazione memoria");
						free(a);
						return NULL;
					}
					strcpy(a->dumpfile, optarg);
				 }
				 break;
			 default : {
				 errno=EINVAL;
				 perror("Opzione non riconosciuta"); 
				 free(a->dumpfile);
				 free(a);
				 return NULL;
				}
			 	break;
		}
	}

	if( (optind-argc) == -1) {
		a->file=(char*) malloc(sizeof(char)*strlen(argv[optind])+1);
		strcpy(a->file,argv[optind]);
	}
	else{
		errno=EINVAL;
		perror("Errore parametri file passati");
		if(a->dumpfile != NULL)
			free(a->dumpfile);
		free(a);
		return NULL;
	}

	return a;
}