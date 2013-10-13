/* File: Test23d.c
   Read in asset file with baseline theta and estimated delta-theta & cost values.
   Compute optimum delta-theta/cost combinations.
   
   Deviate dtheta values across multiple points and runs.
   Deviation = % change in dtheta value.
   Coverage  = % dtheta values (i.e., points) that will be changed.
   Calculate average dtheta for each run.

   Output in Comma Delimited File format

   Execute:  Test23d dtheta.dat sensitivity.txt

   Modification from Test23c.c:		1. Change deviation selection procedure.
									2. Incorporate revised theta computation program, Test22e.c.
*/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* CONSTANT DEFINITIONS */
#define INREC_ROWS		5
#define INREC_COLS		11
#define	INREC_VALUES	2
#define OUTREC_COLS		16
#define ROW_ASSET		0
#define	ROW_DPDIS		1
#define ROW_DPDEF		2
#define ROW_DPDEN		3
#define	ROW_DPDIM		4
#define COL_ID			0
#define	COL_TYPE		1
#define	COL_LOC			2
#define	COL_PDIS		3
#define	COL_PDEF		4
#define	COL_PDEN		5
#define	COL_PDIM		6
#define COL_PDAM		7
#define	COL_THETA		8
#define COL_DTHETA		9
#define COL_TCOST		10
#define COL_DDIS		11
#define COL_DDEF		12
#define	COL_DDEN		13
#define COL_DDIM		14
#define COL_RUN			15
#define IDX_POINTS		0
#define IDX_DATA		0
#define IDX_DTHETA		0
#define IDX_COST		1 
#define MAX_TABLES		4
#define MAX_TYPE		13
#define MAX_LOC			50
#define	MAX_STEPS		10
#define STEP_INCR		0.10

/* FUNCTION HEADERS */
int getnext (FILE *ifp, float rec[][INREC_COLS][INREC_VALUES]);
int tallypts(float rec[][INREC_COLS][INREC_VALUES], int tally[][INREC_COLS][INREC_VALUES]);
int select	(int pct);
int deviate (float *dtheta, int *dcount, int dcov, int dpts, int *dsum, int dev);
int	prtrec  (FILE *ofp, float rec[][INREC_COLS][INREC_VALUES]);
int prtout	(FILE *ofp, float outrec[]);

