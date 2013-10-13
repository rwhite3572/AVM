/* File: Test20c.c
   Generate x number of asset records and associated theta values.
		ID		= #
		Type	= 1 <= n <= 13: 1-4 = CBRN; 5-13 = CI
		Location= 1 <= n <= 50
		P(dis)	= 0.001 <= n <= 0.01
		P(def)	= 0.001 <= n <= 0.70
		P(den)	= 0.001 <= n <= 0.10
		P(dim)	= 0.001 <= n <= 0.50
		%(dam)	= 0.001 <= n <= 0.01
		theta   = P(dis) * P(def) * P(den) * P(dim) * %(dam)
   
	Store assets to output file in Comma Delimited File (CDF) format for Excel
	Execute:  Test20c # assets.txt

	Modification from Test20.c:  P(dis) and P(dim) are tied to asset type and location respectively.
*/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* CONSTANT DEFINITIONS */
#define MAX_TYPE	13
#define MIN_TYPE	1
#define MAX_LOC		50
#define MIN_LOC		1
#define MAX_DIS		0.01
#define MAX_DEF		0.70
#define MAX_DEN		0.10
#define MAX_DIM		0.50
#define MAX_DAM		0.01
#define MIN_VAL		0.001
#define NUM_VALS	9




/* MAIN PROGRAM */
int main(int argc, char *argv[]) 
{

/* DATA DECLARATIONS */
	FILE *ofp;											/* Output File Pointer
														*/
	float asset[NUM_VALS];								/* One asset = 8 sets of values
														*/
	float type_pdis[MAX_TYPE+1];						/* P(dis) tied to asset type (indexed 1-13).
														*/
	float loc_pdim[MAX_LOC+1];							/* P(dim) tied to asset location (indexed 1-50).
														*/
	int max_assets, asset_num, type_idx, loc_idx;		/* Asset variables
														*/
	int asset_type, asset_loc;
	float p_dis, p_def, p_den, p_dim, p_dam, theta;

/* DATA PROCESSING */
	printf ("Start Program...\n");
    /* Get # of tables. */
    if (argc == 1)
    {
        printf ("  Number of assets not specified.\n");
        return -1;
    }
    
	/* Get output file name. */
	if (argc == 2)
	{
		printf ("  No output file specified.\n");
	    return -1;
	}
	ofp = fopen (argv[2], "w");

	/* Convert # of tables from string to integer */
	max_assets = atoi(argv[1]);

	printf ("  # assets = %5d\n", max_assets);
	printf ("  outfile  = %s\n", argv[2]);

	/* Seed random number generator */
    srand ( (unsigned int)time ( NULL ) );
    
	/* Generate P(dis) for each asset type */
	for (type_idx = 0; type_idx < MAX_TYPE; type_idx++)
	{
		type_pdis[type_idx+1] = MIN_VAL + (float)rand()/((float)RAND_MAX/(MAX_DIS - MIN_VAL));
	}

	/* Generate P(dim) for each asset location */
	for (loc_idx = 0; loc_idx < MAX_LOC; loc_idx++)
	{
		loc_pdim[loc_idx+1] = MIN_VAL + (float)rand()/((float)RAND_MAX/(MAX_DIM - MIN_VAL));
	}

	/* Generate asset records */
	for (asset_num = 0; asset_num < max_assets; asset_num++)
	{
		printf ("  Generating Asset #%5d\n", asset_num+1);

		/* Generate asset type between MIN_TYPE and MAX_TYPE*/
		asset_type = MIN_TYPE + rand() / (RAND_MAX / (MAX_TYPE - MIN_TYPE +1) +1);
		printf ("    Type     = %2d\n", asset_type);

		/* Generate asset location between MIN_LOC and MAX_LOC*/
		asset_loc  = MIN_LOC + rand() / ( RAND_MAX / ( MAX_LOC - MIN_LOC +1) + 1);
		printf ("    Location = %2d\n", asset_loc);

		/* Assign asset P(dis) based on asset type*/
		p_dis = type_pdis[asset_type];
		printf ("    P(dis)   = %1.4f\n", p_dis);
	
		/* Generate asset P(def) between MIN_VAL and MAX_DEF*/
		p_def = MIN_VAL + (float)rand()/((float)RAND_MAX/(MAX_DEF - MIN_VAL));
		printf ("    P(def)   = %1.4f\n", p_def);
	
		/* Generate asset P(den) between MIN_VAL and MAX_DEN*/
		p_den = MIN_VAL + (float)rand()/((float)RAND_MAX/(MAX_DEN - MIN_VAL));
		printf ("    P(den)   = %1.4f\n", p_den);
	
		/* Assign asset P(dim) based on asset location*/
		p_dim = loc_pdim[asset_loc];
		printf ("    P(dim)   = %1.4f\n", p_dim);
	
		/* Generate asset %(dam) between MIN_VAL and MAX_DAM*/
		p_dam = MIN_VAL + (float)rand()/((float)RAND_MAX/(MAX_DAM - MIN_VAL));
		printf ("    %%(dam)   = %1.4f\n", p_dam);

		/* Calculate Theta = P(dis) * P(def) * P(den) * P(dim) * %(dam)*/
		theta = p_dis * p_def * p_den * p_dim * p_dam;
		printf ("    Theta    = %E\n",    theta);

		/*Print File Output in CDF Format:
		0      1    2        3      4      5      6      7      8
		asset#,type,location,P(dis),P(def),P(den),P(dim),%(dam),theta
		*/

		fprintf (ofp, "%d,%d,%2d,%f,%f,%f,%f,%f,%E\n", 
		           asset_num+1, asset_type, asset_loc,
				   p_dis, p_def, p_den, p_dim, p_dam, theta);
	}
	/* Append EOF to end of output file */
	fprintf (ofp, "%d\n", EOF);

	fclose (ofp);

    return 0;
}