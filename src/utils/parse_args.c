// parse_args.c
// Parse command line arguments for batch automation of sts.c

/*
 * This code has been heavily modified by the following people:
 *
 *      Landon Curt Noll
 *      Tom Gilgan
 *      Riccardo Paccagnella
 *
 * See the README.md and the initial comment in sts.c for more information.
 *
 * WE (THOSE LISTED ABOVE WHO HEAVILY MODIFIED THIS CODE) DISCLAIM ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL WE (THOSE LISTED ABOVE
 * WHO HEAVILY MODIFIED THIS CODE) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo (Landon Curt Noll, http://www.isthe.com/chongo/index.html) /\oo/\
 *
 * Share and enjoy! :-)
 */


// Exit codes: 0 thru 4
// NOTE: 5-9 is used in sts.c

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include "../utils/externs.h"
#include "utilities.h"
#include "debug.h"

/*
 * Forward static function declarations
 */
static void change_params(struct state *state, long int parameter, long int value, double d_value);

/*
 * Default run state
 *
 * Default state allows sts to run without any human intervention
 */
static struct state const defaultstate = {
/* *INDENT-OFF* */

	// batchmode
	true,				// batch non-interactive mode

	// testVectorFlag & testVector
	false,				// No -t test1[,test2].. was given
	{false, false, false, false,
	 false, false, false, false,
	 false, false, false, false,
	 false, false, false, false,
	},				// Do not invoke any test

	// iterationFlag
	false,				// No -i iterations was given

	// reportCycleFlag & reportCycle
	false,				// No -I reportCycle was given
	0,				// Do not report on iteration progress

	// runModeFlag & runMode
	false,				// No -m mode was given
	MODE_ITERATE_AND_ASSESS,	// Iterate and assess only

	// workDirFlag & workDir
	false,				// No -w workDir was given
	".",				// Write results under experiments

	// subDirsFlag & subDirs
	false,				// No -c was given
	true,				// Create directories

	// resultstxtFlag
	false,				// No -s, don't create results.txt, data*.txt and stats.txt files

	// randomDataArg & randomDataPath
	false,				// no randdata arg was given
	"/dev/null",			// default input file is /dev/null
	false,				// not reading randdata from stdin by default

	// dataFormatFlag & dataFormat
	false,				// -F format was not given
	FORMAT_RAW_BINARY,		// Read data as raw binary

	// numberOfThreads
	false,
	0,
	0,

	// jobnumFlag, jobnum & base_seek
	false,				// No -j jobnum was given
	0,				// Begin at start of randdata (-j 0)
	0,				// Default seek to 0

	// pvalues_dir & filenames
	NULL,				// Directory where to look for the .pvalues binary files
	NULL,				// Names of the .pvalues files

	// tp, promptFlag, uniformityBinsFlag
	{DEFAULT_BLOCK_FREQUENCY,	// -P 1=M, Block Frequency Test - block length
	 DEFAULT_NON_OVERLAPPING,	// -P 2=m, NonOverlapping Template Test - block length
	 DEFAULT_OVERLAPPING,		// -P 3=m, Overlapping Template Test - block length
	 DEFAULT_APEN,			// -P 4=m, Approximate Entropy Test - block length
	 DEFAULT_SERIAL,		// -P 5=m, Serial Test - block length
	 DEFAULT_LINEARCOMPLEXITY,	// -P 6=M, Linear Complexity Test - block length
	 DEFAULT_ITERATIONS,		// -P 7=iterations (-i iterations)
	 DEFAULT_UNIFORMITY_BINS,	// -P 8=bins, uniformity test is thru this many bins
	 DEFAULT_BITCOUNT,		// -P 9=bitcount, Length of a single bit stream
	 DEFAULT_UNIFORMITY_LEVEL,	// -P 10=uni_level, uniformity errors have values below this
	 DEFAULT_ALPHA,			// -P 11=alpha, p_value significance level
	},
	false,				// Do not prompt for change of parameters
	false,				// No -P 8 was given with custom uniformity bins

	// c, cSetup
	{UNSET_DOUBLE,			// Square root of 2
	 UNSET_DOUBLE,			// log(2)
	 UNSET_DOUBLE,			// Square root of n
	 UNSET_DOUBLE,			// log(n)
	 0,				// Number of crossings required to complete the test
	},
	false,				// init() has not yet initialized c

	// streamFile, finalReptPath, finalRept, freqFilePath, finalRept
	NULL,				// Initially the randomDataPath is not open
	NULL,				// Path of the final results file
	NULL,				// Initially the final results file is not open
	NULL,				// Path of freq.txt
	NULL,				// Initially freq.txt is not open