/* MAIN PROGRAM */
int main(int argc, char *argv[]) 
{
/* DATA DECLARATIONS */
    FILE *ifp;											/* Input File Pointer 
														*/
	FILE *ofp;											/* Output File Pointer
														*/
	float rec[INREC_ROWS][INREC_COLS][INREC_VALUES];			/* Input Record Data
															  Row 0:  Asset Data
															  Row 1:  Delta P(dis) Data
															  Row 2:  Delta P(def) Data
															  Row 3:  Delta P(den) Data
															  Row 4:  Delta P(dim) Data
															               0  1    2        3      4      5      6      7      8
															  Asset Data:  ID Type Location P(dis) P(def) P(den) P(dim) %(dam) Theta
															               0    1   2   3   4   5   6   7   8   9   10
															  Delta Data:  #pts pt1 pt2 pt3 pt4 pt5 pt6 pt7 pt8 pt9 pt10
															               0            1
															  Point Data:  delta-theta	cost
														*/
	int		dis_pts, def_pts, den_pts, dim_pts,
			disx, defx, denx, dimx;
	float	dis_dtheta, dis_cost, 
			def_dtheta, def_cost,
			den_dtheta, den_cost,
			dim_dtheta, dim_cost,
			dis_prop, def_prop, den_prop, dim_prop,
			dis_tsave, dis_csave,
			def_tsave, def_csave,
			den_tsave, den_csave,
			dim_tsave, dim_csave;
	int		runs, run_num;
	float	theta_prop, theta_cost, theta_max;
	int		i, asset_type, asset_loc;
	float	tflag_dtheta[MAX_TYPE+1], tflag_cost[MAX_TYPE+1], dis_opt[MAX_TYPE+1],
			lflag_dtheta[MAX_LOC+1],  lflag_cost[MAX_LOC+1],  dim_opt[MAX_LOC+1];

	float	outrec[OUTREC_COLS];						/* Output Record Data
														      <=========== same as input record format ===========>
															  0  1    2        3      4      5      6      7      8		9	   10	 11	    12	   13	  14	 15
															  ID Type Location P(dis) P(def) P(den) P(dim) %(dam) Theta DTheta TCost D(dis) D(def) D(den) D(dim) Run#
														*/
	int		max_tests, tidx, num_assets,
			dis_dev, dis_cov,
			def_dev, def_cov,
			den_dev, den_cov,
			dim_dev, dim_cov,
			dam_dev, dam_cov,
			tally[INREC_ROWS][INREC_COLS][INREC_VALUES],
			dis_count, def_count, den_count, dim_count, dam_count,
			dis_mod, def_mod, den_mod, dim_mod, dam_mod,
			dis_sum, def_sum, den_sum, dim_sum, dam_sum,
			dis_np, def_np, den_np, dim_np, dam_np;
	float	sum_dtheta, cum_dtheta, sum_theta, cum_theta; 

	int		step,
			dis_cumnp, dis_cumsum,
			def_cumnp, def_cumsum,
			den_cumnp, den_cumsum,
			dim_cumnp, dim_cumsum,
			dam_cumnp, dam_cumsum;

/* DATA PROCESSING */
	printf ("Start Program...\n");
    
	/* Get input file name. */
    if (argc == 1)
    {
        printf ("  No input file specified.\n");
        return -1;
    }
    
	/* Get output file name. */
    if (argc == 2)
    {
        printf ("  No output file specified.\n");
        return -1;
    }

	/* Get test input parameters */
	printf ("\n  Enter # tests: ");
	scanf  ("%d", &max_tests);
	printf ("\n    P(dis) %% coverage : ");
	scanf  ("%d", &dis_cov);
	printf ("\n    P(def) %% coverage : ");
	scanf  ("%d", &def_cov);
	printf ("\n    P(den) %% coverage : ");
	scanf  ("%d", &den_cov);
	printf ("\n    P(dim) %% coverage : ");
	scanf  ("%d", &dim_cov);
	printf ("\n    P(dam) %% coverage : ");
	scanf  ("%d", &dam_cov);

	printf ("\n");
	printf ("  infile  = %s\n", argv[1]);
	printf ("  outfile = %s\n", argv[2]);
	printf ("  # runs  = %d\n", max_tests);
	printf ("\n");

	/*
	printf ("  P(dis) %% Deviation = %2d%% %% Coverage = %2d%%\n", dis_dev, dis_cov);
	printf ("  P(def) %% Deviation = %2d%% %% Coverage = %2d%%\n", def_dev, def_cov);
	printf ("  P(den) %% Deviation = %2d%% %% Coverage = %2d%%\n", den_dev, den_cov);
	printf ("  P(dim) %% Deviation = %2d%% %% Coverage = %2d%%\n", dim_dev, dim_cov);
	printf ("  P(dam) %% Deviation = %2d%% %% Coverage = %2d%%\n", dam_dev, dam_cov);
	printf ("\n");
	*/

	/* Open output file */
	ofp = fopen (argv[2], "w+");

	printf ("\n");
	if (dis_cov != 0) fprintf (ofp,  "P(dis), Runs = %d, Coverage = %d%%\n", max_tests, dis_cov);
	if (def_cov != 0) fprintf (ofp,  "P(def), Runs = %d, Coverage = %d%%\n", max_tests, def_cov);
	if (den_cov != 0) fprintf (ofp,  "P(den), Runs = %d, Coverage = %d%%\n", max_tests, den_cov);
	if (dim_cov != 0) fprintf (ofp,  "P(dim), Runs = %d, Coverage = %d%%\n", max_tests, dim_cov);
	if (dam_cov != 0) fprintf (ofp, "%%(dam), Runs = %d, Coverage = %d%%\n", max_tests, dam_cov);

	/* Tally points from input file */
	/*
	printf ("\n");
	printf ("  Tally points...\n");
	*/
	/* Initialize tally */
	tally[ROW_ASSET][COL_ID]    [IDX_DATA] = 0;		/* Number of Asset Records */
	tally[ROW_DPDIS][IDX_POINTS][IDX_DATA] = 0;		/* Number of P(dis) Points */
	tally[ROW_DPDEF][IDX_POINTS][IDX_DATA] = 0;		/* Number of P(def) Points */
	tally[ROW_DPDEN][IDX_POINTS][IDX_DATA] = 0;		/* Number of P(den) Points */
	tally[ROW_DPDIM][IDX_POINTS][IDX_DATA] = 0;		/* Number of P(dim) Points */
	tally[ROW_ASSET][COL_PDAM]  [IDX_DATA] = 0;		/* Number of P(dam) Points = # Records */

	/* Open input file */
	ifp = fopen (argv[1], "r+");
	while (getnext (ifp, rec))
	{
		tallypts (rec, tally);
	}
	fclose (ifp);

	/* Copy values to shorthand variables */
	num_assets = tally[ROW_ASSET][COL_ID]    [IDX_DATA];

	printf ("  # assets    : %d \n", tally[ROW_ASSET][COL_ID]    [IDX_DATA]);
	printf ("  # P(dis) pts: %d \n", tally[ROW_DPDIS][IDX_POINTS][IDX_DATA]);
	printf ("  # P(def) pts: %d \n", tally[ROW_DPDEF][IDX_POINTS][IDX_DATA]);
	printf ("  # P(den) pts: %d \n", tally[ROW_DPDEN][IDX_POINTS][IDX_DATA]);
	printf ("  # P(dim) pts: %d \n", tally[ROW_DPDIM][IDX_POINTS][IDX_DATA]);
	printf ("  # P(dam) pts: %d \n", tally[ROW_ASSET][COL_PDAM]  [IDX_DATA]);
	printf ("\n");

	dis_cumnp = 0;
	def_cumnp = 0;
	den_cumnp = 0;
	dim_cumnp = 0;
	dam_cumnp = 0;

	dis_cumsum = 0;
	def_cumsum = 0;
	den_cumsum = 0;
	dim_cumsum = 0;
	dam_cumsum = 0;
	
	/* Seed random number generator */
	srand ( (unsigned int)time ( NULL ) );

	for (step = 0; step < MAX_STEPS+1; step++)
	{
		/* Initialize deviation values */
		dis_dev = step * STEP_INCR * 100;
		def_dev = step * STEP_INCR * 100;
		den_dev = step * STEP_INCR * 100;
		dim_dev = step * STEP_INCR * 100;
		dam_dev = step * STEP_INCR * 100;

		printf ("\n");
		printf ("  Step #: %d...\n", step);

		/*
		printf ("  P(dis) %% Deviation = %2d%% %% Coverage = %2d%%\n", dis_dev, dis_cov);
		printf ("  P(def) %% Deviation = %2d%% %% Coverage = %2d%%\n", def_dev, def_cov);
		printf ("  P(den) %% Deviation = %2d%% %% Coverage = %2d%%\n", den_dev, den_cov);
		printf ("  P(dim) %% Deviation = %2d%% %% Coverage = %2d%%\n", dim_dev, dim_cov);
		printf ("  P(dam) %% Deviation = %2d%% %% Coverage = %2d%%\n", dam_dev, dam_cov);
		printf ("\n");
		*/
		cum_dtheta = 0;
		cum_theta  = 0;

		for (tidx = 0; tidx < max_tests; tidx++)
		{
			
			/* Initialize variables that keep track of how many changes have been made */
			dis_np = 0;
			def_np = 0;
			den_np = 0;
			dim_np = 0;
			dam_np = 0;

			dis_sum = 0;
			def_sum = 0;
			den_sum = 0;
			dim_sum = 0;
			dam_sum = 0;

			dis_mod = 100 - dis_cov;
			def_mod = 100 - def_cov;
			den_mod = 100 - den_cov;
			dim_mod = 100 - dim_cov;
			dam_mod = 100 - dam_cov;

			dis_count = rand() / (RAND_MAX / (dis_mod + 1) + 1);
			def_count = rand() / (RAND_MAX / (def_mod + 1) + 1);
			den_count = rand() / (RAND_MAX / (den_mod + 1) + 1);
			dim_count = rand() / (RAND_MAX / (dim_mod + 1) + 1);
			dam_count = rand() / (RAND_MAX / (dam_mod + 1) + 1);
			/*
			printf ("  dis_count = %d\n", dis_count);
			printf ("  def_count = %d\n", def_count);
			printf ("  den_count = %d\n", den_count);
			printf ("  dim_count = %d\n", dim_count);
			printf ("  dam_count = %d\n", dam_count);
			printf ("\n");
			*/
			/* Initialize dtheta accumulation variable */
			sum_dtheta = 0;
			sum_theta  = 0;

			/* Open input file */
		    ifp = fopen (argv[1], "r+");

/****************************************************************************************/
/* Test22.e Begin                                                                       */
/****************************************************************************************/
			/*
			printf (" Process Asset Records...\n");
			*/
			/* Process each asset record */
			while (getnext (ifp, rec))
			{
				/* Find optimum theta/cost combinations for each asset */
				dis_pts = rec [ROW_DPDIS][IDX_POINTS][IDX_DATA];
				def_pts = rec [ROW_DPDEF][IDX_POINTS][IDX_DATA];
				den_pts = rec [ROW_DPDEN][IDX_POINTS][IDX_DATA];
				dim_pts = rec [ROW_DPDIM][IDX_POINTS][IDX_DATA];

				/* P(dis) */
				dis_tsave = 0;
				dis_csave = 0;
				dis_prop  = 0;
				for (disx = 0; disx < dis_pts; disx++)
				{
					dis_dtheta = rec[ROW_DPDIS][disx+1][IDX_DTHETA];
					dis_cost   = rec[ROW_DPDIS][disx+1][IDX_COST];

					if (dis_dtheta / dis_cost > dis_prop)
					{
						dis_prop  = dis_dtheta / dis_cost;
						dis_tsave = dis_dtheta;
						dis_csave = dis_cost;
					}
				}

				/* P(def) */
				def_tsave = 0;
				def_csave = 0;
				def_prop  = 0;
				for (defx = 0; defx < def_pts; defx++)
				{
					def_dtheta = rec[ROW_DPDEF][defx+1][IDX_DTHETA];
					def_cost   = rec[ROW_DPDEF][defx+1][IDX_COST];

					if (def_dtheta / def_cost > def_prop)
					{
						def_prop  = def_dtheta / def_cost;
						def_tsave = def_dtheta;
						def_csave = def_cost;
					}
				}

				/* P(den) */
				den_tsave = 0;
				den_csave = 0;
				den_prop  = 0;
				for (denx = 0; denx < den_pts; denx++)
				{
					den_dtheta = rec[ROW_DPDEN][denx+1][IDX_DTHETA];
					den_cost   = rec[ROW_DPDEN][denx+1][IDX_COST];

					if (den_dtheta / den_cost > den_prop)
					{
						den_prop  = den_dtheta / den_cost;
						den_tsave = den_dtheta;
						den_csave = den_cost;
					}
				}


				/* P(dim) */
				dim_tsave = 0;
				dim_csave = 0;
				dim_prop  = 0;
				for (dimx = 0; dimx < dim_pts; dimx++)
				{
					dim_dtheta = rec[ROW_DPDIM][dimx+1][IDX_DTHETA];
					dim_cost   = rec[ROW_DPDIM][dimx+1][IDX_COST];

					if (dim_dtheta / dim_cost > dim_prop)
					{
						dim_prop  = dim_dtheta / dim_cost;
						dim_tsave = dim_dtheta;
						dim_csave = dim_cost;
					}
				}

/****************************************************************************************/
/* Deviate selected dthetas. Begin														*/
/****************************************************************************************/
				dis_np++;
				def_np++;
				den_np++;
				dim_np++;
				dam_np++;
				
				if (dis_cov > 0) deviate(&dis_tsave, &dis_count, dis_cov, dis_np, &dis_sum, dis_dev);
				if (def_cov > 0) deviate(&def_tsave, &def_count, def_cov, def_np, &def_sum, def_dev);
				if (den_cov > 0) deviate(&den_tsave, &den_count, den_cov, den_np, &den_sum, den_dev);
				if (dim_cov > 0) deviate(&dim_tsave, &dim_count, dim_cov, dim_np, &dim_sum, dim_dev);
				if (dam_cov > 0) deviate (&rec[ROW_ASSET][COL_PDAM][IDX_DATA], &dam_count, dam_cov, dam_np, &dam_sum, dam_dev);

/****************************************************************************************/
/* Deviate selected dthetas. End														*/
/****************************************************************************************/
		
				/* Prepare output record */
				outrec[COL_ID]     = rec[ROW_ASSET][COL_ID]   [IDX_DATA];
				outrec[COL_TYPE]   = rec[ROW_ASSET][COL_TYPE] [IDX_DATA];
				outrec[COL_LOC]    = rec[ROW_ASSET][COL_LOC]  [IDX_DATA];
				outrec[COL_PDIS]   = rec[ROW_ASSET][COL_PDIS] [IDX_DATA] + dis_tsave;
				outrec[COL_PDEF]   = rec[ROW_ASSET][COL_PDEF] [IDX_DATA] + def_tsave;
				outrec[COL_PDEN]   = rec[ROW_ASSET][COL_PDEN] [IDX_DATA] + den_tsave;
				outrec[COL_PDIM]   = rec[ROW_ASSET][COL_PDIM] [IDX_DATA] + dim_tsave;
				outrec[COL_PDAM]   = rec[ROW_ASSET][COL_PDAM] [IDX_DATA];
				outrec[COL_THETA]  = outrec[COL_PDIS] * outrec[COL_PDEF] * outrec[COL_PDEN] * 
									 outrec[COL_PDIM] * outrec[COL_PDAM];
				/* Recalculate previous theta to avert precision errors */
				outrec[COL_DTHETA] = outrec[COL_THETA] - 
									 (rec[ROW_ASSET][COL_PDIS][IDX_DATA] *
									  rec[ROW_ASSET][COL_PDEF][IDX_DATA] *
									  rec[ROW_ASSET][COL_PDEN][IDX_DATA] *
									  rec[ROW_ASSET][COL_PDIM][IDX_DATA] *
									  rec[ROW_ASSET][COL_PDAM][IDX_DATA] );
				/* Adjust for precision errors from very small numbers */
				if ((dis_tsave + def_tsave + den_tsave + dim_tsave) == 0) 
				{
					outrec[COL_THETA]  = rec[ROW_ASSET][COL_THETA][IDX_DATA];
					outrec[COL_DTHETA] = 0; 
				}
				outrec[COL_TCOST]  = dis_csave + def_csave + den_csave + dim_csave;
				outrec[COL_DDIS]   = dis_csave;
				outrec[COL_DDEF]   = def_csave;
				outrec[COL_DDEN]   = den_csave;
				outrec[COL_DDIM]   = dim_csave;
				
/****************************************************************************************/
/* Test22.e End                                                                         */
/****************************************************************************************/

				/* Update/collect test data */
				sum_dtheta = sum_dtheta + outrec[COL_DTHETA];
				sum_theta  = sum_theta  + outrec[COL_THETA];
				/*
				printf  ("      Theta = %E Dtheta = %E\n", outrec[COL_THETA], outrec[COL_DTHETA]);
				*/
			}
	
			/* Update deviation accumulation variables */
			dis_cumnp = dis_cumnp + dis_np;
			def_cumnp = def_cumnp + def_np;
			den_cumnp = den_cumnp + den_np;
			dim_cumnp = dim_cumnp + dim_np;
			dam_cumnp = dam_cumnp + dam_np;

			dis_cumsum = dis_cumsum + dis_sum;
			def_cumsum = def_cumsum + def_sum;
			den_cumsum = den_cumsum + den_sum;
			dim_cumsum = dim_cumsum + dim_sum;
			dam_cumsum = dam_cumsum + dam_sum;

			/* Calculate average dtheta across all records and add to cummulative average */
			cum_dtheta = cum_dtheta + sum_dtheta / num_assets;
			cum_theta  = cum_theta  + sum_theta  / num_assets;
			/*
			printf ("\n");
			printf ("  Run #: %d average dtheta = %E\n", tidx+1, sum_dtheta / num_assets);
			*/
			fclose (ifp);
		}
		/* Calculate average dtheta across all runs and record */
		printf  ("      Commulative Average Theta = %E Dtheta = %E\n", cum_theta / max_tests, cum_dtheta / max_tests);
		fprintf (ofp, "%f,%E,%E\n", step*STEP_INCR, cum_theta / max_tests, cum_dtheta / max_tests);
	}
	printf ("\n");
	printf (" P(Dis) %%coverage = %3.2f #pts = %d #deviated = %d %%deviated = %3.2f\n",
				(float)dis_cov/100, dis_cumnp, dis_cumsum, (float)dis_cumsum/(float)dis_cumnp);
	printf (" P(Def) %%coverage = %3.2f #pts = %d #deviated = %d %%deviated = %3.2f\n",
				(float)def_cov/100, def_cumnp, def_cumsum, (float)def_cumsum/(float)def_cumnp);
	printf (" P(Den) %%coverage = %3.2f #pts = %d #deviated = %d %%deviated = %3.2f\n",
				(float)den_cov/100, den_cumnp, den_cumsum, (float)den_cumsum/(float)den_cumnp);
	printf (" P(Dim) %%coverage = %3.2f #pts = %d #deviated = %d %%deviated = %3.2f\n",
				(float)dim_cov/100, dim_cumnp, dim_cumsum, (float)dim_cumsum/(float)dim_cumnp);
	printf (" P(Dam) %%coverage = %3.2f #pts = %d #deviated = %d %%deviated = %3.2f\n",
				(float)dam_cov/100, dam_cumnp, dam_cumsum, (float)dam_cumsum/(float)dam_cumnp);
	printf ("\n");


	/* Append EOF to end of output file */
	fprintf (ofp, "%d\n", EOF);
	fclose (ofp);

	printf ("End Program...\n");
    return 0;
}

