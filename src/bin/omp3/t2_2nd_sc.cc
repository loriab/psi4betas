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

/** Required PSI3 includes */ 
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

/** Required libmints includes */
#include <libmints/mints.h>
#include <libmints/factory.h>
#include <libmints/wavefunction.h>

#include "defines.h"
#include "omp3wave.h"

using namespace boost;
using namespace psi;
using namespace std;


namespace psi{ namespace omp3wave{
  
void OMP3Wave::t2_2nd_sc()
{   

//===========================================================================================
//========================= RHF =============================================================
//===========================================================================================
if (reference == "RHF") {

     dpdbuf4 K, T, TAA, TAB, TBB, Tp, D, W, Tau, Ttemp;     
     psio_->open(PSIF_LIBTRANS_DPD, PSIO_OPEN_OLD);
     psio_->open(PSIF_OMP3_DPD, PSIO_OPEN_OLD);

    // Build T(IA,JB)    
    // T_IJ^AB(2) = \sum_{M,E} Tau_IM^AE(1) W_MBEJ(1) => T(IA,JB)(2) = \sum_{M,E} Tau'(IA,ME) (ME|JB)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "T2_2 (IA|JB)");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "Tau_1 (OV|OV)");
    dpd_buf4_init(&K, PSIF_LIBTRANS_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "MO Ints (OV|OV)");
    dpd_contract444(&Tp, &K, &T, 0, 0, 1.0, 0.0);
    // T_IJ^AB(2) += \sum_{M,E} Tau_JM^BE(1) W_MAEI(1) => T(IA,JB)(2) = \sum_{M,E} Tau'(JB,ME) (ME|IA)
    dpd_contract444(&K, &Tp, &T, 0, 0, 1.0, 1.0);
    dpd_buf4_close(&K);
    dpd_buf4_close(&Tp);
    
    // T_IJ^AB(2) -= \sum_{m,e} T_im^ae(1) W_mbje(1) => T(IA,JB)(2) -= \sum_{m,e} T'(ia,me) <me|jb>
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "T2_1 (OV|OV)");
    dpd_buf4_init(&K, PSIF_LIBTRANS_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "MO Ints <OV|OV>");
    dpd_contract444(&Tp, &K, &T, 0, 0, -1.0, 1.0);
    // T_IJ^AB(2) -= \sum_{m,e} T_jm^be(1) W_maie(1) => T(IA,JB)(2) -= \sum_{m,e} T'(jb,me) <me|ia>
    dpd_contract444(&K, &Tp, &T, 0, 0, -1.0, 1.0);
    dpd_buf4_close(&K);
    dpd_buf4_close(&Tp);
    
    // T(IA,JB) => T_IJ^AB(2)
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , prqs, ID("[O,O]"), ID("[V,V]"), "T2_2 <IJ|AB>");
    dpd_buf4_close(&T);
    
    
    
