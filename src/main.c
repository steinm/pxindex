#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <getopt.h>
#include <libintl.h>
#include <sys/types.h>
#include <regex.h>
#include <libgen.h>
#include "config.h"
#ifdef HAVE_GSF
#include <paradox-gsf.h>
#else
#include <paradox.h>
#endif

#ifdef MEMORY_DEBUGGING
#include <paradox-mp.h>
#endif

#ifdef ENABLE_NLS
#define _(String) gettext(String)
#else
#define _(String) String
#endif

void errorhandler(pxdoc_t *p, int error, const char *str, void *data) {
	  fprintf(stderr, "PXLib: %s\n", str);
}

/* usage() {{{
 * Output usage information
 */
void usage(char *progname) {
	int recode;

	printf(_("Version: %s %s http://sourceforge.net/projects/pxlib"), progname, VERSION);
	printf("\n");
	printf(_("Copyright: Copyright (C) 2003 Uwe Steinmann <uwe@steinmann.cx>"));
	printf("\n\n");
	printf(_("Usage: %s [OPTIONS] FILE"), progname);
	printf("\n\n");
	printf(_("Options:"));
	printf("\n\n");
	printf(_("  -h, --help          this usage information."));
	printf("\n");
	printf(_("  --version           show version information."));
	printf("\n");
	printf(_("  -v, --verbose       be more verbose."));
	printf("\n");
	printf(_("  -d, --database-file=FILE read database from this file."));
	printf("\n");
#ifdef HAVE_GSF
	if(PX_has_gsf_support()) {
		printf(_("  --use-gsf           use gsf library to read input file."));
		printf("\n");
	}
#endif
	printf("\n");
	recode = PX_has_recode_support();
	switch(recode) {
		case 1:
			printf(_("libpx uses librecode for recoding."));
			break;
		case 2:
			printf(_("libpx uses iconv for recoding."));
			break;
		case 0:
			printf(_("libpx has no support for recoding."));
			break;
	}
	printf("\n");
	if(PX_is_bigendian())
		printf(_("libpx has been compiled for big endian architecture."));
	else
		printf(_("libpx has been compiled for little endian architecture."));
	printf("\n");
	printf(_("libpx has gsf support: %s"), PX_has_gsf_support() == 1 ? _("Yes") : _("No"));
	printf("\n");
	printf(_("libpx has version: %d.%d.%d"), PX_get_majorversion(), PX_get_minorversion(), PX_get_subminorversion());
	printf("\n\n");
}
/* }}} */

/* main() {{{
 */
int main(int argc, char *argv[]) {
	pxhead_t *pxh;
	pxfield_t *pxf;
	pxdoc_t *pxdoc = NULL;
	pxdoc_t *pxindexdoc = NULL;
	char *progname = NULL;
	char *data, *buffer = NULL;
	int i, j, c; // general counters
	int usegsf = 0;
	int verbose = 0;
	int numprimkeys;
	char *inputfile = NULL;
	char *outputfile = NULL;
	FILE *infp = NULL;
	FILE *outfp = NULL;

#ifdef MEMORY_DEBUGGING
	PX_mp_init();
#endif

#ifdef ENABLE_NLS
	setlocale (LC_ALL, "");
	setlocale (LC_NUMERIC, "C");
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain (GETTEXT_PACKAGE);
#endif

	/* Handle program options {{{
	 */
	progname = basename(strdup(argv[0]));
	while(1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
			{"verbose", 0, 0, 'v'},
			{"output-file", 1, 0, 'o'},
			{"help", 0, 0, 'h'},
			{"use-gsf", 0, 0, 8},
			{"database-file", 1, 0, 'd'},
			{"version", 0, 0, 11},
			{0, 0, 0, 0}
		};
		c = getopt_long (argc, argv, "vo:d:h",
				long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
			case 8:
				usegsf = 1;
				break;
			case 'h':
				usage(progname);
				exit(0);
				break;
			case 11:
				fprintf(stdout, "%s\n", VERSION);
				exit(0);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'o':
				outputfile = strdup(optarg);
				break;
			case 'd':
				inputfile = strdup(optarg);
				break;
		}
	}

	if (optind < argc) {
		outputfile = strdup(argv[optind]);
	}

	if(!outputfile) {
		fprintf(stderr, _("You must at least specify an output file."));
		fprintf(stderr, "\n");
		fprintf(stderr, "\n");
		usage(progname);
		exit(1);
	}
	/* }}} */

	/* Open database file {{{
	 */