/* FUNCTIONS */

int   getnext (FILE *ifp, float rec[][INREC_COLS][INREC_VALUES])
{
	int		i, j, next_rec, num_pts;
	float	dtheta, cost;
      
	/*File Format
		ID Type Location P(dis) P(def) P(den) P(dim) %(dam) Theta
			#delta-P(dis) points
				dtheta	cost
				dtheta	cost
				...
			#delta-P(def) points
				dtheta	cost
				dtheta	cost
				...
			#delta-P(den) points
				dtheta	cost
				dtheta	cost
				...
			#delta-P(dim) points
				dtheta	cost
				dtheta	cost
				...
		...
		EOF
	*/

	fscanf (ifp, "%d %f %f %f %f %f %f %f %E\n", 
				&next_rec,
				&rec[ROW_ASSET][COL_TYPE] [IDX_DATA],
				&rec[ROW_ASSET][COL_LOC]  [IDX_DATA],
				&rec[ROW_ASSET][COL_PDIS] [IDX_DATA],
				&rec[ROW_ASSET][COL_PDEF] [IDX_DATA],
				&rec[ROW_ASSET][COL_PDEN] [IDX_DATA],
				&rec[ROW_ASSET][COL_PDIM] [IDX_DATA],
				&rec[ROW_ASSET][COL_PDAM] [IDX_DATA],
				&rec[ROW_ASSET][COL_THETA][IDX_DATA]);

	if (next_rec == EOF) return 0;

	rec[ROW_ASSET][COL_ID][IDX_DATA] = next_rec;
	for (i = 0; i < MAX_TABLES; i++)
		{
			fscanf (ifp, "%d\n", &num_pts);
			rec[i+1][IDX_POINTS][IDX_DATA] = num_pts;
			for (j = 0; j < num_pts; j++)
			{
				fscanf (ifp, "%f %f\n", &dtheta, &cost);
				rec[i+1][j+1][IDX_DTHETA] = dtheta;
				rec[i+1][j+1][IDX_COST]   = cost;
			}
		}


	return 1;
}

