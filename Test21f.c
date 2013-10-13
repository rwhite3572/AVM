/*	File: Test21f.c
	Take input asset file.
	Add tables for each delta-theta / delta-cost improvement.
	Save to output 
	Execute:  test21b infile outfile

	Modification from Test21.c:  P(dis) and P(dim) are tied to asset type and location respectively.
	Modification from Test21b.c:  1. dtheta and cost tuples are independent of each other.
								  2. some assets have no planned improvements; i.e., no tables added.
								  3. dtheta is relative to current theta; i.e., current theta < dtheta < max theta.
	Modification from Test21c.c   P(dis) and P(dim) must be the same across asset types and locations, including empty tables.
	Modification from Test21d.c	  Compute fractional cost values for asset type and location.
	Modification from Test21e.c   Change method for selecting which tables are empty.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* CONSTANT DEFINITIONS */
#define ASSET_VALUES	9
#define	IDX_NUM			0
#define IDX_TYPE		1
#define IDX_LOC			2
#define IDX_PDIS		3
#define IDX_PDEF		4
#define IDX_PDEN		5
#define IDX_PDIM		6
#define IDX_PDAM		7
#define IDX_THETA		8
#define MAX_TYPE		13
#define MIN_TYPE		1
#define MAX_LOC			50
#define MIN_LOC			1
#define MAX_DIS			0.01
#define MAX_DEF			0.70
#define MAX_DEN			0.10
#define MAX_DIM			0.50
#define MAX_DAM			0.01		/* No change. Not computed. %(dam) is constant based on asset. */
#define MIN_VAL			0.001		/* No change. Not computed. theta < dtheta < max theta.        */

#define MAX_COST		10000
#define MIN_COST		10
#define	MAX_TABLES		4
#define TABLE_VALUES	2
#define MAX_POINTS		10
#define MIN_POINTS		0
#define IDX_POINTS		0
#define IDX_DTHETA		0
#define	IDX_COST		1

/* FUNCTION HEADERS */
int	getnextrec	(FILE *ifp, float asset[]);
int gentable	(float table[][TABLE_VALUES], float lo, float hi);
int pass		(int pct);
int prtasset	(FILE *ofp, float asset[]);
int prttable	(FILE *ofp, float table[][TABLE_VALUES]);
int cpytable	(float from[][TABLE_VALUES], float to[][TABLE_VALUES]);