	// testNames, subDir, driver_state
	{"((all_tests))",		// TEST_ALL = 0, convention for indicating run all tests
	 "Frequency",			// TEST_FREQUENCY = 1, Frequency test (frequency.c)
	 "BlockFrequency",		// TEST_BLOCK_FREQUENCY = 2, Block Frequency test (blockFrequency.c)
	 "CumulativeSums",		// TEST_CUSUM = 3, Cumulative Sums test (cusum.c)
	 "Runs",			// TEST_RUNS = 4, Runs test (runs.c)
	 "LongestRun",			// TEST_LONGEST_RUN = 5, Longest Runs test (longestRunOfOnes.c)
	 "Rank",			// TEST_RANK = 6, Rank test (rank.c)
	 "DFT",				// TEST_DFT = 7, Discrete Fourier Transform test (discreteFourierTransform.c)
	 "NonOverlappingTemplate",	// TEST_NON_OVERLAPPING = 8, Non-overlapping Template Matchings test
					// (nonOverlappingTemplateMatchings.c)
	 "OverlappingTemplate",		// TEST_OVERLAPPING = 9, Overlapping Template test (overlappingTemplateMatchings.c)
	 "Universal",			// TEST_UNIVERSAL = 10, Universal test (universal.c)
	 "ApproximateEntropy",		// TEST_APEN = 11, Approximate Entropy test (approximateEntropy.c)
	 "RandomExcursions",		// TEST_RND_EXCURSION = 12, Random Excursions test (randomExcursions.c)
	 "RandomExcursionsVariant",	// TEST_RND_EXCURSION_VAR = 13, Random Excursions Variant test (randomExcursionsVariant.c)
	 "Serial",			// TEST_SERIAL = 14, Serial test (serial.c)
	 "LinearComplexity",		// TEST_LINEARCOMPLEXITY = 15, Linear Complexity test (linearComplexity.c)
	},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	},

	// partitionCount, datatxt_fmt
	{0,					// TEST_ALL = 0 - not a real test
	 1,					// TEST_FREQUENCY = 1
	 1,					// TEST_BLOCK_FREQUENCY = 2
	 2,					// TEST_CUSUM = 3
	 1,					// TEST_RUNS = 4
	 1,					// TEST_LONGEST_RUN = 5
	 1,					// TEST_RANK = 6
	 1,					// TEST_DFT = 7
	 MAX_NUMOFTEMPLATES,			// TEST_NON_OVERLAPPING = 8
						// NOTE: Value may be changed by OverlappingTemplateMatchings_init()
	 1,					// TEST_OVERLAPPING = 9
	 1,					// TEST_UNIVERSAL = 10
	 1,					// TEST_APEN = 11
	 NUMBER_OF_STATES_RND_EXCURSION,	// TEST_RND_EXCURSION = 12
	 NUMBER_OF_STATES_RND_EXCURSION_VAR,	// TEST_RND_EXCURSION_VAR = 13
	 2,					// TEST_SERIAL = 14
	 1,					// TEST_LINEARCOMPLEXITY = 15
	},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	},

	// stats, p_val - per test dynamic arrays
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	},

	// is_excursion
	{false, false, false, false, false, false, false, false,
	 false, false, false, false, true, true, false, false,
	},

	// epsilon, tmpepsilon
	NULL,
	NULL,

	// count, valid, success, failure, valid_p_val
	{0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	},
	{0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	},
	{0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	},
	{0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	},
	{0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	},

	// metric_results & successful_tests
	{FAILED_BOTH, FAILED_BOTH, FAILED_BOTH, FAILED_BOTH,
	 FAILED_BOTH, FAILED_BOTH, FAILED_BOTH, FAILED_BOTH,
	 FAILED_BOTH, FAILED_BOTH, {FAILED_BOTH, FAILED_BOTH},
	 {FAILED_BOTH, FAILED_BOTH}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
	},
	0,

	// maxGeneralSampleSize, maxRandomExcursionSampleSize
	0,
	0,

	// nonovTemplates
	NULL,

	// fft_m, fft_X
	NULL,
	NULL,

# if defined(LEGACY_FFT)
	// fft_wsave
	NULL,
#else /* LEGACY_FFT */
	// fftw_p and fftw_out
	NULL,
	NULL,
#endif /* LEGACY_FFT */

	// rank_matrix
	NULL,

	// rnd_excursion_var_stateX, ex_var_partial_sums
	NULL,
	NULL,

	// linear_b, linear_c, linear_t
	NULL,
	NULL,
	NULL,

	// apen_C, apen_C_len
	NULL,
	0,

	// serial_v, serial_v_len
	NULL,
	0,

	// nonper_seq
	NULL,

	// universal_L, universal_T
	0,
	0,

	// rnd_excursion_S, rnd_excursion_cycle, rnd_excursion_stateX, rnd_excursion_pi_terms
	NULL,
	NULL,
	NULL,
	NULL,

	// legacy_output
	false,
/* *INDENT-ON* */
};


/*
 * Command line usage information
 */