    // Build T(JA,IB)    
    // T_IJ^AB(2) = -\sum_{M,E} T_MJ^AE(1) W_MBIE(1) => T(JA,IB)(2) = -\sum_{M,E} T"(JA,ME) <ME|IB>
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "T2_2 (JA|IB)");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "T2_1pp (OV|OV)");
    dpd_buf4_init(&K, PSIF_LIBTRANS_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "MO Ints <OV|OV>");
    dpd_contract444(&Tp, &K, &T, 0, 0, -1.0, 0.0);
    // T_IJ^AB(2) = -\sum_{M,E} T_IM^EB(1) W_MAJE(1) => T(JA,IB)(2) = -\sum_{M,E} T"(ME,IB) <ME|JA>
    dpd_contract444(&K, &Tp, &T, 1, 1, -1.0, 1.0);
    dpd_buf4_close(&K);
    dpd_buf4_close(&Tp);
    
    // T(JA,IB) => T_IJ^AB(2)
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rpqs, ID("[O,O]"), ID("[V,V]"), "T2_2 (IJ|AB)");
    dpd_buf4_close(&T);    
    
     
     
    // Build T2AB
    // T_IJ^AB(2) = T(IA,JB)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <IJ|AB>");
    dpd_buf4_copy(&T, PSIF_OMP3_DPD, "T2_2 <OO|VV>");
    dpd_buf4_close(&T); 
    
    // T_IJ^AB(2) += T(JA,IB)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <OO|VV>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 (IJ|AB)");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&Tp); 
    
    // T_IJ^AB(2) += \sum_{M,N} T_MN^AB(1) W_MNIJ(1) = \sum_{M,N} T_MN^AB(1) <MN|IJ>
    dpd_buf4_init(&TAA, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_1 <OO|VV>");
    dpd_buf4_init(&K, PSIF_LIBTRANS_DPD, 0, ID("[O,O]"), ID("[O,O]"),
                  ID("[O,O]"), ID("[O,O]"), 0, "MO Ints <OO|OO>");
    dpd_contract444(&K, &TAA, &T, 1, 1, 1.0, 1.0);
    dpd_buf4_close(&K);
    dpd_buf4_close(&T);
   

    // NOTE: in contract444 Z = X * Y, the order of X and Y is important for algorithm selecting.
    // In order to push libdpd to OOC choose X as the larger matrix.

    // T_IJ^AB(2) += \sum_{E,F} T_IJ^EF(1) <AB|EF>
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[V,V]"), ID("[O,O]"),
                  ID("[V,V]"), ID("[O,O]"), 0, "Z2_2 <VV|OO>");
    dpd_buf4_init(&K, PSIF_LIBTRANS_DPD, 0, ID("[V,V]"), ID("[V,V]"),
                  ID("[V,V]"), ID("[V,V]"), 0, "MO Ints <VV|VV>");
    dpd_contract444(&K, &TAA, &T, 0, 0, 1.0, 0.0);
    dpd_buf4_close(&K);
    dpd_buf4_close(&TAA);
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rspq, ID("[O,O]"), ID("[V,V]"), "Z2_2 <OO|VV>");
    dpd_buf4_close(&T);
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <OO|VV>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "Z2_2 <OO|VV>");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&T);
    dpd_buf4_close(&Tp);
 
    
    // T_IJ^AB = T_IJ^AB / D_IJ^AB
    dpd_buf4_init(&D, PSIF_LIBTRANS_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "D <OO|VV>");
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <OO|VV>");
    dpd_buf4_dirprd(&D, &T);
    dpd_buf4_close(&D);
    if (print_ > 2) dpd_buf4_print(&T, outfile, 1);
    dpd_buf4_close(&T);


    // Build T2 = T2(1) + T2(2)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_1 <OO|VV>");
    dpd_buf4_copy(&T, PSIF_OMP3_DPD, "T2 <OO|VV>");
    dpd_buf4_close(&T);
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2 <OO|VV>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <OO|VV>");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&T);
    dpd_buf4_close(&Tp);

     // Build Tau(ij,ab) = 2*T(ij,ab) - T(ji,ab)
     // Build TAA(ij,ab) = T(ij,ab) - T(ji,ab)
     dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <OO|VV>");
     dpd_buf4_copy(&T, PSIF_OMP3_DPD, "Tau_2 <OO|VV>");
     dpd_buf4_copy(&T, PSIF_OMP3_DPD, "T2_2AA <OO|VV>");
     dpd_buf4_sort(&T, PSIF_OMP3_DPD, qprs, ID("[O,O]"), ID("[V,V]"), "T2_2jiab <OO|VV>");
     dpd_buf4_close(&T);
     dpd_buf4_init(&Tau, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "Tau_2 <OO|VV>");
     dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2AA <OO|VV>");
     dpd_buf4_init(&Ttemp, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2jiab <OO|VV>");
     dpd_buf4_scm(&Tau, 2.0);
     dpd_buf4_axpy(&Ttemp, &Tau, -1.0); // -1.0*Ttemp + Tau -> Tau
     dpd_buf4_axpy(&Ttemp, &Tp, -1.0); // -1.0*Ttemp + Tp -> Tp
     dpd_buf4_close(&Ttemp);
     dpd_buf4_close(&Tp);
     dpd_buf4_close(&Tau);

     // Build Tau(ij,ab) = 2*T(ij,ab) - T(ji,ab)
     // Build TAA(ij,ab) = T(ij,ab) - T(ji,ab)
     dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2 <OO|VV>");
     dpd_buf4_copy(&T, PSIF_OMP3_DPD, "Tau <OO|VV>");
     dpd_buf4_copy(&T, PSIF_OMP3_DPD, "T2AA <OO|VV>");
     dpd_buf4_sort(&T, PSIF_OMP3_DPD, qprs, ID("[O,O]"), ID("[V,V]"), "T2jiab <OO|VV>");
     dpd_buf4_close(&T);
     dpd_buf4_init(&Tau, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "Tau <OO|VV>");
     dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2AA <OO|VV>");
     dpd_buf4_init(&Ttemp, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2jiab <OO|VV>");
     dpd_buf4_scm(&Tau, 2.0);
     dpd_buf4_axpy(&Ttemp, &Tau, -1.0); // -1.0*Ttemp + Tau -> Tau
     dpd_buf4_axpy(&Ttemp, &Tp, -1.0); // -1.0*Ttemp + Tp -> Tp
     dpd_buf4_close(&Ttemp);
     dpd_buf4_close(&Tp);
     dpd_buf4_close(&Tau);
 
    psio_->close(PSIF_LIBTRANS_DPD, 1);
    psio_->close(PSIF_OMP3_DPD, 1);

}// end if (reference == "RHF") 


//===========================================================================================
//========================= UHF =============================================================
//===========================================================================================
else if (reference == "UHF") {

/********************************************************************************************/
/************************** Build W intermediates *******************************************/
/********************************************************************************************/     
     timer_on("W int");
     W_1st_order();     
     timer_off("W int");
 
     dpdbuf4 K, T, TAA, TAB, TBB, Tp, D, W;     
     psio_->open(PSIF_LIBTRANS_DPD, PSIO_OPEN_OLD);
     psio_->open(PSIF_OMP3_DPD, PSIO_OPEN_OLD);
     
/********************************************************************************************/
/************************** Alpha-Alpha spin case *******************************************/
/********************************************************************************************/   
    // Build T(IA,JB)    
    // T_IJ^AB(2) = \sum_{M,E} T_IM^AE(1) W_MBEJ(1) => T(IA,JB)(2) = \sum_{M,E} T(IA,ME) W(ME,JB)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "T2_2 (IA|JB)");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "T2_1 (OV|OV)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "W_1 (OV|OV)");
    dpd_contract444(&Tp, &W, &T, 0, 0, 1.0, 0.0);
    // T_IJ^AB(2) += \sum_{M,E} T_JM^BE(1) W_MAEI(1) => T(IA,JB)(2) = \sum_{M,E} T(JB,ME) W(ME,IA)
    dpd_contract444(&W, &Tp, &T, 0, 0, 1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    // T_IJ^AB(2) += \sum_{m,e} T_Im^Ae(1) W_JeBm(1) => T(IA,JB)(2) += \sum_{m,e} T(IA,me) W(JB,me)
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "T2_1 (OV|ov)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "W_1 (OV|ov)");
    dpd_contract444(&Tp, &W, &T, 0, 0, 1.0, 1.0);
    // T_IJ^AB(2) += \sum_{m,e} T_Jm^Be(1) W_IeAm(1) => T(IA,JB)(2) += \sum_{m,e} T(JB,me) W(IA,me)
    dpd_contract444(&W, &Tp, &T, 0, 0, 1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    // T(IA,JB) => T_IJ^AB(2)
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , prqs, ID("[O,O]"), ID("[V,V]"), "T2_2 <IJ|AB>");
    dpd_buf4_close(&T);
    
    
    
    // Build T(JA,IB)    
    // T_IJ^AB(2) = -\sum_{M,E} T_JM^AE(1) W_MBEI(1) => T(JA,IB)(2) = -\sum_{M,E} T(JA,ME) W(ME,IB)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "T2_2 (JA|IB)");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "T2_1 (OV|OV)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "W_1 (OV|OV)");
    dpd_contract444(&Tp, &W, &T, 0, 0, -1.0, 0.0);
    // T_IJ^AB(2) = -\sum_{M,E} T_IM^BE(1) W_MAEJ(1) => T(JA,IB)(2) = -\sum_{M,E} T(IB,ME) W(ME,JA)
    dpd_contract444(&W, &Tp, &T, 0, 0, -1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    // T_IJ^AB(2) = -\sum_{m,e} T_Jm^Ae(1) W_IeBm(1) => T(JA,IB)(2) -= \sum_{m,e} T(JA,me) W(IB,me)
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "T2_1 (OV|ov)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "W_1 (OV|ov)");
    dpd_contract444(&Tp, &W, &T, 0, 0, -1.0, 1.0);
    // T_IJ^AB(2) = -\sum_{m,e} T_Im^Be(1) W_JeAm(1) => T(JA,IB)(2) -= \sum_{m,e} T(IB,me) W(JA,me)
    dpd_contract444(&W, &Tp, &T, 0, 0, -1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    // T(JA,IB) => T_IJ^AB(2)
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rpqs, ID("[O,O]"), ID("[V,V]"), "T2_2 (IJ|AB)");
    dpd_buf4_close(&T);    
    
     
     
    // Build T2AA
    // T_IJ^AB(2) = T(IA,JB)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <IJ|AB>");
    dpd_buf4_copy(&T, PSIF_OMP3_DPD, "T2_2 <OO|VV>");
    dpd_buf4_close(&T); 
    
    // T_IJ^AB(2) += T(JA,IB)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <OO|VV>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 (IJ|AB)");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&Tp); 
    
    // T_IJ^AB(2) += 1/2 \sum_{M,N} T_MN^AB(1) W_MNIJ(1)
    dpd_buf4_init(&TAA, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_1 <OO|VV>");
    //dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[O,O]"),
    //              ID("[O,O]"), ID("[O,O]"), 0, "W_1 <OO|OO>");
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[O,O]"), ID("[O,O]"),
                  ID("[O,O]"), ID("[O,O]"), 0, "MO Ints <OO||OO>");
    dpd_contract444(&W, &TAA, &T, 1, 1, 0.5, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&T);
   
    /* 
    // T_IJ^AB(2) += 1/2 \sum_{E,F} T_IJ^EF(1) W_ABEF(1)
    //dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[V,V]"), ID("[V,V]"),
    //              ID("[V,V]"), ID("[V,V]"), 0, "W_1 <VV|VV>");
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[V,V]"), ID("[V,V]"),
                  ID("[V,V]"), ID("[V,V]"), 0, "MO Ints <VV||VV>");
    dpd_contract444(&TAA, &W, &T, 0, 0, 0.5, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&T);
    dpd_buf4_close(&TAA);
    */

    /*
    // T_IJ^AB(2) += 1/2 \sum_{E,F} T_IJ^EF(1) <EF||AB> = \sum_{E,F} T_IJ^EF(1) <AB|EF>
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[V,V]"), ID("[V,V]"),
                  ID("[V,V]"), ID("[V,V]"), 0, "MO Ints <VV|VV>");
    dpd_contract444(&TAA, &W, &T, 0, 0, 1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&T);
    dpd_buf4_close(&TAA);
    */

    // NOTE: in contract444 Z = X * Y, the order of X and Y is important for algorithm selecting.
    // In order to push libdpd to OOC choose X as the larger matrix.

    // T_IJ^AB(2) += 1/2 \sum_{E,F} T_IJ^EF(1) <EF||AB> = \sum_{E,F} T_IJ^EF(1) <AB|EF>
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[V,V]"), ID("[O,O]"),
                  ID("[V,V]"), ID("[O,O]"), 0, "Z2_2 <VV|OO>");
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[V,V]"), ID("[V,V]"),
                  ID("[V,V]"), ID("[V,V]"), 0, "MO Ints <VV|VV>");
    dpd_contract444(&W, &TAA, &T, 0, 0, 1.0, 0.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&TAA);
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rspq, ID("[O,O]"), ID("[V,V]"), "Z2_2 <OO|VV>");
    dpd_buf4_close(&T);
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <OO|VV>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "Z2_2 <OO|VV>");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&T);
    dpd_buf4_close(&Tp);
 
    
    // T_IJ^AB = T_IJ^AB / D_IJ^AB
    dpd_buf4_init(&D, PSIF_LIBTRANS_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "D <OO|VV>");
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <OO|VV>");
    dpd_buf4_dirprd(&D, &T);
    dpd_buf4_close(&D);
    if (print_ > 2) dpd_buf4_print(&T, outfile, 1);
    dpd_buf4_close(&T);
    