#ifdef MEMORY_DEBUGGING
	if(NULL == (pxdoc = PX_new2(errorhandler, PX_mp_malloc, PX_mp_realloc, PX_mp_free))) {
#else
	if(NULL == (pxdoc = PX_new2(errorhandler, NULL, NULL, NULL))) {
#endif
		fprintf(stderr, _("Could not create new paradox instance."));
		fprintf(stderr, "\n");
		exit(1);
	}

#ifdef HAVE_GSF
	if(PX_has_gsf_support() && usegsf) {
		GsfInput *input = NULL;
		GsfInputStdio  *in_stdio;
		GsfInputMemory *in_mem;
		GError *gerr = NULL;
		fprintf(stderr, "Inputfile:  %s\n", inputfile);
		gsf_init ();
		in_mem = gsf_input_mmap_new (inputfile, NULL);
		if (in_mem == NULL) {
			in_stdio = gsf_input_stdio_new(inputfile, &gerr);
			if(in_stdio != NULL)
				input = GSF_INPUT (in_stdio);
			else {
				fprintf(stderr, _("Could not open gsf input file."));
				fprintf(stderr, "\n");
				g_object_unref (G_OBJECT (input));
				exit(1);
			}
		} else {
			input = GSF_INPUT (in_mem);
		}
		if(0 > PX_open_gsf(pxdoc, input)) {
			fprintf(stderr, _("Could not open input file."));
			fprintf(stderr, "\n");
			exit(1);
		}
	} else {
#endif
		if(0 > PX_open_file(pxdoc, inputfile)) {
			fprintf(stderr, _("Could not open input file."));
			fprintf(stderr, "\n");
			exit(1);
		}
#ifdef HAVE_GSF
	}
#endif
	/* }}} */

	numprimkeys = pxdoc->px_head->px_primarykeyfields;
	if(numprimkeys <= 0) {
		fprintf(stderr, _("The database file has no primary key fields."));
		fprintf(stderr, "\n");
		exit(1);
	}

	/* Create index file {{{
	 */
#ifdef MEMORY_DEBUGGING
	if(NULL == (pxindexdoc = PX_new2(errorhandler, PX_mp_malloc, PX_mp_realloc, PX_mp_free)))
#else
	if(NULL == (pxindexdoc = PX_new2(errorhandler, NULL, NULL, NULL))) {
#endif
		fprintf(stderr, _("Could not create new paradox instance."));
		fprintf(stderr, "\n");
		exit(1);
	}

	/* Create the schema for the primary index file. */
	if((pxf = (pxfield_t *) pxindexdoc->malloc(pxindexdoc, numprimkeys*sizeof(pxfield_t), _("Could not get memory for field definitions."))) == NULL){
		fprintf(stderr, "\n");
		exit(1);
	}
	for(i=0; i<numprimkeys; i++) {
		pxfield_t *pfield;
		pfield = PX_get_field(pxdoc, i);
		if(!pfield) {
			fprintf(stderr, _("Could not get field definition of %i. primary key field."));
			fprintf(stderr, "\n");
			exit(1);
		}
		pxf[i].px_fname = px_strdup(pxindexdoc, pfield->px_fname);
		pxf[i].px_ftype = pfield->px_ftype;
		pxf[i].px_flen = pfield->px_flen;
		pxf[i].px_fdc = pfield->px_fdc;
	}

	if(0 > PX_create_file(pxindexdoc, pxf, numprimkeys, outputfile, pxfFileTypPrimIndex)) {
		fprintf(stderr, _("Could not create primary index file."));
		fprintf(stderr, "\n");
		exit(1);
	}
	PX_write_primary_index(pxdoc, pxindexdoc);
	/* }}} */

	/* Free resources and close files {{{
	 */
	PX_close(pxindexdoc);
	PX_delete(pxindexdoc);

	PX_close(pxdoc);
	PX_delete(pxdoc);

#ifdef HAVE_GSF
	if(PX_has_gsf_support() && usegsf) {
		gsf_shutdown();
	}
#endif
	/* }}} */

#ifdef MEMORY_DEBUGGING
	PX_mp_list_unfreed();
#endif

	exit(0);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