/* *INDENT-OFF* */
static const char * const usage =
"[-v level] [-A] [-t test1[,test2]..]\n"
"             [-P num=value[,num=value]..] [-i iterations] [-I reportCycle] [-O]\n"
"             [-w workDir] [-c] [-s] [-F format] [-j jobnum] [-S bitcount]\n"
"             [-m mode] [-T numOfThreads] [-d pvaluesdir] [-h] [randdata]\n"
"\n"
"    -v  debuglevel     debug level (def: 0 -> no debug messages)\n"
"    -A                 ask a human what to do, use obsolete interactive mode (def: batch mode)\n"
"                       As batch mode is now default, the old -b flag was removed.\n"
"    -t test1[,test2].. tests to invoke, 0-15 (def: 0 -> run all tests)\n"
"\n"
"        0: Run all tests (1-15)\n"
"        1: Frequency                        2: Block Frequency\n"
"        3: Cumulative Sums                  4: Runs\n"
"        5: Longest Run of Ones              6: Rank\n"
"        7: Discrete Fourier Transform       8: Nonperiodic Template Matchings\n"
"        9: Overlapping Template Matchings  10: Universal Statistical\n"
"       11: Approximate Entropy             12: Random Excursions\n"
"       13: Random Excursions Variant       14: Serial\n"
"       15: Linear Complexity\n"
"\n"
"    -P num=value[,num=value]..     change parameter num to value (def: keep defaults)\n"
"\n"
"       1: Block Frequency Test - block length(M):		16384\n"
"       2: NonOverlapping Template Test - block length(m):	9\n"
"       3: Overlapping Template Test - block length(m):		9\n"
"       4: Approximate Entropy Test - block length(m):		10\n"
"       5: Serial Test - block length(m):			16\n"
"       6: Linear Complexity Test - block length(M):		500\n"
"       7: Number of bitcount runs (same as -i iterations):	1\n"
"       8: Uniformity bins:                         		sqrt(iterations) or 10 (if -O)\n"
"       9: Bits to process per iteration (same as -S bitcount):	1048576 (== 1024*1024)\n"
"      10: Uniformity Cutoff Level:				0.0001\n"
"      11: Alpha Confidence Level:				0.01\n"
"      Warning: Change the above parameters only if you really know what you are doing!\n";
static const char * const usage2 =
"\n"
"    -i iterations      number of iterations (number of bitstreams) to test (if no -A, def: 1) (same as -P 7=iterations)\n"
"\n"
"    -I reportCycle     report after completion of reportCycle iterations (def: 0: do not report)\n"
"    -O                 try to mimic output format of legacy code (def: don't be output compatible)\n"
"\n"
"    -w workDir         write experiment results under workDir (def: .)\n"
"    -c                 don't create any directories needed for creating files (def: do create)\n"
"    -s                 create result.txt, data*.txt, and stats.txt (def: don't create)\n"
"    -F format          randdata format: 'r': raw binary, 'a': ASCII '0'/'1' chars (def: 'r')\n"
"    -S bitcount        Number of bits to process in a single iteration (def: 1048576 == 1024*1024) (same as -P 9=bitcount)\n"
"    -j jobnum          seek into randdata, jobnum * bitcount * iterations bits (def: 0)\n"
"                       Seeking is disabled if randdata is - and data for all jobs is read from beginning of standard input.\n"
"\n"
"    -m mode            b --> test pseudo-random data from from randdata (default mode)\n"
"                       i --> test the given data, but not assess it, and instead save the p-values in a binary filename\n"
"                             of the form: workDir/sts.__jobnum__.__iterations__.__bitcount__.pvalue\n"
"                       a --> collect the p-values from the binary files specified from '-d pvaluesdir' and assess them\n"
"\n"
"    -T numOfThreads    custom number of threads for this run (default: takes the number of cores of the CPU)\n"
"\n"
"    -d pvaluesdir      path to the folder with the binary files with previously computed p-values (requires mode -m a)\n"
"                       This will assess p-values found files of the form:\n"
"\n"
"                           pvaluesdir/sts.__jobnum__.__iterations__.__bitcount__.pvalues\n"
"\n"
"                       where __bitcount__ is a specified bitcount value.  The __iterations__ field is the number of\n"
"                       iterations that the given file holds.  The __jobnum__ field is the job number and is ignored.\n"
"                       All other files and directories under pvaluesdir are ignored.\n"
"\n"
"    -h                 print this message and exit\n"
"\n"
"    randdata           path to the input file to test (required for -m b and -m i, optional for -A and -m a)\n"
"                       If randdata is -, data is read from the beginning standard input. No seek for -j jobnum is performed.\n";
/* *INDENT-ON* */

#ifdef _WIN32
/*
 * Get next token from string *stringp, where tokens are possibly-empty
 * strings separated by characters from delim.
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no moretokens).
 *
 * If *stringp is NULL, strsep returns NULL.
 */