int tallypts(float rec[][INREC_COLS][INREC_VALUES], int tally[][INREC_COLS][INREC_VALUES])
{
	tally[ROW_ASSET][COL_ID]    [IDX_DATA]++;
	tally[ROW_DPDIS][IDX_POINTS][IDX_DATA] = tally[ROW_DPDIS][IDX_POINTS][IDX_DATA] + rec[ROW_DPDIS][IDX_POINTS][IDX_DATA];
	tally[ROW_DPDEF][IDX_POINTS][IDX_DATA] = tally[ROW_DPDEF][IDX_POINTS][IDX_DATA] + rec[ROW_DPDEF][IDX_POINTS][IDX_DATA];
	tally[ROW_DPDEN][IDX_POINTS][IDX_DATA] = tally[ROW_DPDEN][IDX_POINTS][IDX_DATA] + rec[ROW_DPDEN][IDX_POINTS][IDX_DATA];
	tally[ROW_DPDIM][IDX_POINTS][IDX_DATA] = tally[ROW_DPDIM][IDX_POINTS][IDX_DATA] + rec[ROW_DPDIM][IDX_POINTS][IDX_DATA];
	tally[ROW_ASSET][COL_PDAM]  [IDX_DATA]++;
	
	return 0;
}

int select	(int pct)
{
	int		ran;

	/* Generate random number between 1 and 100 */
	ran = 1 + rand() / (RAND_MAX / 100 +1);
	if (ran <= pct) return 1;					/* Yes, Select      */
	return 0;									/* No, Don't Select */
}

