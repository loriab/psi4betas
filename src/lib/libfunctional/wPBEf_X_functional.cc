/**********************************************************
* wPBEf_X_functional.cc: definitions for wPBEf_X_functional for KS-DFT
* Robert Parrish, robparrish@gmail.com
* Autogenerated by MATLAB Script on 07-Mar-2012
*
***********************************************************/
#include <libmints/mints.h>
#include <libfock/points.h>
#include "wPBEf_X_functional.h"
#include <stdlib.h>
#include <cmath>
#include <string>
#include <string>
#include <vector>

using namespace psi;
using namespace boost;
using namespace std;

namespace psi { namespace functional {

wPBEf_X_Functional::wPBEf_X_Functional(int npoints, int deriv) : Functional(npoints, deriv)
{

    name_ = "wPBEf_X";
    description_ = "Fake SR PBE Exchange Functional";
    citation_ = "J.P. Perdew et. al., Phys. Rev. Lett., 77(18), 3865-3868, 1996";

    double k = 3.0936677262801355E+00;
    params_.push_back(make_pair("k",k));
    double e = -2.3873241463784300E-01;
    params_.push_back(make_pair("e",e));
    double kp = 8.0400000000000005E-01;
    params_.push_back(make_pair("kp",kp));
    double mu_ = 2.1951497276451709E-01;
    params_.push_back(make_pair("mu_",mu_));

    is_gga_ = true;
    is_meta_ = false;

    //Required allocateion
    allocate();
}
wPBEf_X_Functional::~wPBEf_X_Functional()
{
}
void wPBEf_X_Functional::computeRKSFunctional(boost::shared_ptr<RKSFunctions> prop)
{
    int ntrue = prop->npoints();

    const double* restrict rho_a;
    const double* restrict gamma_aa;
    const double* restrict tau_a;

    rho_a = prop->property_value("RHO_A")->pointer();
    if (is_gga_) {
        gamma_aa = prop->property_value("GAMMA_AA")->pointer();
    }
    if (is_meta_) {
        tau_a = prop->property_value("TAU_A")->pointer();
    }

    double k = params_[0].second;
    double e = params_[1].second;
    double kp = params_[2].second;
    double mu_ = params_[3].second;

    //Functional
    for (int index = 0; index < ntrue; index++) {

        //Functional Value
        if (rho_a[index] > cutoff_) {
            double t128828 = rho_a[index]*2.0;
            functional_[index] = e*k*rho_a[index]*pow(t128828,1.0/3.0)*(kp-kp/((gamma_aa[index]*1.0/(k*k)*mu_* \
               1.0/pow(t128828,8.0/3.0))/kp+1.0)+1.0)*2.0;
        } else {
            functional_[index] = 0.0;
        } 

    }
    //First Partials
    for (int index = 0; index < ntrue && deriv_ >= 1; index++) {

        //V_rho_a
        if (rho_a[index] > cutoff_) {
            double t128830 = rho_a[index]*2.0;
            double t128831 = 1.0/(k*k);
            double t128832 = 1.0/kp;
            double t128833 = 1.0/pow(t128830,8.0/3.0);
            double t128834 = gamma_aa[index]*mu_*t128831*t128832*t128833;
            double t128835 = t128834+1.0;
            double t128836 = 1.0/t128835;
            double t128837 = kp-kp*t128836+1.0;
            v_rho_a_[index] = e*k*pow(t128830,1.0/3.0)*t128837+e*k*rho_a[index]*1.0/pow(t128830,2.0/3.0)*t128837* \
               (2.0/3.0)-(e*gamma_aa[index]*mu_*rho_a[index]*1.0/pow(t128830,1.0E1/3.0)*1.0/(t128835*t128835)*(1.6E1/ \
               3.0))/k;
        } else {
            v_rho_a_[index] = 0.0;
        } 

        if (is_gga_) {

            if (rho_a[index] > cutoff_) {
                double t128839 = rho_a[index]*2.0;
                v_gamma_aa_[index] = (e*mu_*rho_a[index]*1.0/pow(t128839,7.0/3.0)*1.0/pow((gamma_aa[index]*1.0/(k* \
                   k)*mu_*1.0/pow(t128839,8.0/3.0))/kp+1.0,2.0)*2.0)/k;
            } else {
                v_gamma_aa_[index] = 0.0;
            } 

        }
        if (is_meta_) {

            //V_tau_a
            if (rho_a[index] > cutoff_) {
                v_tau_a_[index] = 0.0;
            } else {
                v_tau_a_[index] = 0.0;
            } 

        }
    }
    //Second Partials
    for (int index = 0; index < ntrue && deriv_ >= 2; index++) {

        //V_rho_a_rho_a
        if (rho_a[index] > cutoff_) {
            double t128842 = rho_a[index]*2.0;
            double t128843 = 1.0/(k*k);
            double t128844 = 1.0/kp;
            double t128845 = 1.0/pow(t128842,8.0/3.0);
            double t128846 = gamma_aa[index]*mu_*t128843*t128844*t128845;
            double t128847 = t128846+1.0;
            double t128848 = 1.0/t128847;
            double t128849 = kp-kp*t128848+1.0;
            double t128850 = 1.0/k;
            double t128851 = 1.0/(t128847*t128847);
            v_rho_a_rho_a_[index] = e*k*1.0/pow(t128842,2.0/3.0)*t128849*(4.0/3.0)-e*k*rho_a[index]*1.0/pow(t128842,5.0/ \
               3.0)*t128849*(8.0/9.0)-e*gamma_aa[index]*mu_*t128850*1.0/pow(t128842,1.0E1/3.0)*t128851*(3.2E1/3.0) \
               -e*(gamma_aa[index]*gamma_aa[index])*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_a[index]*rho_a[index]*rho_a[index]* \
               rho_a[index]*rho_a[index]*rho_a[index])*t128844*1.0/(t128847*t128847*t128847)*(4.0/9.0)+e*gamma_aa[index]* \
               mu_*rho_a[index]*t128850*1.0/pow(t128842,1.3E1/3.0)*t128851*3.2E1;
        } else {
            v_rho_a_rho_a_[index] = 0.0;
        } 

        if (is_gga_) {

            //V_rho_a_gamma_aa
            if (rho_a[index] > cutoff_) {
                double t128853 = rho_a[index]*2.0;
                double t128854 = 1.0/k;
                double t128855 = 1.0/(k*k);
                double t128856 = 1.0/kp;
                double t128857 = 1.0/pow(t128853,8.0/3.0);
                double t128858 = gamma_aa[index]*mu_*t128855*t128856*t128857;
                double t128859 = t128858+1.0;
                double t128860 = 1.0/(t128859*t128859);
                v_rho_a_gamma_aa_[index] = e*mu_*t128860*1.0/pow(t128853,7.0/3.0)*t128854-e*mu_*rho_a[index]*t128860* \
                   1.0/pow(t128853,1.0E1/3.0)*t128854*(1.4E1/3.0)+e*gamma_aa[index]*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_a[index]* \
                   rho_a[index]*rho_a[index]*rho_a[index]*rho_a[index])*t128856*1.0/(t128859*t128859*t128859)*(1.0/6.0) \
                   ;
            } else {
                v_rho_a_gamma_aa_[index] = 0.0;
            } 

            //V_gamma_aa_gamma_aa
            if (rho_a[index] > cutoff_) {
                double t128862 = 1.0/kp;
                v_gamma_aa_gamma_aa_[index] = e*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_a[index]*rho_a[index]*rho_a[index]* \
                   rho_a[index])*t128862*1.0/pow(gamma_aa[index]*1.0/(k*k)*mu_*t128862*1.0/pow(rho_a[index]*2.0,8.0/3.0) \
                   +1.0,3.0)*(-1.0/8.0);
            } else {
                v_gamma_aa_gamma_aa_[index] = 0.0;
            } 

        }
        if (is_meta_) {

            //V_rho_a_tau_a
            if (rho_a[index] > cutoff_) {
                v_rho_a_tau_a_[index] = 0.0;
            } else {
                v_rho_a_tau_a_[index] = 0.0;
            } 

            //V_tau_a_tau_a
            if (rho_a[index] > cutoff_) {
                v_tau_a_tau_a_[index] = 0.0;
            } else {
                v_tau_a_tau_a_[index] = 0.0;
            } 

            if (is_gga_) {

                //V_gamma_aa_tau_a
                if (rho_a[index] > cutoff_) {
                    v_gamma_aa_tau_a_[index] = 0.0;
                } else {
                    v_gamma_aa_tau_a_[index] = 0.0;
                } 

            }
        }
    }
}
void wPBEf_X_Functional::computeUKSFunctional(boost::shared_ptr<UKSFunctions> prop)
{
    int ntrue = prop->npoints();

    const double* restrict rho_a;
    const double* restrict rho_b;
    const double* restrict gamma_aa;
    const double* restrict gamma_ab;
    const double* restrict gamma_bb;
    const double* restrict tau_a;
    const double* restrict tau_b;

    rho_a = prop->property_value("RHO_A")->pointer();
    rho_b = prop->property_value("RHO_B")->pointer();
    if (is_gga_) {
        gamma_aa = prop->property_value("GAMMA_AA")->pointer();
        gamma_ab = prop->property_value("GAMMA_AB")->pointer();
        gamma_bb = prop->property_value("GAMMA_BB")->pointer();
    }
    if (is_meta_) {
        tau_a = prop->property_value("TAU_A")->pointer();
        tau_b = prop->property_value("TAU_B")->pointer();
    }

    double k = params_[0].second;
    double e = params_[1].second;
    double kp = params_[2].second;
    double mu_ = params_[3].second;

    //Functional
    for (int index = 0; index < ntrue; index++) {

        //Functional Value
        if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
            double t128566 = rho_a[index]*2.0;
            double t128567 = 1.0/(k*k);
            double t128568 = 1.0/kp;
            double t128569 = rho_b[index]*2.0;
            functional_[index] = e*k*rho_a[index]*pow(t128566,1.0/3.0)*(kp-kp/(gamma_aa[index]*mu_*1.0/pow(t128566,8.0/ \
               3.0)*t128567*t128568+1.0)+1.0)+e*k*rho_b[index]*pow(t128569,1.0/3.0)*(kp-kp/(gamma_bb[index]*mu_*t128567* \
               t128568*1.0/pow(t128569,8.0/3.0)+1.0)+1.0);
        } else if (rho_a[index] > cutoff_) {
            double t128727 = rho_a[index]*2.0;
            functional_[index] = e*k*rho_a[index]*pow(t128727,1.0/3.0)*(kp-kp/((gamma_aa[index]*1.0/(k*k)*mu_* \
               1.0/pow(t128727,8.0/3.0))/kp+1.0)+1.0);
        } else if (rho_b[index] > cutoff_) {
            double t128662 = rho_b[index]*2.0;
            functional_[index] = e*k*rho_b[index]*pow(t128662,1.0/3.0)*(kp-kp/((gamma_bb[index]*1.0/(k*k)*mu_* \
               1.0/pow(t128662,8.0/3.0))/kp+1.0)+1.0);
        } else {
            functional_[index] = 0.0;
        } 

    }
    //First Partials
    for (int index = 0; index < ntrue && deriv_ >= 1; index++) {

        //V_rho_a
        if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
            double t128571 = rho_a[index]*2.0;
            double t128572 = 1.0/(k*k);
            double t128573 = 1.0/kp;
            double t128574 = 1.0/pow(t128571,8.0/3.0);
            double t128575 = gamma_aa[index]*mu_*t128572*t128573*t128574;
            double t128576 = t128575+1.0;
            double t128577 = 1.0/t128576;
            double t128578 = kp-kp*t128577+1.0;
            v_rho_a_[index] = e*k*pow(t128571,1.0/3.0)*t128578+e*k*rho_a[index]*1.0/pow(t128571,2.0/3.0)*t128578* \
               (2.0/3.0)-(e*gamma_aa[index]*mu_*rho_a[index]*1.0/pow(t128571,1.0E1/3.0)*1.0/(t128576*t128576)*(1.6E1/ \
               3.0))/k;
        } else if (rho_a[index] > cutoff_) {
            double t128729 = rho_a[index]*2.0;
            double t128730 = 1.0/(k*k);
            double t128731 = 1.0/kp;
            double t128732 = 1.0/pow(t128729,8.0/3.0);
            double t128733 = gamma_aa[index]*mu_*t128730*t128731*t128732;
            double t128734 = t128733+1.0;
            double t128735 = 1.0/t128734;
            double t128736 = kp-kp*t128735+1.0;
            v_rho_a_[index] = e*k*t128736*pow(t128729,1.0/3.0)+e*k*rho_a[index]*t128736*1.0/pow(t128729,2.0/3.0) \
               *(2.0/3.0)-(e*gamma_aa[index]*mu_*rho_a[index]*1.0/(t128734*t128734)*1.0/pow(t128729,1.0E1/3.0)*(1.6E1/ \
               3.0))/k;
        } else if (rho_b[index] > cutoff_) {
            v_rho_a_[index] = 0.0;
        } else {
            v_rho_a_[index] = 0.0;
        } 

        //V_rho_b
        if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
            double t128580 = rho_b[index]*2.0;
            double t128581 = 1.0/(k*k);
            double t128582 = 1.0/kp;
            double t128583 = 1.0/pow(t128580,8.0/3.0);
            double t128584 = gamma_bb[index]*mu_*t128581*t128582*t128583;
            double t128585 = t128584+1.0;
            double t128586 = 1.0/t128585;
            double t128587 = kp-kp*t128586+1.0;
            v_rho_b_[index] = e*k*pow(t128580,1.0/3.0)*t128587+e*k*rho_b[index]*1.0/pow(t128580,2.0/3.0)*t128587* \
               (2.0/3.0)-(e*gamma_bb[index]*mu_*rho_b[index]*1.0/pow(t128580,1.0E1/3.0)*1.0/(t128585*t128585)*(1.6E1/ \
               3.0))/k;
        } else if (rho_a[index] > cutoff_) {
            v_rho_b_[index] = 0.0;
        } else if (rho_b[index] > cutoff_) {
            double t128665 = rho_b[index]*2.0;
            double t128666 = 1.0/(k*k);
            double t128667 = 1.0/kp;
            double t128668 = 1.0/pow(t128665,8.0/3.0);
            double t128669 = gamma_bb[index]*mu_*t128666*t128667*t128668;
            double t128670 = t128669+1.0;
            double t128671 = 1.0/t128670;
            double t128672 = kp-kp*t128671+1.0;
            v_rho_b_[index] = e*k*t128672*pow(t128665,1.0/3.0)+e*k*rho_b[index]*t128672*1.0/pow(t128665,2.0/3.0) \
               *(2.0/3.0)-(e*gamma_bb[index]*mu_*rho_b[index]*1.0/(t128670*t128670)*1.0/pow(t128665,1.0E1/3.0)*(1.6E1/ \
               3.0))/k;
        } else {
            v_rho_b_[index] = 0.0;
        } 

        if (is_gga_) {

            //V_gamma_aa
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                double t128589 = rho_a[index]*2.0;
                v_gamma_aa_[index] = (e*mu_*rho_a[index]*1.0/pow(t128589,7.0/3.0)*1.0/pow((gamma_aa[index]*1.0/(k* \
                   k)*mu_*1.0/pow(t128589,8.0/3.0))/kp+1.0,2.0))/k;
            } else if (rho_a[index] > cutoff_) {
                double t128739 = rho_a[index]*2.0;
                v_gamma_aa_[index] = (e*mu_*rho_a[index]*1.0/pow(t128739,7.0/3.0)*1.0/pow((gamma_aa[index]*1.0/(k* \
                   k)*mu_*1.0/pow(t128739,8.0/3.0))/kp+1.0,2.0))/k;
            } else if (rho_b[index] > cutoff_) {
                v_gamma_aa_[index] = 0.0;
            } else {
                v_gamma_aa_[index] = 0.0;
            } 

            //V_gamma_ab
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_gamma_ab_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_gamma_ab_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_gamma_ab_[index] = 0.0;
            } else {
                v_gamma_ab_[index] = 0.0;
            } 

            //V_gamma_bb
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                double t128592 = rho_b[index]*2.0;
                v_gamma_bb_[index] = (e*mu_*rho_b[index]*1.0/pow(t128592,7.0/3.0)*1.0/pow((gamma_bb[index]*1.0/(k* \
                   k)*mu_*1.0/pow(t128592,8.0/3.0))/kp+1.0,2.0))/k;
            } else if (rho_a[index] > cutoff_) {
                v_gamma_bb_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                double t128676 = rho_b[index]*2.0;
                v_gamma_bb_[index] = (e*mu_*rho_b[index]*1.0/pow(t128676,7.0/3.0)*1.0/pow((gamma_bb[index]*1.0/(k* \
                   k)*mu_*1.0/pow(t128676,8.0/3.0))/kp+1.0,2.0))/k;
            } else {
                v_gamma_bb_[index] = 0.0;
            } 
        }
        if (is_meta_) {

            //V_tau_a
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_tau_a_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_tau_a_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_tau_a_[index] = 0.0;
            } else {
                v_tau_a_[index] = 0.0;
            } 

            //V_tau_a
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_tau_b_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_tau_b_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_tau_b_[index] = 0.0;
            } else {
                v_tau_b_[index] = 0.0;
            } 
        }
    }
    //Second Partials
    for (int index = 0; index < ntrue && deriv_ >= 2; index++) {

        //V_rho_a_rho_a
        if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
            double t128596 = rho_a[index]*2.0;
            double t128597 = 1.0/(k*k);
            double t128598 = 1.0/kp;
            double t128599 = 1.0/pow(t128596,8.0/3.0);
            double t128600 = gamma_aa[index]*mu_*t128597*t128598*t128599;
            double t128601 = t128600+1.0;
            double t128602 = 1.0/t128601;
            double t128603 = kp-kp*t128602+1.0;
            double t128604 = 1.0/k;
            double t128605 = 1.0/(t128601*t128601);
            v_rho_a_rho_a_[index] = e*k*t128603*1.0/pow(t128596,2.0/3.0)*(4.0/3.0)-e*k*rho_a[index]*t128603*1.0/ \
               pow(t128596,5.0/3.0)*(8.0/9.0)-e*gamma_aa[index]*mu_*t128604*t128605*1.0/pow(t128596,1.0E1/3.0)*(3.2E1/ \
               3.0)-e*(gamma_aa[index]*gamma_aa[index])*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_a[index]*rho_a[index]*rho_a[index]* \
               rho_a[index]*rho_a[index]*rho_a[index])*1.0/(t128601*t128601*t128601)*t128598*(4.0/9.0)+e*gamma_aa[index]* \
               mu_*rho_a[index]*t128604*t128605*1.0/pow(t128596,1.3E1/3.0)*3.2E1;
        } else if (rho_a[index] > cutoff_) {
            double t128745 = rho_a[index]*2.0;
            double t128746 = 1.0/(k*k);
            double t128747 = 1.0/kp;
            double t128748 = 1.0/pow(t128745,8.0/3.0);
            double t128749 = gamma_aa[index]*mu_*t128746*t128747*t128748;
            double t128750 = t128749+1.0;
            double t128751 = 1.0/t128750;
            double t128752 = kp-kp*t128751+1.0;
            double t128753 = 1.0/k;
            double t128754 = 1.0/(t128750*t128750);
            v_rho_a_rho_a_[index] = e*k*t128752*1.0/pow(t128745,2.0/3.0)*(4.0/3.0)-e*k*rho_a[index]*t128752*1.0/ \
               pow(t128745,5.0/3.0)*(8.0/9.0)-e*gamma_aa[index]*mu_*t128753*1.0/pow(t128745,1.0E1/3.0)*t128754*(3.2E1/ \
               3.0)-e*(gamma_aa[index]*gamma_aa[index])*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_a[index]*rho_a[index]*rho_a[index]* \
               rho_a[index]*rho_a[index]*rho_a[index])*1.0/(t128750*t128750*t128750)*t128747*(4.0/9.0)+e*gamma_aa[index]* \
               mu_*rho_a[index]*t128753*1.0/pow(t128745,1.3E1/3.0)*t128754*3.2E1;
        } else if (rho_b[index] > cutoff_) {
            v_rho_a_rho_a_[index] = 0.0;
        } else {
            v_rho_a_rho_a_[index] = 0.0;
        } 

        //V_rho_a_rho_b
        if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
            v_rho_a_rho_b_[index] = 0.0;
        } else if (rho_a[index] > cutoff_) {
            v_rho_a_rho_b_[index] = 0.0;
        } else if (rho_b[index] > cutoff_) {
            v_rho_a_rho_b_[index] = 0.0;
        } else {
            v_rho_a_rho_b_[index] = 0.0;
        } 

        //V_rho_b_rho_b
        if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
            double t128608 = rho_b[index]*2.0;
            double t128609 = 1.0/(k*k);
            double t128610 = 1.0/kp;
            double t128611 = 1.0/pow(t128608,8.0/3.0);
            double t128612 = gamma_bb[index]*mu_*t128610*t128611*t128609;
            double t128613 = t128612+1.0;
            double t128614 = 1.0/t128613;
            double t128615 = kp-kp*t128614+1.0;
            double t128616 = 1.0/k;
            double t128617 = 1.0/(t128613*t128613);
            v_rho_b_rho_b_[index] = e*k*t128615*1.0/pow(t128608,2.0/3.0)*(4.0/3.0)-e*k*rho_b[index]*t128615*1.0/ \
               pow(t128608,5.0/3.0)*(8.0/9.0)-e*gamma_bb[index]*mu_*t128616*1.0/pow(t128608,1.0E1/3.0)*t128617*(3.2E1/ \
               3.0)-e*(gamma_bb[index]*gamma_bb[index])*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_b[index]*rho_b[index]*rho_b[index]* \
               rho_b[index]*rho_b[index]*rho_b[index])*t128610*1.0/(t128613*t128613*t128613)*(4.0/9.0)+e*gamma_bb[index]* \
               mu_*rho_b[index]*t128616*1.0/pow(t128608,1.3E1/3.0)*t128617*3.2E1;
        } else if (rho_a[index] > cutoff_) {
            v_rho_b_rho_b_[index] = 0.0;
        } else if (rho_b[index] > cutoff_) {
            double t128682 = rho_b[index]*2.0;
            double t128683 = 1.0/(k*k);
            double t128684 = 1.0/kp;
            double t128685 = 1.0/pow(t128682,8.0/3.0);
            double t128686 = gamma_bb[index]*mu_*t128683*t128684*t128685;
            double t128687 = t128686+1.0;
            double t128688 = 1.0/t128687;
            double t128689 = kp-kp*t128688+1.0;
            double t128690 = 1.0/k;
            double t128691 = 1.0/(t128687*t128687);
            v_rho_b_rho_b_[index] = e*k*1.0/pow(t128682,2.0/3.0)*t128689*(4.0/3.0)-e*k*rho_b[index]*1.0/pow(t128682,5.0/ \
               3.0)*t128689*(8.0/9.0)-e*gamma_bb[index]*mu_*t128690*1.0/pow(t128682,1.0E1/3.0)*t128691*(3.2E1/3.0) \
               -e*(gamma_bb[index]*gamma_bb[index])*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_b[index]*rho_b[index]*rho_b[index]* \
               rho_b[index]*rho_b[index]*rho_b[index])*t128684*1.0/(t128687*t128687*t128687)*(4.0/9.0)+e*gamma_bb[index]* \
               mu_*rho_b[index]*t128690*1.0/pow(t128682,1.3E1/3.0)*t128691*3.2E1;
        } else {
            v_rho_b_rho_b_[index] = 0.0;
        } 

        if (is_gga_) {

            //V_rho_a_gamma_aa
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                double t128619 = rho_a[index]*2.0;
                double t128620 = 1.0/k;
                double t128621 = 1.0/(k*k);
                double t128622 = 1.0/kp;
                double t128623 = 1.0/pow(t128619,8.0/3.0);
                double t128624 = gamma_aa[index]*mu_*t128621*t128622*t128623;
                double t128625 = t128624+1.0;
                double t128626 = 1.0/(t128625*t128625);
                v_rho_a_gamma_aa_[index] = e*mu_*t128620*t128626*1.0/pow(t128619,7.0/3.0)-e*mu_*rho_a[index]*t128620* \
                   t128626*1.0/pow(t128619,1.0E1/3.0)*(1.4E1/3.0)+e*gamma_aa[index]*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_a[index]* \
                   rho_a[index]*rho_a[index]*rho_a[index]*rho_a[index])*t128622*1.0/(t128625*t128625*t128625)*(1.0/6.0) \
                   ;
            } else if (rho_a[index] > cutoff_) {
                double t128758 = rho_a[index]*2.0;
                double t128759 = 1.0/k;
                double t128760 = 1.0/(k*k);
                double t128761 = 1.0/kp;
                double t128762 = 1.0/pow(t128758,8.0/3.0);
                double t128763 = gamma_aa[index]*mu_*t128760*t128761*t128762;
                double t128764 = t128763+1.0;
                double t128765 = 1.0/(t128764*t128764);
                v_rho_a_gamma_aa_[index] = e*mu_*t128765*1.0/pow(t128758,7.0/3.0)*t128759-e*mu_*rho_a[index]*t128765* \
                   1.0/pow(t128758,1.0E1/3.0)*t128759*(1.4E1/3.0)+e*gamma_aa[index]*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_a[index]* \
                   rho_a[index]*rho_a[index]*rho_a[index]*rho_a[index])*t128761*1.0/(t128764*t128764*t128764)*(1.0/6.0) \
                   ;
            } else if (rho_b[index] > cutoff_) {
                v_rho_a_gamma_aa_[index] = 0.0;
            } else {
                v_rho_a_gamma_aa_[index] = 0.0;
            } 

            //V_rho_a_gamma_ab
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_rho_a_gamma_ab_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_rho_a_gamma_ab_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_rho_a_gamma_ab_[index] = 0.0;
            } else {
                v_rho_a_gamma_ab_[index] = 0.0;
            } 

            //V_rho_a_gamma_bb
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_rho_a_gamma_bb_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_rho_a_gamma_bb_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_rho_a_gamma_bb_[index] = 0.0;
            } else {
                v_rho_a_gamma_bb_[index] = 0.0;
            } 

            //V_rho_b_gamma_aa
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_rho_b_gamma_aa_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_rho_b_gamma_aa_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_rho_b_gamma_aa_[index] = 0.0;
            } else {
                v_rho_b_gamma_aa_[index] = 0.0;
            } 

            //V_rho_b_gamma_ab
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_rho_b_gamma_ab_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_rho_b_gamma_ab_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_rho_b_gamma_ab_[index] = 0.0;
            } else {
                v_rho_b_gamma_ab_[index] = 0.0;
            } 

            //V_rho_b_gamma_bb
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                double t128632 = rho_b[index]*2.0;
                double t128633 = 1.0/k;
                double t128634 = 1.0/(k*k);
                double t128635 = 1.0/kp;
                double t128636 = 1.0/pow(t128632,8.0/3.0);
                double t128637 = gamma_bb[index]*mu_*t128634*t128635*t128636;
                double t128638 = t128637+1.0;
                double t128639 = 1.0/(t128638*t128638);
                v_rho_b_gamma_bb_[index] = e*mu_*1.0/pow(t128632,7.0/3.0)*t128633*t128639-e*mu_*rho_b[index]*1.0/ \
                   pow(t128632,1.0E1/3.0)*t128633*t128639*(1.4E1/3.0)+e*gamma_bb[index]*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_b[index]* \
                   rho_b[index]*rho_b[index]*rho_b[index]*rho_b[index])*t128635*1.0/(t128638*t128638*t128638)*(1.0/6.0) \
                   ;
            } else if (rho_a[index] > cutoff_) {
                v_rho_b_gamma_bb_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                double t128698 = rho_b[index]*2.0;
                double t128699 = 1.0/k;
                double t128700 = 1.0/(k*k);
                double t128701 = 1.0/kp;
                double t128702 = 1.0/pow(t128698,8.0/3.0);
                double t128703 = gamma_bb[index]*mu_*t128700*t128701*t128702;
                double t128704 = t128703+1.0;
                double t128705 = 1.0/(t128704*t128704);
                v_rho_b_gamma_bb_[index] = e*mu_*t128705*1.0/pow(t128698,7.0/3.0)*t128699-e*mu_*rho_b[index]*t128705* \
                   1.0/pow(t128698,1.0E1/3.0)*t128699*(1.4E1/3.0)+e*gamma_bb[index]*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_b[index]* \
                   rho_b[index]*rho_b[index]*rho_b[index]*rho_b[index])*t128701*1.0/(t128704*t128704*t128704)*(1.0/6.0) \
                   ;
            } else {
                v_rho_b_gamma_bb_[index] = 0.0;
            } 

            //V_gamma_aa_gamma_aa
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                double t128641 = 1.0/kp;
                v_gamma_aa_gamma_aa_[index] = e*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_a[index]*rho_a[index]*rho_a[index]* \
                   rho_a[index])*t128641*1.0/pow(gamma_aa[index]*1.0/(k*k)*mu_*t128641*1.0/pow(rho_a[index]*2.0,8.0/3.0) \
                   +1.0,3.0)*(-1.0/1.6E1);
            } else if (rho_a[index] > cutoff_) {
                double t128772 = 1.0/kp;
                v_gamma_aa_gamma_aa_[index] = e*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_a[index]*rho_a[index]*rho_a[index]* \
                   rho_a[index])*t128772*1.0/pow(gamma_aa[index]*1.0/(k*k)*mu_*t128772*1.0/pow(rho_a[index]*2.0,8.0/3.0) \
                   +1.0,3.0)*(-1.0/1.6E1);
            } else if (rho_b[index] > cutoff_) {
                v_gamma_aa_gamma_aa_[index] = 0.0;
            } else {
                v_gamma_aa_gamma_aa_[index] = 0.0;
            } 

            //V_gamma_aa_gamma_ab
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_gamma_aa_gamma_ab_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_gamma_aa_gamma_ab_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_gamma_aa_gamma_ab_[index] = 0.0;
            } else {
                v_gamma_aa_gamma_ab_[index] = 0.0;
            } 

            //V_gamma_aa_gamma_bb
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_gamma_aa_gamma_bb_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_gamma_aa_gamma_bb_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_gamma_aa_gamma_bb_[index] = 0.0;
            } else {
                v_gamma_aa_gamma_bb_[index] = 0.0;
            } 

            //V_gamma_ab_gamma_ab
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_gamma_ab_gamma_ab_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_gamma_ab_gamma_ab_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_gamma_ab_gamma_ab_[index] = 0.0;
            } else {
                v_gamma_ab_gamma_ab_[index] = 0.0;
            } 

            //V_gamma_ab_gamma_bb
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_gamma_ab_gamma_bb_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_gamma_ab_gamma_bb_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_gamma_ab_gamma_bb_[index] = 0.0;
            } else {
                v_gamma_ab_gamma_bb_[index] = 0.0;
            } 

            //V_gamma_bb_gamma_bb
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                double t128647 = 1.0/kp;
                v_gamma_bb_gamma_bb_[index] = e*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_b[index]*rho_b[index]*rho_b[index]* \
                   rho_b[index])*t128647*1.0/pow(gamma_bb[index]*1.0/(k*k)*mu_*t128647*1.0/pow(rho_b[index]*2.0,8.0/3.0) \
                   +1.0,3.0)*(-1.0/1.6E1);
            } else if (rho_a[index] > cutoff_) {
                v_gamma_bb_gamma_bb_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                double t128712 = 1.0/kp;
                v_gamma_bb_gamma_bb_[index] = e*1.0/(k*k*k)*(mu_*mu_)*1.0/(rho_b[index]*rho_b[index]*rho_b[index]* \
                   rho_b[index])*t128712*1.0/pow(gamma_bb[index]*1.0/(k*k)*mu_*t128712*1.0/pow(rho_b[index]*2.0,8.0/3.0) \
                   +1.0,3.0)*(-1.0/1.6E1);
            } else {
                v_gamma_bb_gamma_bb_[index] = 0.0;
            } 

        }
        if (is_meta_) {

            //V_rho_a_tau_a
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_rho_a_tau_a_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_rho_a_tau_a_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_rho_a_tau_a_[index] = 0.0;
            } else {
                v_rho_a_tau_a_[index] = 0.0;
            } 

            //V_rho_a_tau_b
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_rho_a_tau_b_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_rho_a_tau_b_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_rho_a_tau_b_[index] = 0.0;
            } else {
                v_rho_a_tau_b_[index] = 0.0;
            } 

            //V_rho_b_tau_a
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_rho_b_tau_a_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_rho_b_tau_a_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_rho_b_tau_a_[index] = 0.0;
            } else {
                v_rho_b_tau_a_[index] = 0.0;
            } 

            //V_rho_b_tau_b
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_rho_b_tau_b_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_rho_b_tau_b_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_rho_b_tau_b_[index] = 0.0;
            } else {
                v_rho_b_tau_b_[index] = 0.0;
            } 

            //V_tau_a_tau_a
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_tau_a_tau_a_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_tau_a_tau_a_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_tau_a_tau_a_[index] = 0.0;
            } else {
                v_tau_a_tau_a_[index] = 0.0;
            } 

            //V_tau_a_tau_b
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_tau_a_tau_b_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_tau_a_tau_b_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_tau_a_tau_b_[index] = 0.0;
            } else {
                v_tau_a_tau_b_[index] = 0.0;
            } 

            //V_tau_b_tau_b
            if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                v_tau_b_tau_b_[index] = 0.0;
            } else if (rho_a[index] > cutoff_) {
                v_tau_b_tau_b_[index] = 0.0;
            } else if (rho_b[index] > cutoff_) {
                v_tau_b_tau_b_[index] = 0.0;
            } else {
                v_tau_b_tau_b_[index] = 0.0;
            } 

            if (is_gga_) {

                //V_gamma_aa_tau_a
                if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                    v_gamma_aa_tau_a_[index] = 0.0;
                } else if (rho_a[index] > cutoff_) {
                    v_gamma_aa_tau_a_[index] = 0.0;
                } else if (rho_b[index] > cutoff_) {
                    v_gamma_aa_tau_a_[index] = 0.0;
                } else {
                    v_gamma_aa_tau_a_[index] = 0.0;
                } 

                //V_gamma_aa_tau_b
                if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                    v_gamma_aa_tau_b_[index] = 0.0;
                } else if (rho_a[index] > cutoff_) {
                    v_gamma_aa_tau_b_[index] = 0.0;
                } else if (rho_b[index] > cutoff_) {
                    v_gamma_aa_tau_b_[index] = 0.0;
                } else {
                    v_gamma_aa_tau_b_[index] = 0.0;
                } 

                //V_gamma_ab_tau_a
                if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                    v_gamma_ab_tau_a_[index] = 0.0;
                } else if (rho_a[index] > cutoff_) {
                    v_gamma_ab_tau_a_[index] = 0.0;
                } else if (rho_b[index] > cutoff_) {
                    v_gamma_ab_tau_a_[index] = 0.0;
                } else {
                    v_gamma_ab_tau_a_[index] = 0.0;
                } 

                //V_gamma_ab_tau_b
                if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                    v_gamma_ab_tau_b_[index] = 0.0;
                } else if (rho_a[index] > cutoff_) {
                    v_gamma_ab_tau_b_[index] = 0.0;
                } else if (rho_b[index] > cutoff_) {
                    v_gamma_ab_tau_b_[index] = 0.0;
                } else {
                    v_gamma_ab_tau_b_[index] = 0.0;
                } 

                //V_gamma_bb_tau_a
                if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                    v_gamma_bb_tau_a_[index] = 0.0;
                } else if (rho_a[index] > cutoff_) {
                    v_gamma_bb_tau_a_[index] = 0.0;
                } else if (rho_b[index] > cutoff_) {
                    v_gamma_bb_tau_a_[index] = 0.0;
                } else {
                    v_gamma_bb_tau_a_[index] = 0.0;
                } 

                //V_gamma_bb_tau_b
                if (rho_a[index] > cutoff_ && rho_b[index] > cutoff_) {
                    v_gamma_bb_tau_b_[index] = 0.0;
                } else if (rho_a[index] > cutoff_) {
                    v_gamma_bb_tau_b_[index] = 0.0;
                } else if (rho_b[index] > cutoff_) {
                    v_gamma_bb_tau_b_[index] = 0.0;
                } else {
                    v_gamma_bb_tau_b_[index] = 0.0;
                } 

            }
        }
    }
}

}}