char *strsep(char **stringp, const char *delim)
{
	char *s;
	int sc;
	char *tok;
	if ((s = *stringp)== NULL)
		return (NULL);
	for (tok = s;;) {
		int c = *s++;
		const char *spanp = delim;
		do {
			if ((sc =*spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}
#endif

/*
 * parse_args - parse command line arguments and setup run state
 *
 * given:
 *      state           // run state it initialize and set according to command line
 *      argc            // command line arg count
 *      argv            // array of argument strings
 *
 * This function does not return on error.
 */
void
parse_args(struct state *state, int argc, char **argv)
{
	int option;		// getopt() parsed option
	extern char *optarg;	// Parsed option argument
	extern int optind;	// Index to the next argv element to parse
	extern int opterr;	// 0 ==> disable internal getopt() error messages
	extern int optopt;	// Last known option character returned by getopt()
	int scan_cnt;		// Number of items scanned by sscanf()
	char *brkt;		// Last state of strtok_r()
	char *phrase;		// String without separator as parsed by strtok_r()
	long int testnum;	// Parsed test number
	long int num;		// Parsed parameter number
	long int value;		// Parsed parameter integer value
	double d_value;		// Parsed parameter floating point
	bool success = false;	// true if str2longint was successful
	int test_cnt = 0;
	long int i;

	/*
	 * Record the program name
	 */
	program = argv[0];
	if (program == NULL) {
		program = "((NULL))";	// paranoia
	} else if (program[0] == '\0') {
		program = "((empty))";	// paranoia
	}

	/*
	 * Check preconditions (firewall)
	 */
	if (argc <= 0 || state == NULL) {
		err(1, __func__, "called with bogus args");
	}

	/*
	 * Initialize state to default state
	 */
	*state = defaultstate;

	/*
	 * Parse the command line arguments
	 */
	opterr = 0;
	brkt = NULL;
	while ((option = getopt(argc, argv, "v:Abt:g:pP:S:i:I:Ow:csf:F:j:m:T:d:h")) != -1) {
		switch (option) {

		case 'v':	// -v debuglevel
			debuglevel = str2longint(&success, optarg);
			if (success == false) {
				usage_errp(1, __func__, "error in parsing -v debuglevel: %s", optarg);
			} else if (debuglevel < 0) {
				usage_err(1, __func__, "error debuglevel: %lu must >= 0", debuglevel);
			}
			break;

		case 'A':	// -A (ask a human obsolete interactive mode)
			state->batchmode = false;
			state->promptFlag = true;
			break;

		case 'b':	// -b is now obsolete because batch is the default
			usage_err(1, __func__, "-b is no longer required as batch is now the default");
			break;

		case 't':	// -t test1[,test2]..
			state->testVectorFlag = true;

			/*
			 * Parse each comma separated arg
			 */
			for (phrase = strtok_r(optarg, ",", &brkt); phrase != NULL; phrase = strtok_r(NULL, ",", &brkt)) {

				/*
			 	 * Determine test number
			 	 */
				testnum = str2longint(&success, phrase);
				if (success == false) {
					usage_errp(1, __func__,
						   "-t test1[,test2].. must only have comma separated integers: %s", phrase);
				}

				/*
				 * Enable all tests if testnum is 0 (special case)
				 */
				if (testnum == 0) {
					for (i = 1; i <= NUMOFTESTS; i++) {
						state->testVector[i] = true;
					}
				} else if (testnum < 0 || testnum > NUMOFTESTS) {
					usage_err(1, __func__, "-t test: %lu must be in the range [0-%d]", testnum,
						  NUMOFTESTS);
				} else {
					state->testVector[testnum] = true;
				}
			}
			break;

		case 'g':	// -g generator
			usage_err(1, __func__, "-g no longer supported, -g 0 (read from file) is the only generator\n"
				  "For all other generators use the generator tool, or online data files");
			break;

		case 'P':	// -P num=value[,num=value]..

			/*
			 * Parse each comma separated arg
			 */
			for (phrase = strtok_r(optarg, ",", &brkt); phrase != NULL; phrase = strtok_r(NULL, ",", &brkt)) {

				/*
				 * Parse parameter number
				 */
				scan_cnt = sscanf(phrase, "%ld=", &num);
				if (scan_cnt == EOF) {
					usage_errp(1, __func__,
						   "-P num=value[,num=value].. end of string parsing num=..: %s", phrase);
				} else if (scan_cnt != 1) {
					usage_err(1, __func__,
						  "-P num=value[,num=value].. "
						  "failed to parse num=value, expecting integer=value: %s", phrase);
				}
				if (num < MIN_PARAM || num > MAX_PARAM) {
					usage_err(1, __func__,
						  "-P num=value[,num=value].. num: %lu must be in the range [1-%d]", num,
						  MAX_PARAM);
				}

				/*
				 * Parse parameter value
				 */
				if (num <= MAX_INT_PARAM) {

					// Parse parameter number as an integer
					scan_cnt = sscanf(phrase, "%ld=%ld", &num, &value);
					if (scan_cnt == EOF) {
						usage_errp(1, __func__,
							   "-P num=value[,num=value].. error parsing integer=integer: %s",
							   phrase);
					} else if (scan_cnt != 2) {
						usage_err(1, __func__,
							  "-P num=value[,num=value].. "
							  "failed to parse num=value, expected integer=integer: %s", phrase);
					}
					if (num < 0 || num > MAX_PARAM) {
						usage_err(1, __func__,
							  "-P num=value[,num=value].. num: %lu must be in range [1-%d]", num,
							  MAX_PARAM);
					}
					change_params(state, num, value, 0.0);
				} else {

					// Parse parameter number as a floating point number
					scan_cnt = sscanf(phrase, "%ld=%lf", &num, &d_value);
					if (scan_cnt == EOF) {
						usage_errp(1, __func__,
							   "-P num=value[,num=value].. error parsing a num=float: %s", phrase);
					} else if (scan_cnt != 2) {
						usage_err(1, __func__,
							  "-P num=value[,num=value].. "
							  "failed to parse a num=value, expecting integer=float: %s", phrase);
					}
					if (num < 0 || num > MAX_PARAM) {
						usage_err(1, __func__,
							  "-P num=value[,num=value].. num: %lu must within range [1-%d]", num,
							  MAX_PARAM);
					}
					change_params(state, num, 0, d_value);
				}
			}
			break;

		case 'p':	// -p is now obsolete because batch is the default
			usage_err(1, __func__, "-p is no longer needed");
			break;
                case 'S':      // -S just a workaround
                        state->tp.n = str2longint(&success, optarg);
                        break;

		case 'i':	// -i iterations
			state->iterationFlag = true;
			state->tp.numOfBitStreams = str2longint(&success, optarg);
			if (success == false) {
				usage_errp(1, __func__, "error in parsing -i iterations: %s", optarg);
			}
			if (state->tp.numOfBitStreams < 1) {
				usage_err(1, __func__,
					  "iterations (number of bit streams): %lu can't be less than 1",
					  state->tp.numOfBitStreams);
			}
			break;

		case 'I':	// -I reportCycle
			state->reportCycleFlag = true;
			state->reportCycle = str2longint(&success, optarg);
			if (success == false) {
				usage_errp(1, __func__, "error in parsing -I reportCycle: %s", optarg);
			}
			if (state->reportCycle < 0) {
				usage_err(1, __func__, "-I reportCycle: %lu must be >= 0", state->reportCycle);
			}
			break;

		case 'O':	// -O (try to mimic output format of legacy code)
			state->legacy_output = true;
			break;

		case 'w':	// -w workDir

			/*
			 * Write experiment results under workDir
			 */
			state->workDirFlag = true;
			state->workDir = strdup(optarg);
			if (state->workDir == NULL) {
				errp(1, __func__, "strdup of %lu bytes for -w workDir failed", strlen(optarg));
			}
			break;

		case 'c':	// -c (do not create directories)
			state->subDirsFlag = true;
			state->subDirs = false;	// do not create directories
			break;

		case 's':	// -s (create result.txt and stats.txt)
			state->resultstxtFlag = true;
			break;

		case 'F':	// -F format: 'r' or '1': raw binary, 'a' or '0': ASCII '0'/'1' chars
			state->dataFormatFlag = true;
			state->dataFormat = (enum format) (optarg[0]);
			switch (state->dataFormat) {
			case FORMAT_0:
				state->dataFormat = FORMAT_ASCII_01;
				break;
			case FORMAT_1:
				state->dataFormat = FORMAT_RAW_BINARY;
				break;
			case FORMAT_ASCII_01:
			case FORMAT_RAW_BINARY:
				break;
			default:
				err(1, __func__, "-F format: %s must be r or a", optarg);
			}
			if (optarg[1] != '\0') {
				err(1, __func__, "-F format: %s must be a single character: r or a", optarg);
			}
			break;

		case 'f':
			usage_err(1, __func__, "-f is no longer needed, instead put randdata as last argument");
			break;

		case 'j':	// -j jobnum (seek into randomData unless randdata is stdin)
			state->jobnumFlag = true;
			state->jobnum = str2longint(&success, optarg);
			if (success == false) {
				usage_errp(1, __func__, "error in parsing -j jobnum: %s", optarg);
			}
			if (state->jobnum < 0) {
				usage_err(1, __func__, "-j jobnum: %lu must be greater than 0", state->jobnum);
			}
			break;

		case 'm':	// -m mode (w-->write only. i-->iterate only, a-->assess only, b-->iterate & assess)
			state->runModeFlag = true;
			if (optarg[0] == '\0' || optarg[1] != '\0') {
				usage_err(1, __func__, "-m mode must be a single character: %s", optarg);
			}
			switch (optarg[0]) {
			case MODE_ITERATE_AND_ASSESS:
				state->runMode = MODE_ITERATE_AND_ASSESS;
				break;
			case MODE_ITERATE_ONLY:
				state->runMode = MODE_ITERATE_ONLY;
				break;
			case MODE_ASSESS_ONLY:
				state->runMode = MODE_ASSESS_ONLY;
				break;
			default:
				usage_err(1, __func__, "-m mode must be one of w, b, i or a: %c", optarg[0]);
				break;
			}
			break;

		case 'T':	// -v debuglevel
			state->numberOfThreadsFlag = true;
			state->numberOfThreads = str2longint(&success, optarg);
			if (success == false) {
				usage_errp(1, __func__, "error in parsing -T numOfThreads: %s", optarg);
			}
			if (state->numberOfThreads < 0) {
				usage_err(1, __func__, "-T numOfThreads: %lu must be >= 0", state->numberOfThreads);
			}
			break;

		case 'd':	// -d folder with precomputed .pvalues files
			state->pvalues_dir = strdup(optarg);
			if (state->pvalues_dir == NULL) {
				errp(1, __func__, "strdup of %lu bytes for -w pvalues_dir failed", strlen(optarg));
			}
			break;

		case 'h':	// -h (print out help)
			if (program == NULL) {
				fprintf(stderr, "usage: sts %s%s", usage, usage2);
			} else {
				fprintf(stderr, "usage: %s %s%s", program, usage, usage2);
			}
			fprintf(stderr, "\nVersion: %s\n", version);
			exit(0);
			break;

		case '?':
			usage_err(1, __func__, "unknown option: -%c", (char) optopt);
			break;

		default:
			usage_err(1, __func__, "getopt returned an unexpected error");
			break;
		}
	}

	// parse last argument based on mode
	if (optind == argc - 1) {
		state->randomDataPath = strdup(argv[argc-1]);
		if (state->randomDataPath == NULL) {
			errp(1, __func__, "strdup of %lu bytes for randdata arg", strlen(optarg));
		}
		if (strcmp(state->randomDataPath, "-") == 0) {
			state->stdinData = true;
		}
		state->randomDataArg = true;
	} else if (optind < argc - 1) {
		usage_err(1, __func__, "unexpected arguments");
	}
	switch (state->runMode) {
	case MODE_ITERATE_AND_ASSESS:
		/*FALLTHRU*/
	case MODE_ITERATE_ONLY:
		if (state->randomDataArg == false) {
			usage_err(1, __func__, "missing randdata argument");
		}
		break;
	case MODE_ASSESS_ONLY:
		break;
	default:
		err(1, __func__, "unknown run mode: %u", state->runMode);
		break;
	}


	// if reading random data from stdin, we cannot be interactive
	if (state->stdinData == true) {
		if (state->batchmode == false) {
			usage_err(1, __func__, "-A not allowed when randdata is - (reading data from standard input)");
		}
		if (state->iterationFlag == false && state->runMode != MODE_ASSESS_ONLY) {
			usage_err(1, __func__, "-i bitstreams or -m a requited when randdata is - "
					       "(reading data from standard input)");
		}
	}

	/*
	 * Ask how many iterations have to be performed unless batch mode (-b) is enabled or -i bitstreams was not given
	 */
	if (state->batchmode == false && state->iterationFlag == false && state->stdinData == false) {
		// Ask question
		printf("   How many bitstreams? ");
		fflush(stdout);

		// Read numberic answer
		state->tp.numOfBitStreams = getNumber(stdin, stdout);
		putchar('\n');
		fflush(stdout);
	}

	/*
	 * If no -B and no -t test given, enable all tests
	 *
	 * Test 0 is an historical alias for all tests enabled.
	 */
	if ((state->batchmode == true && state->testVectorFlag == false) || state->testVector[0] == true) {
		// Under -b without and -t test, enable all tests
		for (i = 1; i <= NUMOFTESTS; i++) {
			state->testVector[i] = true;
		}
	}

	/*
	 * Count the number of tests enabled
	 *
	 * We do not count test 0 (historical alias for all tests enabled).
	 */
	if (state->batchmode == true) {
		for (i = 1; i <= NUMOFTESTS; i++) {
			if (state->testVector[i] == true) {
				++test_cnt;
			}
		}
	}
	if (test_cnt == 0 && state->batchmode == true) {
		err(1, __func__, "no tests enabled");
	}

	/*
	 * Set the number of uniformity bins to sqrt(iterations) if running in non-legacy mode and
	 * no custom number was provided
	 */
	if (state->uniformityBinsFlag == false && state->legacy_output == false) {
		state->tp.uniformity_bins = (long int) sqrt(state->tp.numOfBitStreams);
	}

	/*
	 * Set the number of uniformity bins back to their default value if a custom number was
	 * provided but the legacy mode is on
	 */
	if (state->uniformityBinsFlag == true && state->legacy_output == true) {
		warn(__func__, "The number of uniformity bins was set back to %d due to '-O' legacy mode flag",
		    DEFAULT_UNIFORMITY_BINS);
		state->tp.uniformity_bins = DEFAULT_UNIFORMITY_BINS;
	}

	/*
	 * If no custom number of threads was set, set the number of threads to be equal to the minimum
	 * between the number of bitstreams and the number of cores of the computer where sts is running.
	 */
	else if (state->numberOfThreadsFlag == false) {
#ifndef _WIN32
		state->numberOfThreads = MIN(sysconf(_SC_NPROCESSORS_ONLN), state->tp.numOfBitStreams);
#endif
	}

	/*
	 * If a custom number of threads was set and this number is greater than the number of processors
	 * in the computer where sts is running, fire a warning to the user that this will not benefit sts.
	 */
#ifndef _WIN32
	if (state->numberOfThreadsFlag == true && state->numberOfThreads > sysconf(_SC_NPROCESSORS_ONLN)) {
		warn(__func__, "You selected a number of threads which is greater than the number of cores in this computer."
				     " For better performance, you should choose a number of threads < %ld.",
		     sysconf(_SC_NPROCESSORS_ONLN));
	}
#endif

	/*
	 * If a custom number of threads was set and this number is greater than the number of bitstreams
	 * (aka iterations) set, fire a warning to the user that only $numOfBitstreams threads will be used.
	 */
	if (state->numberOfThreadsFlag == true && state->numberOfThreads > state->tp.numOfBitStreams) {
		warn(__func__, "You chose to use %ld threads. However this number is greater than the number of bitstreams, which"
				     " you set to %ld. Therefore only %ld threads will be used.", state->numberOfThreads,
		     state->tp.numOfBitStreams, state->tp.numOfBitStreams);
		state->numberOfThreads = state->tp.numOfBitStreams;
	}

	/*
	 * Look for the matching .pvalues files in the folder given with -d
	 */
	if (state->pvalues_dir != NULL) {
		DIR *dir;
		struct dirent *entry;
		struct stat path_stat;

		if ((dir = opendir(state->pvalues_dir)) != NULL) {

			/*
			 * Set the number of bitstreams to 0, since we will count the number of iterations
			 * from the filenames (assuming they were not renamed).
			 */
			state->tp.numOfBitStreams = 0;

			/*
			 * Consider every entry within dir
			 */
			while ((entry = readdir (dir)) != NULL) {

				/*
				 * Take the full path of each entry
				 */
				char *fullpath = malloc(strlen(state->pvalues_dir) + strlen(entry->d_name) + 2);
				sprintf(fullpath, "%s/%s", state->pvalues_dir, entry->d_name);
				stat(fullpath, &path_stat);

				/*
				 * Check if the each entry is a regular file
				 */
				if (S_ISREG(path_stat.st_mode)) {

					int token_number = 0;
					char **parsed_tokens = malloc(sizeof(*parsed_tokens) * 5);
					char *entry_name, *to_free;	// entry_name will be modified by strsep
					to_free = entry_name = strdup(entry->d_name);

					/*
					 * Take the first 5 (if present) tokens of the filename, where for token
					 * we mean the sub-strings delimited by a '.' (dot).
					 */
					while (token_number < 5 &&
							(*(parsed_tokens + token_number) = strsep(&entry_name, ".")) != NULL) {
						token_number++;
					}

					/*
					 * If we were able to count 5 tokens and the tokens match the naming pattern
					 * of the sts p-values files (sts.*.*.$n.pvalues)
					 */
					if (token_number == 5 && strcmp(*parsed_tokens, "sts") == 0 &&
							strcmp(*(parsed_tokens + token_number - 1), "pvalues") == 0 &&
							atoi(*(parsed_tokens + token_number - 2)) == state->tp.n) {

						/*
						 * Count the number of iterations done in the run for this file.
						 */
						state->tp.numOfBitStreams += atoi(*(parsed_tokens + token_number - 3));

						/*
						 * Store the filename in the list of filenames for opening it later.
						 */
						append_string_to_linked_list(&state->filenames, entry->d_name);

					}

					/*
					 * Free the pointer which we don't need anymore
					 */
					free(to_free);
				}

				/*
				 * Free another pointer which we don't need anymore
				 */
				free(fullpath);
			}

			/*
			 * Close the directory
			 */
			closedir (dir);

		} else {
			/* could not open directory */
			err(1, __func__, "Could not open the directory: %s", state->pvalues_dir);
		}
	}

	/*
	 * When running in ASSESS_ONLY MODE
	 */
	if (state->runMode == MODE_ASSESS_ONLY) {

		if (state->resultstxtFlag == true) {
			warn(__func__, "You have chosen to use the sts in mode 'a' (assess only). In this mode the -s flag is "
					"not supported. This run won't produce any stats.txt or results.txt file.");
			state->resultstxtFlag = false;
		}
	}

	/*
	 * verify that bitcount is OK
	 */
	if ((state->tp.n % 8) != 0) {
		usage_err(1, __func__,
			  "bitcount(n): %ld must be a multiple of 8. The added complexity of supporting "
					  "a sequence that starts or ends on a non-byte boundary outweighs the "
					  "convenience of permitting arbitrary bit lengths", state->tp.n);
	}
	if (state->tp.n < GLOBAL_MIN_BITCOUNT) {
		usage_err(1, __func__, "bitcount(n): %ld must >= %d", state->tp.n, GLOBAL_MIN_BITCOUNT);
	}

	/*
	 * Report on how we will run, if debugging
	 */
	if (debuglevel > 0) {
		print_option_summary(state, "parsed command line");
	}

	return;
}


static void
change_params(struct state *state, long int parameter, long int value, double d_value)
{
	/*
	 * Check preconditions (firewall)
	 */
	if (state == NULL) {
		err(2, __func__, "state arg is NULL");
	}

	/*
	 * Load integer or floating point value into a parameter
	 */
	switch (parameter) {
	case PARAM_blockFrequencyBlockLength:
		state->tp.blockFrequencyBlockLength = value;
		break;
	case PARAM_nonOverlappingTemplateBlockLength:
		state->tp.nonOverlappingTemplateLength = value;
		break;
	case PARAM_overlappingTemplateBlockLength:
		state->tp.overlappingTemplateLength = value;
		break;
	case PARAM_approximateEntropyBlockLength:
		state->tp.approximateEntropyBlockLength = value;
		break;
	case PARAM_serialBlockLength:
		state->tp.serialBlockLength = value;
		break;
	case PARAM_linearComplexitySequenceLength:
		state->tp.linearComplexitySequenceLength = value;
		break;
	case PARAM_numOfBitStreams:
		state->uniformityBinsFlag = true;
		state->tp.numOfBitStreams = value;
		break;
	case PARAM_uniformity_bins:
		state->tp.uniformity_bins = value;
		break;
	case PARAM_n:
		state->tp.n = value;
		break;
	case PARAM_uniformity_level:
		state->tp.uniformity_level = d_value;
		break;
	case PARAM_alpha:
		state->tp.alpha = d_value;
		break;
	default:
		err(2, __func__, "invalid parameter option: %ld", parameter);
		break;
	}

	return;
}

void
print_option_summary(struct state *state, char *where)
{
	int j;
	int test_cnt = 0;

	/*
	 * Check preconditions (firewall)
	 */
	if (state == NULL) {
		err(3, __func__, "state arg is NULL");
	}
	if (where == NULL) {
		err(3, __func__, "where arg is NULL");
	}

	/*
	 * Report on high level state
	 */
	dbg(DBG_MED, "High level state for: %s", where);
	dbg(DBG_MED, "\tsts version: %s", version);
	dbg(DBG_MED, "\tdebuglevel = %ld", debuglevel);
	if (state->batchmode == true) {
		dbg(DBG_MED, "\tRunning in (non-interactive) batch mode");
		if (state->runModeFlag == true) {
			switch (state->runMode) {

			case MODE_ITERATE_AND_ASSESS:
				dbg(DBG_MED, "\tWill test pseudo-random data from from a file");
				break;

			case MODE_ITERATE_ONLY:
				dbg(DBG_MED, "\tWill test the given data, but not assess it, and instead save the p-values "
						"in a binary file");
				break;

			case MODE_ASSESS_ONLY:
				dbg(DBG_MED, "\tCollect the p-values from the binary files specified from '-d file...' and "
						"assess them'");
				break;

			default:
				dbg(DBG_MED, "\tUnknown assessment mode: %c", state->runMode);
				break;
			}
		}
		dbg(DBG_MED, "\tTesting %lld bits of data (%lld bytes)", (long long) state->tp.numOfBitStreams *
				(long long) state->tp.n, (((long long) state->tp.numOfBitStreams *
				(long long) state->tp.n) + BITS_N_BYTE - 1) / BITS_N_BYTE);
		dbg(DBG_MED, "\tPerforming %ld iterations each of %ld bits\n", state->tp.numOfBitStreams, state->tp.n);
	} else {
		dbg(DBG_MED, "\tobsolete interactive mode\n");
	}

	/*
	 * Report on tests enabled
	 */
	dbg(DBG_MED, "Tests enabled:");
	for (j = 1; j <= NUMOFTESTS; j++) {
		if (state->testVector[j]) {
			dbg(DBG_MED, "\ttest[%d] %s: enabled", j, state->testNames[j]);
			++test_cnt;
		}
	}
	if (state->batchmode == true) {
		dbg(DBG_MED, "\t%d tests enabled\n", test_cnt);
	} else {
		if (state->testVectorFlag == true) {
			dbg(DBG_LOW, "\t-t used, will NOT prompt for tests to enable\n");
		} else {
			dbg(DBG_LOW, "\tno -t used, will prompt for tests to enable\n");
		}
	}

	/*
	 * Report on generator (or file) to be used
	 */
	if (state->batchmode == true) {
		dbg(DBG_LOW, "Testing data from file: %s", state->randomDataPath);
		if (strcmp(state->randomDataPath, "-") == 0) {
		    dbg(DBG_LOW, "  test data will be read from standard input (stdin)");
		}
	} else {
		dbg(DBG_LOW, "Will prompt user for generator to use");
	}

	/*
	 * Report detailed state summary
	 */
	dbg(DBG_MED, "Details of run state:");
	if (state->iterationFlag == true) {
		dbg(DBG_MED, "\t-i iterations was given");
	} else {
		dbg(DBG_MED, "\tno -i iterations was given");
	}
	dbg(DBG_MED, "\t  iterations (bitstreams): -i %ld", state->tp.numOfBitStreams);
	if (state->reportCycleFlag == true) {
		dbg(DBG_MED, "\t-I reportCycle was given");
	} else {
		dbg(DBG_MED, "\tno -I reportCycle was given");
	}
	if (state->reportCycle == 0) {
		dbg(DBG_MED, "\t  will not report on progress of iterations");
	} else {
		dbg(DBG_MED, "\t  will report on progress every %ld iterations", state->reportCycle);
	}
	if (state->legacy_output == true) {
		dbg(DBG_MED, "\t-O was given, legacy output mode where reasonable");
	} else {
		dbg(DBG_MED, "\tno -O was given, legacy output is not important");
	}
	if (state->runModeFlag == true) {
		dbg(DBG_MED, "\t-m node was given");
	} else {
		dbg(DBG_MED, "\tno -m mode was given");
	}
	switch (state->runMode) {
	case MODE_ITERATE_AND_ASSESS:
		dbg(DBG_MED, "\t  -m b: test pseudo-random data from a file");
		break;
	case MODE_ITERATE_ONLY:
		dbg(DBG_MED, "\t  -m i: test the given data, but not assess it, and instead save the p-values in a binary file");
		break;
	case MODE_ASSESS_ONLY:
		dbg(DBG_MED, "\t  -m a: collect the p-values from the binary files specified from '-d file...' and assess them");
		break;
	default:
		dbg(DBG_MED, "\t  -m %c: unknown runMode", state->runMode);
		break;
	}
	dbg(DBG_MED, "\tworkDir: -w %s", state->workDir);
	if (state->subDirsFlag == true) {
		dbg(DBG_MED, "\t-c was given");
	} else {
		dbg(DBG_MED, "\tno -c was given");
	}
	if (state->subDirs == true) {
		dbg(DBG_MED, "\t  create directories needed for writing to any file");
	} else {
		dbg(DBG_MED, "\t  do not create directories, assume they exist");
	}
	if (state->resultstxtFlag == true) {
		dbg(DBG_MED, "\t-s was given");
		dbg(DBG_MED, "\t  create result.txt, data*.txt and stats.txt");
	} else {
		dbg(DBG_MED, "\tno -s was given");
		dbg(DBG_MED, "\t  do not create result.txt, data*.txt and stats.txt");
	}
	if (state->dataFormatFlag == true) {
		dbg(DBG_MED, "\t-F format was given");
	} else {
		dbg(DBG_MED, "\tno -F format was given");
	}
	switch (state->dataFormat) {
	case FORMAT_ASCII_01:
	case FORMAT_0:
		dbg(DBG_MED, "\t  read as ASCII '0' and '1' character bits");
		break;
	case FORMAT_RAW_BINARY:
	case FORMAT_1:
		dbg(DBG_MED, "\t  read as raw binary 8 bits per byte");
		break;
	default:
		dbg(DBG_MED, "\t  unknown format: %c", (char) state->dataFormat);
		break;
	}
	dbg(DBG_MED, "\tjobnum: -j %ld", state->jobnum);
	if (state->jobnumFlag == true) {
		dbg(DBG_MED, "\t-j jobnum was set to %ld", state->jobnum);
		if (strcmp(state->randomDataPath, "-") == 0) {
			dbg(DBG_MED, "\t  reading from stdin so we will not skip %lld bytes of data",
			    (long long) state->jobnum *
			      (((long long) state->tp.numOfBitStreams * (long long) state->tp.n) + BITS_N_BYTE - 1 / BITS_N_BYTE));
		} else {
			dbg(DBG_MED, "\t  will skip %lld bytes of data before processing",
			    (long long) state->jobnum *
			      (((long long) state->tp.numOfBitStreams * (long long) state->tp.n) + BITS_N_BYTE - 1 / BITS_N_BYTE));
		}
	} else {
		dbg(DBG_MED, "\tno -j jobnum was given");
		dbg(DBG_MED, "\t  will start processing at the beginning of data");
	}
	if (state->numberOfThreadsFlag == true) {
		dbg(DBG_MED, "\t-T numOfThreads was given");
	} else {
		dbg(DBG_MED, "\tno -T numOfThreads was given");
	}
	dbg(DBG_MED, "\t  will use %ld threads\n", state->numberOfThreads);

	/*
	 * Report on test parameters
	 */
	dbg(DBG_MED, "Test parameters:");
	if (state->batchmode == false) {
		dbg(DBG_MED, "\t(showing default parameters)");
	}
	dbg(DBG_MED, "\tsingleBitStreamLength = %ld", state->tp.n);
	dbg(DBG_MED, "\tblockFrequencyBlockLength = %ld", state->tp.blockFrequencyBlockLength);
	dbg(DBG_MED, "\tnonOverlappingTemplateBlockLength = %ld", state->tp.nonOverlappingTemplateLength);
	dbg(DBG_MED, "\toverlappingTemplateBlockLength = %ld", state->tp.overlappingTemplateLength);
	dbg(DBG_MED, "\tserialBlockLength = %ld", state->tp.serialBlockLength);
	dbg(DBG_MED, "\tlinearComplexitySequenceLength = %ld", state->tp.linearComplexitySequenceLength);
	dbg(DBG_MED, "\tapproximateEntropyBlockLength = %ld", state->tp.approximateEntropyBlockLength);
	dbg(DBG_MED, "\tnumOfBitStreams = %ld", state->tp.numOfBitStreams);
	dbg(DBG_MED, "\tbins = %ld", state->tp.uniformity_bins);
	dbg(DBG_MED, "\tuniformityLevel = %f", state->tp.uniformity_level);
	if (state->batchmode == true) {
		dbg(DBG_MED, "\talpha = %f\n", state->tp.alpha);
	} else {
		dbg(DBG_MED, "\talpha = %f", state->tp.alpha);
		if (state->promptFlag == true) {
			dbg(DBG_LOW, "\t    -A was given: will prompt for any changes to default parameters\n");
		} else {
			dbg(DBG_LOW, "\t    -A was not given: will NOT prompt for any changes to default parameters\n");
		}
	}

	/*
	 * report on the randdata filename
	 */
	if (state->randomDataArg == true) {
		dbg(DBG_MED, "\tranddata arg was given");
	} else {
		dbg(DBG_MED, "\tno randdata arg given");
	}
	if (state->stdinData == true) {
		dbg(DBG_MED, "\t  randomDataPath is from standard input\n");
	} else {
		dbg(DBG_MED, "\t  randomDataPath: %s\n", state->randomDataPath);
	}

	return;
}