/********************************************************************************************/
/************************** Beta-Beta spin case *********************************************/
/********************************************************************************************/     
    // Build T(ia,jb)    
    // T_ij^ab(2) = \sum_{m,e} T_im^ae(1) W_mbej(1) => T(ia,jb)(2) = \sum_{m,e} T(ia,me) W(me,jb)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[o,v]"), ID("[o,v]"),
                  ID("[o,v]"), ID("[o,v]"), 0, "T2_2 (ia|jb)");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[o,v]"), ID("[o,v]"),
                  ID("[o,v]"), ID("[o,v]"), 0, "T2_1 (ov|ov)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[o,v]"), ID("[o,v]"),
                  ID("[o,v]"), ID("[o,v]"), 0, "W_1 (ov|ov)");
    dpd_contract444(&Tp, &W, &T, 0, 0, 1.0, 0.0);
    // T_ij^ab(2) += \sum_{m,e} T_jm^be(1) W_maei(1) => T(ia,jb)(2) = \sum_{m,e} T(jb,me) W(me,ia)
    dpd_contract444(&W, &Tp, &T, 0, 0, 1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    // T_ij^ab(2) += \sum_{M,E} T_Mi^Ea(1) W_MbEj(1) => T(ia,jb)(2) += \sum_{M,E} T(ME,ia) W(ME,jb)
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "T2_1 (OV|ov)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "W_1 (OV|ov)");
    dpd_contract444(&Tp, &W, &T, 1, 1, 1.0, 1.0);
    // T_ij^ab(2) += \sum_{M,E} T_Mj^Eb(1) W_MaEi(1) => T(ia,jb)(2) += \sum_{M,E} T(ME,jb) W(ME,ia)
    dpd_contract444(&W, &Tp, &T, 1, 1, 1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    // T(ia,jb) => T_ij^ab(2)
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , prqs, ID("[o,o]"), ID("[v,v]"), "T2_2 <ij|ab>");
    dpd_buf4_close(&T);
    
    
    
    // Build T(ja,ib)    
    // T_ij^ab(2) = -\sum_{m,e} T_jm^ae(1) W_mbei(1) => T(ja,ib)(2) = -\sum_{m,e} T(ja,me) W(me,ib)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[o,v]"), ID("[o,v]"),
                  ID("[o,v]"), ID("[o,v]"), 0, "T2_2 (ja|ib)");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[o,v]"), ID("[o,v]"),
                  ID("[o,v]"), ID("[o,v]"), 0, "T2_1 (ov|ov)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[o,v]"), ID("[o,v]"),
                  ID("[o,v]"), ID("[o,v]"), 0, "W_1 (ov|ov)");
    dpd_contract444(&Tp, &W, &T, 0, 0, -1.0, 0.0);
    // T_ij^ab(2) = -\sum_{m,e} T_im^be(1) W_maej(1) => T(ja,ib)(2) = -\sum_{m,e} T(ib,me) W(me,ja)
    dpd_contract444(&W, &Tp, &T, 0, 0, -1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    // T_ij^ab(2) = -\sum_{M,E} T_Mj^Ea(1) W_MbEi(1) => T(ja,ib)(2) -= \sum_{M,E} T(ME,ja) W(ME,ib)
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "T2_1 (OV|ov)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "W_1 (OV|ov)");
    dpd_contract444(&Tp, &W, &T, 1, 1, -1.0, 1.0);
    // T_ij^ab(2) = -\sum_{M,E} T_Mi^Eb(1) W_MaEj(1) => T(ja,ib)(2) -= \sum_{M,E} T(ME,ib) W(ME,ja)
    dpd_contract444(&W, &Tp, &T, 1, 1, -1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    // T(ja,ib) => T_ij^ab(2)
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rpqs, ID("[o,o]"), ID("[v,v]"), "T2_2 (ij|ab)");
    dpd_buf4_close(&T);    
    
     
     
    // Build T2BB
    // T_ij^ab(2) = T(ia,jb)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "T2_2 <ij|ab>");
    dpd_buf4_copy(&T, PSIF_OMP3_DPD, "T2_2 <oo|vv>");
    dpd_buf4_close(&T); 
    
    // T_ij^ab(2) += T(ja,ib)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "T2_2 <oo|vv>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "T2_2 (ij|ab)");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&Tp); 
    
    // T_ij^ab(2) += 1/2 \sum_{m,n} T_mn^ab(1) W_mnij(1)
    dpd_buf4_init(&TBB, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "T2_1 <oo|vv>");
    //dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[o,o]"),
    //              ID("[o,o]"), ID("[o,o]"), 0, "W_1 <oo|oo>");
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[o,o]"), ID("[o,o]"),
                  ID("[o,o]"), ID("[o,o]"), 0, "MO Ints <oo||oo>");
    dpd_contract444(&W, &TBB, &T, 1, 1, 0.5, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&T);
   
    /* 
    // T_ij^ab(2) += 1/2 \sum_{e,f} T_ij^ef(1) W_abef(1)
    //dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[v,v]"), ID("[v,v]"),
    //              ID("[v,v]"), ID("[v,v]"), 0, "W_1 <vv|vv>");
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[v,v]"), ID("[v,v]"),
                  ID("[v,v]"), ID("[v,v]"), 0, "MO Ints <vv||vv>");
    dpd_contract444(&TBB, &W, &T, 0, 0, 0.5, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&T);
    dpd_buf4_close(&TBB);  
    */
    /*
    // T_ij^ab(2) += 1/2 \sum_{e,f} T_ij^ef(1) <ef||ab> = \sum_{e,f} T_ij^ef(1) <ab|ef>
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[v,v]"), ID("[v,v]"),
                  ID("[v,v]"), ID("[v,v]"), 0, "MO Ints <vv|vv>");
    dpd_contract444(&TBB, &W, &T, 0, 0, 1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&T);
    dpd_buf4_close(&TBB); 
    */
    
    // T_ij^ab(2) += 1/2 \sum_{e,f} T_ij^ef(1) <ef||ab> = \sum_{e,f} T_ij^ef(1) <ab|ef>
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[v,v]"), ID("[o,o]"),
                  ID("[v,v]"), ID("[o,o]"), 0, "Z2_2 <vv|oo>");
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[v,v]"), ID("[v,v]"),
                  ID("[v,v]"), ID("[v,v]"), 0, "MO Ints <vv|vv>");
    dpd_contract444(&W, &TBB, &T, 0, 0, 1.0, 0.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&TBB);
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rspq, ID("[o,o]"), ID("[v,v]"), "Z2_2 <oo|vv>");
    dpd_buf4_close(&T);
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "T2_2 <oo|vv>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "Z2_2 <oo|vv>");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&T);
    dpd_buf4_close(&Tp);

 
    // T_ij^ab = T_ij^ab / D_ij^ab
    dpd_buf4_init(&D, PSIF_LIBTRANS_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "D <oo|vv>");
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "T2_2 <oo|vv>");
    dpd_buf4_dirprd(&D, &T);
    dpd_buf4_close(&D);
    if (print_ > 2) dpd_buf4_print(&T, outfile, 1);
    dpd_buf4_close(&T);    
    
