/* File: Test22e.c
   Read in asset file with baseline theta and estimated delta-theta & cost values.
   Compute optimum delta-theta/cost combinations.
   Save baseline asset data and optimum delta-theta/cost values to output file
   Output in Comma Delimited File format

	Modification from Test 22c.c	1. Account for tables with no data
									2. Find optimum theta within each phase; 
									i.e., p(dis), p(def), p(den), p(dim)
	Modification from Test22d.c		Delta theta values are relative, not absolute.

	Execute:  Test22e dtheta.dat otheta.txt
*/
#include <stdio.h>
#include <math.h>

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

/* FUNCTION HEADERS */
int getnext (FILE *ifp, float rec[][INREC_COLS][INREC_VALUES]);
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
	float	theta_prop, theta_cost, theta_max;

	float	outrec[OUTREC_COLS];						/* Output Record Data
														      <=========== same as input record format ===========>
															  0  1    2        3      4      5      6      7      8		9	   10	 11	    12	   13	  14	 15
															  ID Type Location P(dis) P(def) P(den) P(dim) %(dam) Theta DTheta TCost D(dis) D(def) D(den) D(dim) Run#
														*/

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

	/* Open input and output files */
    ifp = fopen (argv[1], "r+");
	ofp = fopen (argv[2], "w+");

	printf (" Process Asset Records...\n");

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

		prtout (ofp, outrec);
	}

	fclose (ifp);

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

int prtout	(FILE *ofp, float outrec[])
{

	/* Display to monitor */
	printf ("  %2.0f %2.0f %2.0f %E %E %f\n", 
		outrec[COL_ID], outrec[COL_TYPE], outrec[COL_LOC],
		outrec[COL_THETA], outrec[COL_DTHETA], outrec[COL_TCOST]);

	/* Print to output file in CDF format */
	fprintf (ofp, "%4.0f,%2.0f,%2.0f,%f,%f,%f,%f,%f,%E,%E,%f,%f,%f,%f,%f\n", 
		outrec[COL_ID], outrec[COL_TYPE], outrec[COL_LOC],
		outrec[COL_PDIS], outrec[COL_PDEF], outrec[COL_PDEN], outrec[COL_PDIM], outrec[COL_PDAM],
		outrec[COL_THETA], outrec[COL_DTHETA], outrec[COL_TCOST],
		outrec[COL_DDIS], outrec[COL_DDEF], outrec[COL_DDEN], outrec[COL_DDIM]);


	return 0;
}