/* MAIN PROGRAM */
int main(int argc, char *argv[]) 
{

/* DATA DECLARATIONS */
	/*Input File Format
	0      1    2        3      4      5      6      7      8
	asset#,type,location,P(dis),P(def),P(den),P(dim),%(dam),theta
	*/
	FILE	*ifp;

	/*Output File Format
	0      1    2        3      4      5      6      7      8
	asset# type location P(dis) P(def) P(den) P(dim) %(dam) theta
	pre#
	dtheta1	cost1
	dtheta2	cost2
	...
	dtheta10 cost10
	pro#
	dtheta1	cost1
	dtheta2	cost2
	...
	dtheta10 cost10
	mit#
	dtheta1	cost1
	dtheta2	cost2
	...
	dtheta10 cost10
	res#
	dtheta1	cost1
	dtheta2	cost2
	...
	dtheta10 cost10
	asset# type location P(dis) P(def) P(den) P(dim) %(dam) theta
	...
	*/
	FILE	*ofp;

	/*Asset record format
	0      1    2        3      4      5      6      7      8
	asset#,type,location,P(dis),P(def),P(den),P(dim),%(dam),theta
	*/
	float	asset [ASSET_VALUES];

	/*Table record format
	pt_idx	IDX_DTHETA	IDX_COST
	0		num_pts
	1		dtheta1		cost1
	2		dtheta2		cost2
	3		dtheta3		cost3
	...
	10		dtheta10	cost10
	*/
	float	table [MAX_POINTS+1][TABLE_VALUES],
			empty_table [MAX_POINTS+1][TABLE_VALUES];
	float	lo, hi; 
	int		table_idx;
	float	dpdis [MAX_TYPE+1][MAX_POINTS+1][TABLE_VALUES], 
			dpdim [MAX_LOC+1] [MAX_POINTS+1][TABLE_VALUES];
	int		ref_idx, type_idx, loc_idx;
	int		no_asset, num_assets, skip_asset, gen_asset,
			no_type, skip_type, gen_type,
			no_loc, skip_loc, gen_loc,
			type_count, loc_count, tmax, lmax;
	int		tallyt [MAX_TYPE+1], tallyl [MAX_LOC+1];

/* DATA PROCESSING */
	printf ("Start Program...\n");

	/* Seed random number generator */
    srand ( (unsigned int)time ( NULL ) );
    
	/* Verify input file specified. */
    if (argc == 1)
    {
        printf ("  No input file specified.\n");
        return -1;
    }
    
	/* Verify output file specified. */
    if (argc == 2)
    {
        printf ("  No output file specified.\n");
        return -1;
    }

	/* Open input and output files */
    ifp = fopen (argv[1], "r+");
	ofp = fopen (argv[2], "w+");

	/* Initialize delta P[dis] to indicate no stored values */
	for (ref_idx = 0; ref_idx < MAX_TYPE+1; ref_idx++)
	{
		dpdis[ref_idx][IDX_POINTS][IDX_POINTS] = -1;
		tallyt[ref_idx] = 0;
	}
	/* Initialize delta P[dim] to indicate no stored values */
	for (ref_idx = 0; ref_idx < MAX_LOC+1; ref_idx++)
	{
		dpdim[ref_idx][IDX_POINTS][IDX_POINTS] = -1;
		tallyl[ref_idx] = 0;
	}
	num_assets = 0;

	/* Tally number of assets, types, and locations */
		/* Obtain first asset */
	getnextrec (ifp, asset);

	while (asset[IDX_NUM] != EOF)
	{
		num_assets++;
		type_idx = asset[IDX_TYPE];
		tallyt[type_idx]++;
		loc_idx = asset[IDX_LOC];
		tallyl[loc_idx]++;
		getnextrec (ifp, asset);
	}
	fclose (ifp);
	/*
	printf ("\n");
	printf ("  Tally...\n");
	printf ("    num_assets = %d\n", num_assets);
	for (ref_idx = 0; ref_idx < MAX_TYPE; ref_idx++)
		printf ("    %d: %d\n", ref_idx+1, tallyt[ref_idx+1]);
	for (ref_idx = 0; ref_idx < MAX_LOC; ref_idx++)
		printf ("    %d: %d\n", ref_idx+1, tallyl[ref_idx+1]);
	*/
	/* Query what percent of assets should have no data at all */
	printf ("\n  What %% assets should have no data generated? ");
	scanf  ("%d", &no_asset);
	/* Query what percent of assets types should have no data at all */
	printf ("\n  What %% of asset types should have no data generated? ");
	scanf  ("%d", &no_type);
	/* Query what percent of assets locations should have no data at all */
	printf ("\n  What %% of asset locations should have no data generated? ");
	scanf  ("%d", &no_loc);
	/* Initialize related variables                            */
	empty_table [IDX_POINTS][IDX_POINTS] = 0;
	skip_asset = 0;
	skip_type  = 0;
	skip_loc   = 0;
	gen_asset  = 0;
	gen_type   = 0;
	gen_loc    = 0;
	type_count = 0;
	loc_count  = 0;
	tmax       = 0;
	lmax       = 0;

	/*
	printf ("\n  How many assets in the input file? ");
	scanf  ("%d", &num_assets);
	printf ("\n");
	*/

	ifp = fopen (argv[1], "r+");
	/* Obtain first asset */
	getnextrec (ifp, asset);

	while (asset[IDX_NUM] != EOF)
	{
		prtasset (ofp, asset);
		
		/* See if this asset type already has P(dis) generated data.  If not, generate and save it. */
		type_idx = asset[IDX_TYPE];
		if (type_idx > tmax) tmax = type_idx;
		if (dpdis[type_idx][IDX_POINTS][IDX_POINTS] < 0)
		{
			type_count++;
			if (pass(no_type)) 
			{
				skip_type++;
				cpytable (empty_table, dpdis[type_idx]);
			}
			else
			{
				gen_type++;
				gentable (table, asset[IDX_PDIS], MAX_DIS);
				/* Make costs fractional.  Divide cost by number of this type */
				/*
				printf ("    Convert fractonal costs for Type...\n");
				*/
				for (ref_idx = 0; ref_idx < table [IDX_POINTS][IDX_POINTS]; ref_idx++)
				{
					/*
					printf ("    %d: Before = %f ", ref_idx+1, table[ref_idx+1][IDX_COST]);
					*/
					table[ref_idx+1][IDX_COST] = table[ref_idx+1][IDX_COST] / tallyt[type_idx];
					/*
					printf ("    After = %f\n", table[ref_idx+1][IDX_COST]);
					*/
				}
				cpytable (table, dpdis[type_idx]);
			}
		}

		/* See if this asset location already has P(dim) generated data.  If not, generate and save it. */
		loc_idx = asset[IDX_LOC];
		if (loc_idx > lmax) lmax = loc_idx;
		if (dpdim[loc_idx][IDX_POINTS][IDX_POINTS] < 0)
		{
			loc_count++;
			if (pass(no_loc)) 
			{
				skip_loc++;
				cpytable (empty_table, dpdim[loc_idx]);
			}
			else
			{
				gen_loc++;
				gentable (table, asset[IDX_PDIM], MAX_DIM);
				/* Make costs fractional.  Divide cost by number of this location */
				/*
				printf ("    Convert fractonal costs for Type...\n");
				*/
				for (ref_idx = 0; ref_idx < table [IDX_POINTS][IDX_POINTS]; ref_idx++)
				{
					/*
					printf ("    %d: Before = %f ", ref_idx+1, table[ref_idx+1][IDX_COST]);
					*/
					table[ref_idx+1][IDX_COST] = table[ref_idx+1][IDX_COST] / tallyl[loc_idx];
					/*
					printf ("    After = %f\n", table[ref_idx+1][IDX_COST]);
					*/
				}
				cpytable (table, dpdim[loc_idx]);
			}
		}

		/* Skip data generation for this asset? */
		if (pass (no_asset))
		{
			skip_asset++;
			/* Yes, skip this record.  Print empty tables to output file, except for P(dis) and P(dim).
			    P(dis) and P(dim) values tied to asset type and location, not asset.                    */
			prttable (ofp, dpdis[type_idx]);	/* P(dis) */
			prttable (ofp, empty_table);		/* P(def) */
			prttable (ofp, empty_table);		/* P(den) */
			prttable (ofp, dpdim[loc_idx]);		/* P(dim) */
		}
		else
		{
			gen_asset++;
			/* Generate data for each of the four tables (some may be empty) */
			/* P(dis) data already generated for this type.  Print out.      */
			prttable (ofp, dpdis[type_idx]);

			/* P(def) */
			gentable (table, asset[IDX_PDEF], MAX_DEF);
			prttable (ofp, table);

			/* P(den) */
			gentable (table, asset[IDX_PDEN], MAX_DEN);
			prttable (ofp, table);

			/* P(dim) data already generated for this location.  Print out.   */
			prttable (ofp, dpdim[loc_idx]);
		}
		
		/* Get next asset */
		getnextrec (ifp, asset);
	}

	fclose (ifp);
	
	/* Append EOF to end of output file */
	fprintf (ofp, "%d\n", EOF);
	fclose (ofp);

	printf ("\n");
	printf (" skip_asset = %5d num_assets = %5d %%skipped = %2.2f gen_asset = %5d\n", 
				skip_asset, num_assets, (float)skip_asset/(float)num_assets*100, gen_asset);
	printf (" skip_type  = %5d MAX_TYPE   = %5d %%skipped = %2.2f gen_type  = %5d\n", 
				skip_type, MAX_TYPE, (float)skip_type/(float)MAX_TYPE*100, gen_type);
	printf (" skip_loc   = %5d MAX_LOC    = %5d %%skipped = %2.2f gen_loc   = %5d\n", 
				skip_loc, MAX_LOC, (float)skip_loc/(float)MAX_LOC*100, gen_loc);
	printf (" type_count = %d loc_count = %d\n", type_count, loc_count);
	printf (" tmax       = %d lmax      = %d\n", tmax, lmax);
	printf ("\n");

	printf ("End Program.\n");
	return 0;
}
/* FUNCTIONS */