/********************************************************************************************/
/************************** Alpha-Beta spin case ********************************************/
/********************************************************************************************/     
    // Build T(IA,jb)    
    /*
    // T_Ij^Ab(2) = \sum_{M,E} T_IM^AE(1) W_MbEj(1) => T(IA,jb)(2) = \sum_{M,E} T(IA,ME) W(ME,jb)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "T2_2 (IA|jb)");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "T2_1 (OV|OV)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "W_1 (OV|ov)");
    dpd_contract444(&Tp, &W, &T, 0, 1, 1.0, 0.0);
    dpd_buf4_close(&Tp);
    */
    
    // T_Ij^Ab(2) = \sum_{M,E} T_IM^AE(1) W_MbEj(1) => T(IA,jb)(2) = \sum_{M,E} T(IA,ME) W(jb,ME)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "T2_2 (IA|jb)");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "T2_1 (OV|OV)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[o,v]"), ID("[O,V]"),
                  ID("[o,v]"), ID("[O,V]"), 0, "W_1 (ov|OV)");
    dpd_contract444(&Tp, &W, &T, 0, 0, 1.0, 0.0);
    dpd_buf4_close(&Tp);
    dpd_buf4_close(&W);


    // T_Ij^Ab(2) += \sum_{m,e} T_jm^be(1) W_IeAm(1) => T(IA,jb)(2) = \sum_{m,e} W(IA,me) T(jb,me)  
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[o,v]"), ID("[o,v]"),
                  ID("[o,v]"), ID("[o,v]"), 0, "T2_1 (ov|ov)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "W_1 (OV|ov)");
    dpd_contract444(&W, &Tp, &T, 0, 0, 1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    // T_Ij^Ab(2) += \sum_{m,e} T_Im^Ae(1) W_mbej(1) => T(IA,jb)(2) += \sum_{m,e} T(IA,me) W(me,jb)
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[o,v]"),
                  ID("[O,V]"), ID("[o,v]"), 0, "T2_1 (OV|ov)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[o,v]"), ID("[o,v]"),
                  ID("[o,v]"), ID("[o,v]"), 0, "W_1 (ov|ov)");
    dpd_contract444(&Tp, &W, &T, 0, 0, 1.0, 1.0);
    dpd_buf4_close(&W);
    
    // T_Ij^Ab(2) += \sum_{M,E} T_Mj^Eb(1) W_MAEI(1) => T(IA,jb)(2) += \sum_{M,E} W(ME,IA) T(ME,jb)  
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,V]"), ID("[O,V]"),
                  ID("[O,V]"), ID("[O,V]"), 0, "W_1 (OV|OV)");
    dpd_contract444(&W, &Tp, &T, 1, 1, 1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    // T(IA,jb) => T_Ij^Ab(2)
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , prqs, ID("[O,o]"), ID("[V,v]"), "T2_2 <Ij|Ab>");
    dpd_buf4_close(&T);
    
    
    
    // Build T(jA,Ib)    
    // T_Ij^Ab(2) = \sum_{M,e} T_Mj^Ae(1) W_MbeI(1) => T(jA,Ib)(2) = \sum_{M,e} T(jA,Me) W(Me,Ib)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[o,V]"), ID("[O,v]"),
                  ID("[o,V]"), ID("[O,v]"), 0, "T2_2 (jA|Ib)");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[o,V]"), ID("[O,v]"),
                  ID("[o,V]"), ID("[O,v]"), 0, "T2_1 (oV|Ov)");
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,v]"), ID("[O,v]"),
                  ID("[O,v]"), ID("[O,v]"), 0, "W_1 (Ov|Ov)");
    dpd_contract444(&Tp, &W, &T, 0, 0, 1.0, 0.0);
    dpd_buf4_close(&W);
    
    // T_Ij^Ab(2) = +\sum_{m,E} T_Im^Eb(1) W_mAEj(1) => T(jA,Ib)(2) = +\sum_{m,E} W(mE,jA) T(mE,Ib) 
    dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[o,V]"), ID("[o,V]"),
                  ID("[o,V]"), ID("[o,V]"), 0, "W_1 (oV|oV)");
    dpd_contract444(&W, &Tp, &T, 1, 1, 1.0, 1.0); 
    dpd_buf4_close(&W);
    dpd_buf4_close(&Tp);
    
    
    // T(jA,Ib) => T_Ij^Ab(2)
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rpqs, ID("[O,o]"), ID("[V,v]"), "T2_2 (Ij|Ab)");
    dpd_buf4_close(&T);    
    
     
     
    // Build T2AB
    // T_Ij^Ab(2) = T(IA,jb)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "T2_2 <Ij|Ab>");
    dpd_buf4_copy(&T, PSIF_OMP3_DPD, "T2_2 <Oo|Vv>");
    dpd_buf4_close(&T); 
    
    // T_Ij^Ab(2) += T(jA,Ib)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "T2_2 <Oo|Vv>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "T2_2 (Ij|Ab)");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&Tp); 
    
    // T_Ij^Ab(2) += \sum_{M,n} T_Mn^Ab(1) W_MnIj(1) = \sum_{M,n} W(Mn,Ij) T(Mn,Ab)
    dpd_buf4_init(&TAB, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "T2_1 <Oo|Vv>");
    //dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[O,o]"),
    //              ID("[O,o]"), ID("[O,o]"), 0, "W_1 <Oo|Oo>");
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[O,o]"), ID("[O,o]"),
                  ID("[O,o]"), ID("[O,o]"), 0, "MO Ints <Oo|Oo>");
    dpd_contract444(&W, &TAB, &T, 1, 1, 1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&T);

    /*    
    // T_Ij^Ab(2) +=  \sum_{E,f} T_Ij^Ef(1) W_AbEf(1) =  \sum_{E,f} T(Ij,Ef) W(Ab,Ef)
    //             = \sum_{E,f} T(Ij,Ef) <Ef|Ab>
    //dpd_buf4_init(&W, PSIF_OMP3_DPD, 0, ID("[V,v]"), ID("[V,v]"),
    //              ID("[V,v]"), ID("[V,v]"), 0, "W_1 <Vv|Vv>");
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[V,v]"), ID("[V,v]"),
                  ID("[V,v]"), ID("[V,v]"), 0, "MO Ints <Vv|Vv>");
    dpd_contract444(&TAB, &W, &T, 0, 0, 1.0, 1.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&T);
    dpd_buf4_close(&TAB);
    */
  
    // T_Ij^Ab(2) +=  \sum_{E,f} T_Ij^Ef(1) W_AbEf(1) =  \sum_{E,f} T(Ij,Ef) W(Ab,Ef)
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[V,v]"), ID("[O,o]"),
                  ID("[V,v]"), ID("[O,o]"), 0, "Z2_2 <Vv|Oo>");
    dpd_buf4_init(&W, PSIF_LIBTRANS_DPD, 0, ID("[V,v]"), ID("[V,v]"),
                  ID("[V,v]"), ID("[V,v]"), 0, "MO Ints <Vv|Vv>");
    dpd_contract444(&W, &TAB, &T, 0, 0, 1.0, 0.0);
    dpd_buf4_close(&W);
    dpd_buf4_close(&TAB);
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rspq, ID("[O,o]"), ID("[V,v]"), "Z2_2 <Oo|Vv>");
    dpd_buf4_close(&T);
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "T2_2 <Oo|Vv>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "Z2_2 <Oo|Vv>");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&T);
    dpd_buf4_close(&Tp);
 


    // T_Ij^Ab = T_Ij^Ab / D_Ij^Ab
    dpd_buf4_init(&D, PSIF_LIBTRANS_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "D <Oo|Vv>");
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "T2_2 <Oo|Vv>");
    dpd_buf4_dirprd(&D, &T);
    dpd_buf4_close(&D);
    if (print_ > 2) dpd_buf4_print(&T, outfile, 1);
    dpd_buf4_close(&T);    
 
