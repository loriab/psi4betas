/** Standard library includes */
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <fstream>
#include <string> 
#include <iomanip>
#include <vector> 


/** Required PSI4 includes */
#include <psifiles.h>
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>
#include <libchkpt/chkpt.h>
#include <libpsio/psio.hpp>
#include <libchkpt/chkpt.hpp>
#include <libiwl/iwl.h>
#include <libqt/qt.h>
#include <libtrans/mospace.h>
#include <libtrans/integraltransform.h>
#include <libdiis/diismanager.h>


/** Required libmints includes */
#include <libmints/mints.h>
#include <libmints/factory.h>
#include <libmints/wavefunction.h>

#include "omp2wave.h"
#include "defines.h"
#include "arrays.h"

using namespace boost;
using namespace psi;


namespace psi{ namespace omp2wave{
  
void OMP2Wave::occ_iterations()
{
  
fprintf(outfile,"\n");      
fprintf(outfile," ============================================================================== \n");    
fprintf(outfile," ================ Performing OMP2 iterations... =============================== \n");  
fprintf(outfile," ============================================================================== \n");
fprintf(outfile, "\t            Minimizing MP2-L Functional \n");
fprintf(outfile, "\t            --------------------------- \n");
fprintf(outfile, " Iter       E_total           DE           RMS MO Grad      MAX MO Grad      RMS Korb      MAX Korb      RMS T2    \n");
fprintf(outfile, " ----    ---------------    ----------     -----------      -----------     ----------    -----------   ----------  \n");
fflush(outfile);

  
/********************************************************************************************/
/************************** NR iterations **************************************************/
/********************************************************************************************/
      itr_occ = 0;
      mu_ls = 0;
      conver = 1; // Assuming that the MOs will be optimized.
      mo_optimized = 0; 
      
      if (opt_method == "DIIS") {
	nvar = num_vecs +1;
        vecsA = new Array2d(num_vecs, nidpA, "Alpha MO DIIS Vectors");
        errvecsA = new Array2d(num_vecs, nidpA, "Alpha MO DIIS Error Vectors");
        vecsA->zero();
        errvecsA->zero();

        if (reference_ == "UNRESTRICTED") {
            vecsB = new Array2d(num_vecs, nidpB, "Beta MO DIIS Vectors");
            errvecsB = new Array2d(num_vecs, nidpB, "Beta MO DIIS Error Vectors");
            vecsB->zero();
            errvecsB->zero();
        }
      }
      
      
// head of loop      
do
{
       itr_occ++;
	
/********************************************************************************************/
/************************** New orbital step ************************************************/
/********************************************************************************************/
        timer_on("kappa orb rot");
        korbrot_sd();
        timer_off("kappa orb rot");

/********************************************************************************************/ 
/************************** update mo coefficients ******************************************/
/********************************************************************************************/
        timer_on("update_mo");
        update_mo();
        timer_off("update_mo");

/********************************************************************************************/ 
/************************** Transform TEI from SO to MO space *******************************/
/********************************************************************************************/
        timer_on("trans_ints");
	if (reference_ == "RESTRICTED") trans_ints_rhf();  
	else if (reference_ == "UNRESTRICTED") trans_ints_uhf();  
        timer_off("trans_ints");
 
/********************************************************************************************/
/************************** NEW amplitudes **************************************************/
/********************************************************************************************/
        timer_on("T2(1)");
        t2_1st_general();  
        timer_off("T2(1)");

/********************************************************************************************/
/************************** One-particle and two-particle density matrices ******************/
/********************************************************************************************/
        timer_on("Response PDMs");
	response_pdms();
        timer_off("Response PDMs");

/********************************************************************************************/
/************************** Asymmetric Generalizd-Fock matrix *******************************/
/********************************************************************************************/
        timer_on("Generalized-Fock");
	GFockmo();
        timer_off("Generalized-Fock");
	
/********************************************************************************************/
/************************** Compute Lagrangian Energy ***************************************/
/********************************************************************************************/
        timer_on("MP2L Energy");
	mp2l_energy();
        timer_off("MP2L Energy");

/********************************************************************************************/
/************************** new orbital gradient ********************************************/
/********************************************************************************************/	 
        timer_on("MO Grad");
	mograd();
        timer_off("MO Grad");
      
/********************************************************************************************/
/************************** Print ***********************************************************/
/********************************************************************************************/
    if (reference_ == "RESTRICTED") {
	nidp=nidpA;
	rms_wog=rms_wogA;
	biggest_mograd=biggest_mogradA;
	rms_kappa=rms_kappaA;
	biggest_kappa=biggest_kappaA;
    }

    else if (reference_ == "UNRESTRICTED") {
	nidp=MAX0(nidpA,nidpB);
	rms_wog=MAX0(rms_wogA,rms_wogB);
	biggest_mograd=MAX0(biggest_mogradA,biggest_mogradB);
	rms_kappa=MAX0(rms_kappaA,rms_kappaB);
	biggest_kappa=MAX0(biggest_kappaA,biggest_kappaB);
	rms_t2=MAX0(rms_t2AA,rms_t2BB);
	rms_t2=MAX0(rms_t2,rms_t2AB);
    }
	
fprintf(outfile," %3d     %12.10f  %12.2e   %12.2e     %12.2e    %12.2e  %12.2e  %12.2e \n",
	           itr_occ,Emp2L,DE,rms_wog,biggest_mograd,rms_kappa,biggest_kappa,rms_t2);
fflush(outfile);

/********************************************************************************************/
/********************************************************************************************/   
    if (itr_occ >= mo_maxiter) {
      conver = 0; // means MOs are NOT optimized
      break;  
    }

    if (rms_wog == 0.0) break;

}
while(fabs(DE) >= tol_Eod || rms_wog >= tol_grad || rms_kappa >= tol_grad || biggest_mograd >= mograd_max || 
      biggest_kappa >= mograd_max || rms_t2 >= tol_t2); 

if (conver == 1) {
mo_optimized = 1; 
fprintf(outfile,"\n");
fprintf(outfile," ============================================================================== \n");
fprintf(outfile," ======================== OMP2 ITERATIONS ARE CONVERGED ======================= \n");
fprintf(outfile," ============================================================================== \n");
fflush(outfile);
}

else if (conver == 0) {
  fprintf(outfile,"\n ======================== OMP2 IS NOT CONVERGED IN %2d ITERATIONS ============= \n", mo_maxiter);
  fflush(outfile);
}

/********************************************************************************************/
/********************************************************************************************/

}
}} // End Namespaces