int	getnextrec	(FILE *ifp, float asset[])
{
	
	char  c1, c2, c3, c4, c5, c6, c7, c8;

	fscanf (ifp, "%f %c %f %c %f %c %f %c %f %c %f %c %f %c %f %c %E\n",
				 &asset[IDX_NUM], &c1, &asset[IDX_TYPE], &c2, &asset[IDX_LOC], &c3,
				 &asset[IDX_PDIS], &c4, &asset[IDX_PDEF], &c5, &asset[IDX_PDEN], &c6, 
				 &asset[IDX_PDIM], &c7, &asset[IDX_PDAM], &c8, &asset[IDX_THETA]);
	
	return 0;
}

int gentable	(float table[][TABLE_VALUES], float lo_theta, float hi_theta)
{
	int		idx, max_points;
	float	dtheta, dcost;

	/* Randomly pick a maximum number of points to be generated between MIN_POINTS and MAX_POINTS */
	max_points = MIN_POINTS + rand() / (RAND_MAX / (MAX_POINTS - MIN_POINTS +1) +1);
	/*
	printf ("    Generate table:  lo = %1.4f hi = %1.4f max pts = %2d\n", lo_theta, hi_theta, max_points);
	*/
	/* Generate dtheta and cost tuples for each point */
	for (idx = 0; idx < max_points; idx++)
	{
		/* Generate random dtheta between current value and maximum allowable */
		dtheta = lo_theta + (float)rand()/((float)RAND_MAX/(hi_theta - lo_theta));
		/* Convert dtheta to an incremental relative value */
		dtheta = dtheta - lo_theta;
		/* Generate random cost between minimum and maximum cost values */
		dcost = MIN_COST + rand() / (RAND_MAX / (MAX_COST - MIN_COST +1) +1);
		/*
		printf ("      pt #%2d: %1.4f %1.4f\n", idx+1, dtheta, dcost);
		*/
		/* Store values in table record */
		table [idx+1][IDX_DTHETA] = dtheta;
		table [idx+1][IDX_COST]   = dcost;

	}
	/* Store number of points actually generated */
	table [IDX_POINTS][IDX_POINTS] = max_points; 

	return 0;
}

