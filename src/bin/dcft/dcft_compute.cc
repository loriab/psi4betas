#include "dcft.h"
#include <cmath>
#include <libdpd/dpd.h>
#include <libtrans/integraltransform.h>
#include <libdiis/diismanager.h>
#include <libpsio/psio.hpp>
#include "defines.h"

using namespace boost;

namespace psi{ namespace dcft{

/**
 * Computes the DCFT density matrix and energy
 */
double
DCFTSolver::compute_energy()
{

    bool scfDone    = false;
    bool lambdaDone = false;
    bool densityConverged = false;
    bool energyConverged = false;
    scf_guess();
    mp2_guess();

    int cycle = 0;
    fprintf(outfile, "\n\n\t*=================================================================================*\n"
                     "\t* Cycle  RMS [F, Kappa]   RMS Lambda Error   delta E        Total Energy     DIIS *\n"
                     "\t*---------------------------------------------------------------------------------*\n");
    // This is the two-step update - in each macro iteration, update the orbitals first, then update lambda
    // to self-consistency, until converged.  When lambda is converged and only one scf cycle is needed to reach
    // the desired cutoff, we're done


    if(options_.get_str("ALGORITHM") == "TWOSTEP"){
        SharedMatrix tmp = SharedMatrix(new Matrix("temp", nirrep_, nsopi_, nsopi_));
        // Set up the DIIS manager for the density cumulant and SCF iterations
        dpdbuf4 Laa, Lab, Lbb;
        dpd_buf4_init(&Laa, PSIF_LIBTRANS_DPD, 0, ID("[O>O]-"), ID("[V>V]-"),
                      ID("[O>O]-"), ID("[V>V]-"), 0, "Lambda <OO|VV>");
        dpd_buf4_init(&Lab, PSIF_LIBTRANS_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                      ID("[O,o]"), ID("[V,v]"), 0, "Lambda <Oo|Vv>");
        dpd_buf4_init(&Lbb, PSIF_LIBTRANS_DPD, 0, ID("[o>o]-"), ID("[v>v]-"),
                      ID("[o>o]-"), ID("[v>v]-"), 0, "Lambda <oo|vv>");
        DIISManager scfDiisManager(maxdiis_, "DCFT DIIS Orbitals",DIISManager::LargestError,DIISManager::InCore);
        scfDiisManager.set_error_vector_size(2, DIISEntry::Matrix, scf_error_a_.get(),
                                                DIISEntry::Matrix, scf_error_b_.get());
        scfDiisManager.set_vector_size(2, DIISEntry::Matrix, Fa_.get(),
                                          DIISEntry::Matrix, Fb_.get());
        DIISManager lambdaDiisManager(maxdiis_, "DCFT DIIS Lambdas",DIISManager::LargestError,DIISManager::InCore);
        lambdaDiisManager.set_error_vector_size(3, DIISEntry::DPDBuf4, &Laa,
                                                   DIISEntry::DPDBuf4, &Lab,
                                                   DIISEntry::DPDBuf4, &Lbb);
        lambdaDiisManager.set_vector_size(3, DIISEntry::DPDBuf4, &Laa,
                                             DIISEntry::DPDBuf4, &Lab,
                                             DIISEntry::DPDBuf4, &Lbb);
        dpd_buf4_close(&Laa);
        dpd_buf4_close(&Lab);
        dpd_buf4_close(&Lbb);
        old_ca_->copy(Ca_);
        old_cb_->copy(Cb_);
        // Save F0 = H + G * Kappa for the Fock intermediate update in lambda iterations
        F0a_->copy(Fa_);
        F0b_->copy(Fb_);
        F0a_->transform(Ca_);
        F0b_->transform(Cb_);
        // Just so the correct value is printed in the first macro iteration
        scf_convergence_ = compute_scf_error_vector();
        // Start macro-iterations
        while((!scfDone || !lambdaDone) && cycle++ < maxiter_){
            fprintf(outfile, "\t                          *** Macro Iteration %d ***\n"
                             "\tCumulant Iterations\n",cycle);
            // If it's the first iteration and the user requested to relax guess orbitals, then skip the density cumulant update
            if ((cycle != 1) || !options_.get_bool("RELAX_GUESS_ORBITALS")) {
                lambdaDone = false;
                int nLambdaIterations = 0;
                lambdaDiisManager.reset_subspace();
                // Start density cumulant (lambda) iterations
                while((!lambdaDone || !energyConverged) && nLambdaIterations++ < options_.get_int("LAMBDA_MAXITER")){
                    std::string diisString;
                    // Build new Tau from current Lambda
                    if (options_.get_bool("RELAX_TAU")) {
                        build_tau();
                        if (options_.get_str("AO_BASIS") == "DISK") {
                            // Transform new Tau to the SO basis
                            transform_tau();
                            // Build SO basis tensors for the <VV||VV>, <vv||vv>, and <Vv|Vv> terms in the G intermediate
                            build_tensors();
                        }
                        else {
                            // Compute GTau contribution for the Fock operator
                            build_gtau();
                        }
                        // Update Fock operator for the F intermediate
                        update_fock();
                    }
                    else {
                        if (options_.get_str("AO_BASIS") == "DISK") {
                            // Build SO basis tensors for the <VV||VV>, <vv||vv>, and <Vv|Vv> terms in the G intermediate
                            build_tensors();
                        }
                    }
                    // Build G and F intermediates needed for the density cumulant residual equations and DCFT energy computation
                    build_intermediates();
                    // Compute the residuals for density cumulant equations
                    lambda_convergence_ = compute_lambda_residual();
                    // Update density cumulant tensor
                    update_lambda_from_residual();
                    if(lambda_convergence_ < diis_start_thresh_){
                        //Store the DIIS vectors
                        dpdbuf4 Laa, Lab, Lbb, Raa, Rab, Rbb;
                        dpd_buf4_init(&Raa, PSIF_DCFT_DPD, 0, ID("[O>O]-"), ID("[V>V]-"),
                                      ID("[O>O]-"), ID("[V>V]-"), 0, "R <OO|VV>");
                        dpd_buf4_init(&Rab, PSIF_DCFT_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                                      ID("[O,o]"), ID("[V,v]"), 0, "R <Oo|Vv>");
                        dpd_buf4_init(&Rbb, PSIF_DCFT_DPD, 0, ID("[o>o]-"), ID("[v>v]-"),
                                      ID("[o>o]-"), ID("[v>v]-"), 0, "R <oo|vv>");
                        dpd_buf4_init(&Laa, PSIF_DCFT_DPD, 0, ID("[O>O]-"), ID("[V>V]-"),
                                      ID("[O>O]-"), ID("[V>V]-"), 0, "Lambda <OO|VV>");
                        dpd_buf4_init(&Lab, PSIF_DCFT_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                                      ID("[O,o]"), ID("[V,v]"), 0, "Lambda <Oo|Vv>");
                        dpd_buf4_init(&Lbb, PSIF_DCFT_DPD, 0, ID("[o>o]-"), ID("[v>v]-"),
                                      ID("[o>o]-"), ID("[v>v]-"), 0, "Lambda <oo|vv>");

                        if(lambdaDiisManager.add_entry(6, &Raa, &Rab, &Rbb, &Laa, &Lab, &Lbb)){
                            diisString += "S";
                        }
                        if(lambdaDiisManager.subspace_size() >= mindiisvecs_ && maxdiis_ > 0){
                            diisString += "/E";
                            lambdaDiisManager.extrapolate(3, &Laa, &Lab, &Lbb);
                        }
                        dpd_buf4_close(&Raa);
                        dpd_buf4_close(&Rab);
                        dpd_buf4_close(&Rbb);
                        dpd_buf4_close(&Laa);
                        dpd_buf4_close(&Lab);
                        dpd_buf4_close(&Lbb);
                    }
                    // Save old DCFT energy
                    old_total_energy_ = new_total_energy_;
                    // Compute new DCFT energy (lambda contribution)
                    compute_dcft_energy();
                    new_total_energy_ = scf_energy_ + lambda_energy_;
                    // Check convergence for density cumulant iterations
                    lambdaDone = lambda_convergence_ < lambda_threshold_;
                    energyConverged = fabs(new_total_energy_ - old_total_energy_) < lambda_threshold_;
                    fprintf(outfile, "\t* %-3d   %12.3e      %12.3e   %12.3e  %21.15f  %-3s *\n",
                            nLambdaIterations, scf_convergence_, lambda_convergence_, new_total_energy_ - old_total_energy_,
                            new_total_energy_, diisString.c_str());
                    if (fabs(lambda_convergence_) > 100.0) throw PSIEXCEPTION("DCFT density cumulant equations diverged");
                    fflush(outfile);
                }
            }
            else fprintf(outfile, "\tSkipping the cumulant update to relax guess orbitals\n");
            // Build new Tau from the density cumulant in the MO basis and transform it the SO basis
            build_tau();
            transform_tau();
            // Update the orbitals
            int nSCFCycles = 0;
            // Reset the booleans that control the convergence
            densityConverged = false;
            energyConverged = false;
            scfDiisManager.reset_subspace();
            fprintf(outfile, "\tOrbital Updates\n");
            while((!densityConverged || !scfDone || !energyConverged) && nSCFCycles++ < options_.get_int("SCF_MAXITER")){
                std::string diisString;
                // Copy core hamiltonian into the Fock matrix array: F = H
                Fa_->copy(so_h_);
                Fb_->copy(so_h_);
                // Build the new Fock matrix from the SO integrals: F += Gbar * Kappa
                process_so_ints();
                // Save F0 = H + G * Kappa for the Fock intermediate update in lambda iterations
                F0a_->copy(Fa_);
                F0b_->copy(Fb_);
                F0a_->transform(Ca_);
                F0b_->transform(Cb_);
                // Save old SCF energy
                old_total_energy_ = new_total_energy_;
                // Add non-idempotent density contribution (Tau) to the Fock matrix: F += Gbar * Tau
                Fa_->add(g_tau_a_);
                Fb_->add(g_tau_b_);
                // Back up the SO basis Fock before it is symmetrically orthogonalized to transform it to the MO basis
                moFa_->copy(Fa_);
                moFb_->copy(Fb_);
                // Compute new SCF energy
                compute_scf_energy();
                // Check SCF convergence
                scf_convergence_ = compute_scf_error_vector();
                scfDone = scf_convergence_ < scf_threshold_;
                if(scf_convergence_ < diis_start_thresh_){
                    if(scfDiisManager.add_entry(4, scf_error_a_.get(), scf_error_b_.get(), Fa_.get(), Fb_.get()))
                        diisString += "S";
                }
                if(scfDiisManager.subspace_size() > mindiisvecs_){
                    diisString += "/E";
                    scfDiisManager.extrapolate(2, Fa_.get(), Fb_.get());
                }
                // Save the Fock matrix before the symmetric orthogonalization for Tau^2 correction computation
                Fa_copy->copy(Fa_);
                Fb_copy->copy(Fb_);
                // Transform the Fock matrix to the symmetrically orhogonalized basis set and digonalize it
                // Obtain new orbitals
                Fa_->transform(s_half_inv_);
                Fa_->diagonalize(tmp, epsilon_a_);
                old_ca_->copy(Ca_);
                Ca_->gemm(false, false, 1.0, s_half_inv_, tmp, 0.0);
                Fb_->transform(s_half_inv_);
                Fb_->diagonalize(tmp, epsilon_b_);
                old_cb_->copy(Cb_);
                Cb_->gemm(false, false, 1.0, s_half_inv_, tmp, 0.0);
                // Make sure that the orbital phase is retained
                correct_mo_phases(false);
                // Find occupation. It shouldn't be called, at least in the current implementation
                if(!lock_occupation_) find_occupation(epsilon_a_, epsilon_b_);
                // Update SCF density (Kappa) and check its RMS
                densityConverged = update_scf_density() < scf_threshold_;
                // Compute the DCFT energy
                new_total_energy_ = scf_energy_ + lambda_energy_;
                // Check convergence of the total DCFT energy
                energyConverged = fabs(new_total_energy_ - old_total_energy_) < lambda_threshold_;
                fprintf(outfile, "\t* %-3d   %12.3e      %12.3e   %12.3e  %21.15f  %-3s *\n",
                    nSCFCycles, scf_convergence_, lambda_convergence_, new_total_energy_ - old_total_energy_,
                    new_total_energy_, diisString.c_str());
                if (fabs(scf_convergence_) > 100.0) throw PSIEXCEPTION("DCFT orbital updates diverged");
                fflush(outfile);
            }
            // Write orbitals to the checkpoint file
            write_orbitals_to_checkpoint();
            scfDone = nSCFCycles == 1;
            energyConverged = false;
            // Transform the Fock matrix to the MO basis
            moFa_->transform(Ca_);
            moFb_->transform(Cb_);
            // Transform two-electron integrals to the MO basis using new orbitals, build denominators
            transform_integrals();
        }
    }
    // This is the simultaneous orbital/lambda update algorithm
    else if (options_.get_str("ALGORITHM") == "SIMULTANEOUS"){

        SharedMatrix tmp = SharedMatrix(new Matrix("temp", nirrep_, nsopi_, nsopi_));
        // Set up the DIIS manager
        DIISManager diisManager(maxdiis_, "DCFT DIIS vectors");
        dpdbuf4 Laa, Lab, Lbb;
        dpd_buf4_init(&Laa, PSIF_LIBTRANS_DPD, 0, ID("[O>O]-"), ID("[V>V]-"),
                      ID("[O>O]-"), ID("[V>V]-"), 0, "Lambda <OO|VV>");
        dpd_buf4_init(&Lab, PSIF_LIBTRANS_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                      ID("[O,o]"), ID("[V,v]"), 0, "Lambda <Oo|Vv>");
        dpd_buf4_init(&Lbb, PSIF_LIBTRANS_DPD, 0, ID("[o>o]-"), ID("[v>v]-"),
                      ID("[o>o]-"), ID("[v>v]-"), 0, "Lambda <oo|vv>");
        diisManager.set_error_vector_size(5, DIISEntry::Matrix, scf_error_a_.get(),
                                             DIISEntry::Matrix, scf_error_b_.get(),
                                             DIISEntry::DPDBuf4, &Laa,
                                             DIISEntry::DPDBuf4, &Lab,
                                             DIISEntry::DPDBuf4, &Lbb);
        diisManager.set_vector_size(5, DIISEntry::Matrix, Fa_.get(),
                                       DIISEntry::Matrix, Fb_.get(),
                                       DIISEntry::DPDBuf4, &Laa,
                                       DIISEntry::DPDBuf4, &Lab,
                                       DIISEntry::DPDBuf4, &Lbb);
        dpd_buf4_close(&Laa);
        dpd_buf4_close(&Lab);
        dpd_buf4_close(&Lbb);
        while((!scfDone || !lambdaDone || !densityConverged || !energyConverged)
                && cycle++ < maxiter_){
            std::string diisString;
            // Save the old energy
            old_total_energy_ = new_total_energy_;
            // Build new Tau from the density cumulant in the MO basis and transform it the SO basis
            build_tau();
            transform_tau();
            // Copy core hamiltonian into the Fock matrix array: F = H
            Fa_->copy(so_h_);
            Fb_->copy(so_h_);
            // Build the new Fock matrix from the SO integrals: F += Gbar * Kappa
            process_so_ints();
            // Add non-idempotent density contribution (Tau) to the Fock matrix: F += Gbar * Tau
            Fa_->add(g_tau_a_);
            Fb_->add(g_tau_b_);
            // Back up the SO basis Fock before it is symmetrically orthogonalized to transform it to the MO basis
            moFa_->copy(Fa_);
            moFb_->copy(Fb_);
            // Transform the Fock matrix to the MO basis
            moFa_->transform(Ca_);
            moFb_->transform(Cb_);
            // Compute new SCF energy
            compute_scf_energy();
            // Add SCF energy contribution to the total DCFT energy
            new_total_energy_ = scf_energy_;
            // Check SCF convergence
            scf_convergence_ = compute_scf_error_vector();
            scfDone = scf_convergence_ < scf_threshold_;
            // Build G and F intermediates needed for the density cumulant residual equations and DCFT energy computation
            build_intermediates();
            // Compute the residuals for density cumulant equations
            lambda_convergence_ = compute_lambda_residual();
            if (fabs(lambda_convergence_) > 100.0) throw PSIEXCEPTION("DCFT density cumulant equations diverged");
            // Check convergence for density cumulant iterations
            lambdaDone = lambda_convergence_ < lambda_threshold_;
            // Update density cumulant tensor
            update_lambda_from_residual();
            // Compute new DCFT energy (lambda contribution)
            compute_dcft_energy();
            // Add lambda energy to the DCFT total energy
            new_total_energy_ += lambda_energy_;
            // Check convergence of the total DCFT energy
            energyConverged = fabs(old_total_energy_ - new_total_energy_) < lambda_threshold_;
            if(scf_convergence_ < diis_start_thresh_ && lambda_convergence_ < diis_start_thresh_){
                //Store the DIIS vectors
                dpdbuf4 Laa, Lab, Lbb, Raa, Rab, Rbb;
                dpd_buf4_init(&Raa, PSIF_DCFT_DPD, 0, ID("[O>O]-"), ID("[V>V]-"),
                              ID("[O>O]-"), ID("[V>V]-"), 0, "R <OO|VV>");
                dpd_buf4_init(&Rab, PSIF_DCFT_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                              ID("[O,o]"), ID("[V,v]"), 0, "R <Oo|Vv>");
                dpd_buf4_init(&Rbb, PSIF_DCFT_DPD, 0, ID("[o>o]-"), ID("[v>v]-"),
                              ID("[o>o]-"), ID("[v>v]-"), 0, "R <oo|vv>");
                dpd_buf4_init(&Laa, PSIF_DCFT_DPD, 0, ID("[O>O]-"), ID("[V>V]-"),
                              ID("[O>O]-"), ID("[V>V]-"), 0, "Lambda <OO|VV>");
                dpd_buf4_init(&Lab, PSIF_DCFT_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                              ID("[O,o]"), ID("[V,v]"), 0, "Lambda <Oo|Vv>");
                dpd_buf4_init(&Lbb, PSIF_DCFT_DPD, 0, ID("[o>o]-"), ID("[v>v]-"),
                              ID("[o>o]-"), ID("[v>v]-"), 0, "Lambda <oo|vv>");
                if(diisManager.add_entry(10, scf_error_a_.get(), scf_error_b_.get(), &Raa, &Rab, &Rbb,
                                           Fa_.get(), Fb_.get(), &Laa, &Lab, &Lbb)){
                    diisString += "S";
                }
                if(diisManager.subspace_size() > mindiisvecs_){
                    diisString += "/E";
                    diisManager.extrapolate(5, Fa_.get(), Fb_.get(), &Laa, &Lab, &Lbb);
                }
                dpd_buf4_close(&Raa);
                dpd_buf4_close(&Rab);
                dpd_buf4_close(&Rbb);
                dpd_buf4_close(&Laa);
                dpd_buf4_close(&Lab);
                dpd_buf4_close(&Lbb);
            }

            // Save the Fock matrix before the symmetric orthogonalization for Tau^2 correction computation
            Fa_copy->copy(Fa_);
            Fb_copy->copy(Fb_);
            // Transform the Fock matrix to the symmetrically orhogonalized basis set and digonalize it
            // Obtain new orbitals
            Fa_->transform(s_half_inv_);
            Fa_->diagonalize(tmp, epsilon_a_);
            old_ca_->copy(Ca_);
            Ca_->gemm(false, false, 1.0, s_half_inv_, tmp, 0.0);
            Fb_->transform(s_half_inv_);
            Fb_->diagonalize(tmp, epsilon_b_);
            old_cb_->copy(Cb_);
            Cb_->gemm(false, false, 1.0, s_half_inv_, tmp, 0.0);
            // Make sure that the orbital phase is retained
            if(!correct_mo_phases(false)){
                fprintf(outfile,"\t\tThere was a problem correcting the MO phases.\n"
                                "\t\tIf this does not converge, try ALGORITHM=TWOSTEP\n");
            }
            // Write orbitals to the checkpoint file
            write_orbitals_to_checkpoint();
            // Transform two-electron integrals to the MO basis using new orbitals, build denominators
            transform_integrals();
            // Find occupation. It shouldn't be called, at least in the current implementation
            if(!lock_occupation_) find_occupation(epsilon_a_, epsilon_b_);
            // Update SCF density (Kappa) and check its RMS
            densityConverged = update_scf_density() < scf_threshold_;
            // If we've performed enough lambda updates since the last orbitals
            // update, reset the counter so another SCF update is performed
            fprintf(outfile, "\t* %-3d   %12.3e      %12.3e   %12.3e  %21.15f  %-3s *\n",
                    cycle, scf_convergence_, lambda_convergence_, new_total_energy_ - old_total_energy_,
                    new_total_energy_, diisString.c_str());
            fflush(outfile);
        }
    }
    // Quadratically-convergent algorithm: solution of the Newton-Raphson equations
    // for the simultaneous optimization of the cumulant and the orbitals
    else {
        fprintf(outfile,    "\n\n\t\t        Quadratically-Convergent DCFT      \n");
        fprintf(outfile,        "\t\t     by A.Yu. Sokolov and A.C. Simmonett   \n\n");

        if (options_.get_str("DERTYPE") == "FIRST") throw FeatureNotImplemented("QC-DCFT-06", "Analytic gradients", __FILE__, __LINE__);
        run_qc_dcft();

        scfDone = true;
        lambdaDone = true;
        densityConverged = true;
    }

    if(!scfDone || !lambdaDone || !densityConverged)
        throw ConvergenceError<int>("DCFT", maxiter_, lambda_threshold_,
                               lambda_convergence_, __FILE__, __LINE__);

    fprintf(outfile, "\t*=================================================================================*\n");

    // Computes the Tau^2 correction to Tau and the DCFT energy if requested by the user
    Fa_->copy(Fa_copy);
    Fb_->copy(Fb_copy);
    compute_tau_squared();
    compute_energy_tau_squared();

    // Tau^2 should probably be added to SCF energy....
    fprintf(outfile, "\n\t*DCFT SCF Energy                                 = %20.15f\n", scf_energy_);
    fprintf(outfile, "\t*DCFT Lambda Energy                              = %20.15f\n", lambda_energy_);
    fprintf(outfile, "\t*DCFT Total Energy                               = %20.15f\n", new_total_energy_);
    if(options_.get_bool("TAU_SQUARED")) {
        fprintf(outfile, "\t*Tau Squared Correction to DCFT Energy           = %20.15f\n", energy_tau_squared_);
        fprintf(outfile, "\t*DCFT Total Energy with Tau Squared Correction   = %20.15f\n", new_total_energy_ + energy_tau_squared_);
        new_total_energy_ += energy_tau_squared_;
    }

    Process::environment.globals["CURRENT ENERGY"] = new_total_energy_;
    Process::environment.globals["DCFT ENERGY"] = new_total_energy_;
    Process::environment.globals["DCFT SCF ENERGY"] = scf_energy_;
    Process::environment.globals["DCFT LAMBDA ENERGY"] = lambda_energy_;
    Process::environment.globals["DCFT TAU SQUARED CORRECTION"] = energy_tau_squared_;

    if(!options_.get_bool("MO_RELAX")){
        fprintf(outfile, "Warning!  The orbitals were not relaxed\n");
    }

    print_opdm();

    if(options_.get_bool("TPDM")) dump_density();
//    mulliken_charges();
//    check_n_representability();

    if(options_.get_str("DERTYPE") == "FIRST") {
        // Shut down the timers
        tstop();
        // Start the timers
        tstart();
        // Solve the response equations, compute relaxed OPDM and TPDM and dump them to disk
        compute_gradient();
    }

    if(!options_.get_bool("MO_RELAX") && options_.get_bool("IGNORE_TAU")){
        psio_->open(PSIF_LIBTRANS_DPD, PSIO_OPEN_OLD);
        /*
         * Comout the CEPA-0 correlation energy
         * E = 1/4 L_IJAB <IJ||AB>
         *        +L_IjAb <Ij|Ab>
         *    +1/4 L_ijab <ij||ab>
         */
        dpdbuf4 I, L;
        // Alpha - Alpha
        dpd_buf4_init(&I, PSIF_LIBTRANS_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                      ID("[O,O]"), ID("[V,V]"), 1, "MO Ints <OO|VV>");
        dpd_buf4_init(&L, PSIF_DCFT_DPD, 0, ID("[O,O]"), ID("[V,V]"),
                      ID("[O>O]-"), ID("[V>V]-"), 0, "Lambda <OO|VV>");
        double eAA = 0.25 * dpd_buf4_dot(&L, &I);
        dpd_buf4_close(&I);
        dpd_buf4_close(&L);

        // Alpha - Beta
        dpd_buf4_init(&I, PSIF_LIBTRANS_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                      ID("[O,o]"), ID("[V,v]"), 0, "MO Ints <Oo|Vv>");
        dpd_buf4_init(&L, PSIF_DCFT_DPD, 0, ID("[O,o]"), ID("[V,v]"),
                      ID("[O,o]"), ID("[V,v]"), 0, "Lambda <Oo|Vv>");
        double eAB = dpd_buf4_dot(&L, &I);
        dpd_buf4_close(&I);
        dpd_buf4_close(&L);

        // Beta - Beta
        dpd_buf4_init(&I, PSIF_LIBTRANS_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                      ID("[o,o]"), ID("[v,v]"), 1, "MO Ints <oo|vv>");
        dpd_buf4_init(&L, PSIF_DCFT_DPD, 0, ID("[o,o]"), ID("[v,v]"),
                      ID("[o>o]-"), ID("[v>v]-"), 0, "Lambda <oo|vv>");
        double eBB = 0.25 * dpd_buf4_dot(&L, &I);
        dpd_buf4_close(&I);
        dpd_buf4_close(&L);
        fprintf(outfile, "\t!CEPA0 Total Energy         = %20.15f\n",
                scf_energy_ + eAA + eAB + eBB);
        psio_->close(PSIF_LIBTRANS_DPD, 1);
    }

    // Free up memory and close files
    finalize();

    return(new_total_energy_);
}

}} // Namespaces