/********************************************************************************************/
/************************** Sum up 1st & 2nd order amplitudes *******************************/
/********************************************************************************************/    
    // Build T2 = T2(1) + T2(2)
    // Alpha-Alpha spin case
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_1 <OO|VV>");
    dpd_buf4_copy(&T, PSIF_OMP3_DPD, "T2 <OO|VV>");
    dpd_buf4_close(&T);
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2 <OO|VV>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2_2 <OO|VV>");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&T);
    dpd_buf4_close(&Tp);
    
    // Beta-Beta spin case
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "T2_1 <oo|vv>");
    dpd_buf4_copy(&T, PSIF_OMP3_DPD, "T2 <oo|vv>");
    dpd_buf4_close(&T);
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "T2 <oo|vv>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "T2_2 <oo|vv>");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&T);
    dpd_buf4_close(&Tp);
    
    // Alpha-Beta spin case
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "T2_1 <Oo|Vv>");
    dpd_buf4_copy(&T, PSIF_OMP3_DPD, "T2 <Oo|Vv>");
    dpd_buf4_close(&T);
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "T2 <Oo|Vv>");
    dpd_buf4_init(&Tp, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "T2_2 <Oo|Vv>");
    dpd_buf4_axpy(&Tp, &T, 1.0); // 1.0*Tp + T -> T
    dpd_buf4_close(&T);
    dpd_buf4_close(&Tp);
    
    /*
    // Build Lambda amplitudes 
    // T_IJ^AB => L_AB^IJ
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                  ID("[O,O]"), ID("[V,V]"), 0, "T2 <OO|VV>");
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rspq, ID("[V,V]"), ID("[O,O]"), "L2 <VV|OO>");
    dpd_buf4_close(&T);
    
    // T_ij^ab => L_ab^ij
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                  ID("[o,o]"), ID("[v,v]"), 0, "T2 <oo|vv>");
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rspq, ID("[v,v]"), ID("[o,o]"), "L2 <vv|oo>");
    dpd_buf4_close(&T);
     
    // T_Ij^Ab => L_Ab^Ij
    dpd_buf4_init(&T, PSIF_OMP3_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                  ID("[O,o]"), ID("[V,v]"), 0, "T2 <Oo|Vv>");
    dpd_buf4_sort(&T, PSIF_OMP3_DPD , rspq, ID("[V,v]"), ID("[O,o]"), "L2 <Vv|Oo>");
    dpd_buf4_close(&T);  
    */ 

    psio_->close(PSIF_LIBTRANS_DPD, 1);
    psio_->close(PSIF_OMP3_DPD, 1);
}// end if (reference == "UHF") 

} // end t2_2nd_sc
}} // End Namespaces

