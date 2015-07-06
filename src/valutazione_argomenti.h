/** \file valutazione_argomenti.h
  \author Alessandro Pagiaro
Si dichiara che il contenuto di questo file e' in ogni sua parte opera
originale dell' autore.  */
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

/* La funzione valutazione_argomenti si occupa di prendere gli argomenti passati al main e controllare che siano correttamente formattati e semanticamente ragionevoli. 
 * Ritorna i valori in una struct {int nwork, int chron, char* dumpfile}
 */
typedef struct wat_proc_conf1 {
	int nwork;
	long int chron;
	char* dumpfile;
	char* file;
} wat_proc_conf;

wat_proc_conf* valutazione_argomenti(int argc, char** argv, int nwork_def, int chr_def);