int deviate (float *dtheta, int *dcount, int dcov, int dpts, int *dsum, int dev)
{
	int		tcount, tsum;
	float	ttheta, fsum, fpts, fdev;
	/*
	printf ("   *dtheta %f *dcount %d dcov %d dpts %d *dsum %d dev %d\n",
				*dtheta, *dcount, dcov, dpts, *dsum, dev);
	*/
	tcount = *dcount;
	tsum   = *dsum;
	ttheta = *dtheta;
	fsum   = tsum;
	fpts   = dpts;
	fdev   = dev;

	tcount++;
	if (tcount <= dcov)
	{
		tsum++;
		/* deviate dtheta */
		ttheta = ttheta * (fdev/100 +1);
		/* Reset theta if it exceeds 1.0 */
		if (ttheta > 1) ttheta = 1;
	}
	if (tcount == 100) tcount = 0;

	/*
	printf ("    %d: Before %f After %f Deviation %1.5f\n", 
				tsum, *dtheta, ttheta, (fdev/100+1));
	*/

	*dtheta = ttheta;
	*dcount = tcount;
	*dsum   = tsum;

	return 0;
}

int	prtrec  (FILE *ofp, float rec[][INREC_COLS][INREC_VALUES])
{
	int		i, j, num_pts;

	printf ("  %2.0f %2.0f %2.0f %f %f %f %f %f %E\n", 
		rec[ROW_ASSET][COL_ID]	 [IDX_DATA],
		rec[ROW_ASSET][COL_TYPE] [IDX_DATA],
		rec[ROW_ASSET][COL_LOC]  [IDX_DATA],
		rec[ROW_ASSET][COL_PDIS] [IDX_DATA],
		rec[ROW_ASSET][COL_PDEF] [IDX_DATA],
		rec[ROW_ASSET][COL_PDEN] [IDX_DATA],
		rec[ROW_ASSET][COL_PDIM] [IDX_DATA],
		rec[ROW_ASSET][COL_PDAM] [IDX_DATA],
		rec[ROW_ASSET][COL_THETA][IDX_DATA]);

	/*
	for (i = 0; i < MAX_TABLES; i++)
		{
			num_pts = rec[i+1][IDX_POINTS][IDX_DATA];
			printf ("    %d\n", num_pts);
			for (j = 0; j < num_pts; j++)
			{
				printf ("    %2d: %f %f\n", j+1, 
									   rec[i+1][j+1][IDX_DTHETA], 
									   rec[i+1][j+1][IDX_COST]);
			}
		}
	*/
	return 0;
}

int prtout	(FILE *ofp, float outrec[])
{

	/* Display to monitor */
	printf ("  %2.0f %2.0f %2.0f %E %E %f %5.0f\n", 
		outrec[COL_ID], outrec[COL_TYPE], outrec[COL_LOC],
		outrec[COL_THETA], outrec[COL_DTHETA], outrec[COL_TCOST], outrec[COL_RUN]);

	/* Print to output file in CDF format */
	fprintf (ofp, "%4.0f,%2.0f,%2.0f,%f,%f,%f,%f,%f,%E,%E,%f,%f,%f,%f,%f,%5.0f\n", 
		outrec[COL_ID], outrec[COL_TYPE], outrec[COL_LOC],
		outrec[COL_PDIS], outrec[COL_PDEF], outrec[COL_PDEN], outrec[COL_PDIM], outrec[COL_PDAM],
		outrec[COL_THETA], outrec[COL_DTHETA], outrec[COL_TCOST],
		outrec[COL_DDIS], outrec[COL_DDEF], outrec[COL_DDEN], outrec[COL_DDIM], outrec[COL_RUN]);


	return 0;
}