int pass		(int pct)
{
	int		ran;

	/* Generate random number between 1 and 100 */
	ran = 1 + rand() / (RAND_MAX / 100 +1);
	if (ran <= pct) return 1;					/* Yes, Pass      */
	return 0;									/* No, Don't Pass */
}

int prtasset	(FILE *ofp, float asset[])
{
	/* Print asset record to display */
	printf ("  %3.0f %2.0f %2.0f %1.4f %1.4f %1.4f %1.4f %1.4f %E\n",
			   asset[IDX_NUM], asset[IDX_TYPE], asset [IDX_LOC],
			   asset[IDX_PDIS], asset[IDX_PDEF], asset[IDX_PDEN],
			   asset[IDX_PDIM], asset[IDX_PDAM], asset[IDX_THETA]);
	/* Print asset record to output file */
	fprintf (ofp, "%3.0f %2.0f %2.0f %1.4f %1.4f %1.4f %1.4f %1.4f %E\n",
			   asset[IDX_NUM], asset[IDX_TYPE], asset [IDX_LOC],
			   asset[IDX_PDIS], asset[IDX_PDEF], asset[IDX_PDEN],
			   asset[IDX_PDIM], asset[IDX_PDAM], asset[IDX_THETA]);
	return 0;
}

int prttable	(FILE *ofp, float table[][TABLE_VALUES])
{
	int		num_pts, pt_idx;
	float	dtheta, cost;

	printf ("    Print table... \n");
	num_pts = table [IDX_POINTS][IDX_POINTS];

	/* Print # of points generated to output file*/
	fprintf (ofp, "%2d\n", num_pts);

	for (pt_idx = 0; pt_idx < num_pts; pt_idx++)
	{
		dtheta = table[pt_idx+1][IDX_DTHETA];
		cost   = table[pt_idx+1][IDX_COST];
		/* Print dtheta & cost values to display */
		printf ("      pt #%2d: %1.4f %1.4f\n", pt_idx+1, dtheta, cost);
		/* Print dtheta & cost values to output file */
		fprintf (ofp, "%f %f\n", dtheta, cost);
	}

	return 0;
}

int cpytable	(float from[][TABLE_VALUES], float to[][TABLE_VALUES])
{
	int		num_pts, pt_idx;
	float	dtheta, cost;

	/*
	printf ("    Copy table... \n");
	*/
	num_pts = from [IDX_POINTS][IDX_POINTS];

	to[IDX_POINTS][IDX_POINTS] = num_pts;
	for (pt_idx = 0; pt_idx < num_pts; pt_idx++)
	{
		dtheta = from[pt_idx+1][IDX_DTHETA];
		cost   = from[pt_idx+1][IDX_COST];

		to[pt_idx+1][IDX_DTHETA] = dtheta;
		to[pt_idx+1][IDX_COST]   = cost;
		/*
		printf ("      pt #%2d: %1.4f %1.4f\n", pt_idx+1, dtheta, cost);
		*/
	}

	return 0;
}