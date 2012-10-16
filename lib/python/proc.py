from __future__ import print_function
"""Module with functions that encode the sequence of PSI module
calls for each of the *name* values of the energy(), optimize(),
response(), and frequency() function.

"""
import PsiMod
import shutil
import os
import subprocess
import re
import physconst
from molutil import *
from text import *
from procutil import *
from basislist import *
from functional import *
from optproc import *
# never import driver, wrappers, or aliases into this file


def run_dcft(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a density cumulant functional theory calculation.

    """
    optstash = OptionsState(
        ['REFERENCE'])

    # DCFT module should probably take a REFERENCE keyword with only UHF allowed value
    PsiMod.set_global_option('REFERENCE', 'UHF')
    PsiMod.scf()
    returnvalue = PsiMod.dcft()

    optstash.restore()
    return returnvalue


def run_dcft_gradient(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    DCFT gradient calculation.

    """
    optstash = OptionsState(
        ['REFERENCE'],
        ['GLOBALS', 'DERTYPE'])

    PsiMod.set_global_option('DERTYPE', 'FIRST')
    run_dcft(name, **kwargs)
    PsiMod.deriv()

    optstash.restore()


def run_omp2(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    an orbital-optimized MP2 computation

    """
    PsiMod.scf()
    return PsiMod.omp2()


def run_scs_omp2(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a spin-component scaled OMP2 computation

    """
    # Get calls method
    lowername = name.lower()
 
    # what type of scs?
    if (lowername == 'scs-omp2'):
        PsiMod.set_local_option('OMP2', 'SCS_TYPE', 'SCS')
    elif (lowername == 'scsn-omp2'):
        PsiMod.set_local_option('OMP2', 'SCS_TYPE', 'SCSN')
    elif (lowername == 'scs-mi-omp2'):
        PsiMod.set_local_option('OMP2', 'SCS_TYPE', 'SCSMI')
    elif (lowername == 'scs-omp2-vdw'):
        PsiMod.set_local_option('OMP2', 'SCS_TYPE', 'SCSVDW')

    PsiMod.scf()
    PsiMod.set_local_option('OMP2', 'DO_SCS', 'TRUE')
    return PsiMod.omp2()


def run_sos_omp2(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a spin-opposite scaled OMP2 computation

    """
    # Get calls method
    lowername = name.lower()
 
    # what type of sos?
    if (lowername == 'sos-omp2'):
        PsiMod.set_local_option('OMP2', 'SOS_TYPE', 'SOS')
    elif (lowername == 'sos-pi-omp2'):
        PsiMod.set_local_option('OMP2', 'SOS_TYPE', 'SOSPI')

    PsiMod.scf()
    PsiMod.set_local_option('OMP2', 'DO_SOS', 'TRUE')
    return PsiMod.omp2()


def run_omp3(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    an orbital-optimized MP3 computation

    """
    #oldref = PsiMod.get_global_option('REFERENCE')
    #PsiMod.set_global_option('REFERENCE', 'UHF')
    PsiMod.scf()
    return PsiMod.omp3()
    #PsiMod.set_global_option('REFERENCE', oldref)    


def run_scs_omp3(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a spin-component scaled OMP3 computation

    """
    # Get calls method
    lowername = name.lower()
 
    # what type of scs?
    if (lowername == 'scs-omp3'):
        PsiMod.set_local_option('OMP3', 'SCS_TYPE', 'SCS')
    elif (lowername == 'scsn-omp3'):
        PsiMod.set_local_option('OMP3', 'SCS_TYPE', 'SCSN')
    elif (lowername == 'scs-mi-omp3'):
        PsiMod.set_local_option('OMP3', 'SCS_TYPE', 'SCSMI')
    elif (lowername == 'scs-omp3-vdw'):
        PsiMod.set_local_option('OMP3', 'SCS_TYPE', 'SCSVDW')

    PsiMod.scf()
    PsiMod.set_local_option('OMP3', 'DO_SCS', 'TRUE')
    return PsiMod.omp3()


def run_sos_omp3(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a spin-opposite scaled OMP3 computation

    """
    # Get calls method
    lowername = name.lower()
 
    # what type of sos?
    if (lowername == 'sos-omp3'):
        PsiMod.set_local_option('OMP3', 'SOS_TYPE', 'SOS')
    elif (lowername == 'sos-pi-omp3'):
        PsiMod.set_local_option('OMP3', 'SOS_TYPE', 'SOSPI')

    PsiMod.scf()
    PsiMod.set_local_option('OMP3', 'DO_SOS', 'TRUE')
    return PsiMod.omp3()


def run_scf(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a self-consistent-field theory (HF & DFT) calculation.

    """
    lowername = name.lower()

    optstash = OptionsState(
        ['SCF', 'DFT_FUNCTIONAL'],
        ['SCF', 'SCF_TYPE'],
        ['SCF', 'REFERENCE'])

    # Alter default algorithm
    if not PsiMod.has_option_changed('SCF', 'SCF_TYPE'):
        PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')

    if lowername == 'df-scf':
        PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')
    elif lowername == 'hf':
        if PsiMod.get_option('SCF', 'REFERENCE') == 'RKS':
            PsiMod.set_local_option('SCF', 'REFERENCE', 'RHF')
        elif PsiMod.get_option('SCF', 'REFERENCE') == 'UKS':
            PsiMod.set_local_option('SCF', 'REFERENCE', 'UHF')
        else:
            pass
    elif lowername == 'rhf':
        PsiMod.set_local_option('SCF', 'REFERENCE', 'RHF')
    elif lowername == 'uhf':
        PsiMod.set_local_option('SCF', 'REFERENCE', 'UHF')
    elif lowername == 'rohf':
        PsiMod.set_local_option('SCF', 'REFERENCE', 'ROHF')
    elif lowername == 'rscf':
        if (len(PsiMod.get_option('SCF', 'DFT_FUNCTIONAL')) > 0) or PsiMod.get_option('SCF', 'DFT_CUSTOM_FUNCTIONAL') is not None:
            PsiMod.set_local_option('SCF', 'REFERENCE', 'RKS')
        else:
            PsiMod.set_local_option('SCF', 'REFERENCE', 'RHF')
    elif lowername == 'uscf':
        if (len(PsiMod.get_option('SCF', 'DFT_FUNCTIONAL')) > 0) or PsiMod.get_option('SCF', 'DFT_CUSTOM_FUNCTIONAL') is not None:
            PsiMod.set_local_option('SCF', 'REFERENCE', 'UKS')
        else:
            PsiMod.set_local_option('SCF', 'REFERENCE', 'UHF')
    elif lowername == 'roscf':
        if (len(PsiMod.get_option('SCF', 'DFT_FUNCTIONAL')) > 0) or PsiMod.get_option('SCF', 'DFT_CUSTOM_FUNCTIONAL') is not None:
            raise ValidationError('ROHF reference for DFT is not available.')
        else:
            PsiMod.set_local_option('SCF', 'REFERENCE', 'ROHF')
   
    returnvalue = scf_helper(name, **kwargs)
    
    optstash.restore()
    return returnvalue


def run_scf_gradient(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a SCF gradient calculation.

    """
    optstash = OptionsState(
        ['DF_BASIS_SCF'],
        ['SCF', 'SCF_TYPE'])

    # Alter default algorithm
    if not PsiMod.has_option_changed('SCF', 'SCF_TYPE'):
        PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')

    returnvalue = run_scf(name, **kwargs)

    if (PsiMod.get_option('SCF', 'SCF_TYPE') == 'DF'):

        # if the df_basis_scf basis is not set, pick a sensible one.
        if PsiMod.get_global_option('DF_BASIS_SCF') == '':
            jkbasis = corresponding_jkfit(PsiMod.get_global_option('BASIS'))
            if jkbasis:
                PsiMod.set_global_option('DF_BASIS_SCF', jkbasis)
                PsiMod.print_out('\nNo DF_BASIS_SCF auxiliary basis selected, defaulting to %s\n\n' % (jkbasis))
            else:
                raise ValidationError('Keyword DF_BASIS_SCF is required.')

        PsiMod.scfgrad()

    else:
        PsiMod.deriv()

    optstash.restore()
    return returnvalue


def run_libfock(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a calculation through libfock, namely RCPHF,
    RCIS, RTDHF, RTDA, and RTDDFT.

    """
    if (name.lower() == 'cphf'):
        PsiMod.set_global_option('MODULE', 'RCPHF')
    if (name.lower() == 'cis'):
        PsiMod.set_global_option('MODULE', 'RCIS')
    if (name.lower() == 'tdhf'):
        PsiMod.set_global_option('MODULE', 'RTDHF')
    if (name.lower() == 'cpks'):
        PsiMod.set_global_option('MODULE', 'RCPKS')
    if (name.lower() == 'tda'):
        PsiMod.set_global_option('MODULE', 'RTDA')
    if (name.lower() == 'tddft'):
        PsiMod.set_global_option('MODULE', 'RTDDFT')

    PsiMod.libfock()


def run_mcscf(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a multiconfigurational self-consistent-field calculation.

    """
    return PsiMod.mcscf()


def scf_helper(name, **kwargs):
    """Function serving as helper to SCF, choosing whether to cast
    up or just run SCF with a standard guess. This preserves
    previous SCF options set by other procedures (e.g., SAPT
    output file types for SCF).

    """
    optstash = OptionsState(
        ['PUREAM'],
        ['BASIS'],
        ['DF_BASIS_SCF'],
        ['SCF', 'SCF_TYPE'],
        ['SCF', 'GUESS'],
        ['SCF', 'DF_INTS_IO'])

    # if the df_basis_scf basis is not set, pick a sensible one.
    if PsiMod.get_option('SCF', 'SCF_TYPE') == 'DF':
        if PsiMod.get_global_option('DF_BASIS_SCF') == '':
            jkbasis = corresponding_jkfit(PsiMod.get_global_option('BASIS'))
            if jkbasis:
                PsiMod.set_global_option('DF_BASIS_SCF', jkbasis)
                PsiMod.print_out('\nNo DF_BASIS_SCF auxiliary basis selected, defaulting to %s\n\n' % (jkbasis))
            else:
                raise ValidationError('Keyword DF_BASIS_SCF is required.')

    optstash2 = OptionsState(
        ['BASIS'],
        ['DF_BASIS_SCF'],
        ['SCF', 'SCF_TYPE'],
        ['SCF', 'DF_INTS_IO'])

    # sort out cast_up settings. no need to stash these since only read, never reset
    cast = False
    if PsiMod.has_option_changed('SCF', 'BASIS_GUESS'):
        cast = PsiMod.get_option('SCF', 'BASIS_GUESS')
        if yes.match(str(cast)):
            cast = True
        elif no.match(str(cast)):
            cast = False

        if PsiMod.get_option('SCF', 'SCF_TYPE') == 'DF':
            castdf = True
        else:
            castdf = False

        if PsiMod.has_option_changed('SCF', 'DF_BASIS_GUESS'):
            castdf = PsiMod.get_option('SCF', 'DF_BASIS_GUESS')
            if yes.match(str(castdf)):
                castdf = True
            elif no.match(str(castdf)):
                castdf = False

    # sort out broken_symmetry settings.
    if 'brokensymmetry' in kwargs:
        molecule = PsiMod.get_active_molecule()
        multp = molecule.multiplicity()
        if multp != 1:
            raise ValidationError('Broken symmetry is only for singlets.')
        #if PsiMod.get_option('SCF','REFERENCE') != 'UHF' and lowername != 'UHF':
        if PsiMod.get_option('SCF','REFERENCE') != 'UHF':
            raise ValidationError('You must specify "set reference uhf" to use broken symmetry.')
        do_broken = True
    else:
        do_broken = False

    precallback = None
    if 'precallback' in kwargs:
        precallback = kwargs.pop('precallback')

    postcallback = None
    if 'postcallback' in kwargs:
        postcallback = kwargs.pop('postcallback')

    # Hack to ensure cartesian or pure are used throughout
    # Note that can't query PUREAM option directly, as it only
    #   reflects user changes to value, so load basis and
    #   read effective PUREAM setting off of it
    PsiMod.set_global_option('BASIS', PsiMod.get_global_option('BASIS'))
    PsiMod.set_global_option('PUREAM', PsiMod.MintsHelper().basisset().has_puream())

    # broken set-up
    if do_broken:
        molecule.set_multiplicity(3)
#        PsiMod.print_out("\n\n\tComputing high-spin triplet guess\n\n")
        PsiMod.print_out('\n')
        banner('  Computing high-spin triplet guess  ')
        PsiMod.print_out('\n')

    # cast set-up
    if (cast):

        if yes.match(str(cast)):
            guessbasis = '3-21G'
        else:
            guessbasis = cast

        if (castdf):
            if yes.match(str(castdf)):
                guessbasisdf = corresponding_jkfit(guessbasis)
            else:
                guessbasisdf = castdf

        # Switch to the guess namespace
        namespace = PsiMod.IO.get_default_namespace()
        PsiMod.IO.set_default_namespace((namespace + '.guess'))

        # Setup initial SCF
        PsiMod.set_global_option('BASIS', guessbasis)
        if (castdf):
            PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')
            PsiMod.set_local_option('SCF', 'DF_INTS_IO', 'none')
            PsiMod.set_global_option('DF_BASIS_SCF', guessbasisdf)

        # Print some info about the guess
        PsiMod.print_out('\n')
        banner('Guess SCF, %s Basis' % (guessbasis))
        PsiMod.print_out('\n')

    # the FIRST scf call
    if cast or do_broken:
        # Perform the guess scf
        PsiMod.scf()

    # broken clean-up
    if do_broken:
        molecule.set_multiplicity(1)
        PsiMod.set_local_option('SCF', 'GUESS', 'READ')
        PsiMod.print_out('\n')
        banner('  Computing broken symmetry solution from high-spin triplet guess  ')
        PsiMod.print_out('\n')

    # cast clean-up
    if (cast):

        # Move files to proper namespace
        PsiMod.IO.change_file_namespace(180, (namespace + '.guess'), namespace)
        PsiMod.IO.set_default_namespace(namespace)

        # Set to read and project, and reset bases to final ones
        optstash2.restore()
        PsiMod.set_local_option('SCF', 'GUESS', 'READ')

        # Print the banner for the standard operation
        PsiMod.print_out('\n')
        banner(name.upper())
        PsiMod.print_out('\n')


    # the SECOND scf call
    e_scf = PsiMod.scf(precallback, postcallback)

    optstash.restore()
    return e_scf


def run_mp2_select(name, **kwargs):
    """Function selecting the algorithm for a MP2 energy call
    and directing toward the MP2 or the DFMP2 modules.

    """
    if PsiMod.get_option("MP2", "MP2_TYPE") == "CONV":
        # PSI3 docs claimed to have an integral direct algorithm
        #   but can't see it in the code.
        return run_mp2(name, **kwargs)
    else:
        return run_dfmp2(name, **kwargs)


def run_mp2_select_gradient(name, **kwargs):
    """Function selecting the algorithm for a MP2 gradient call
    and directing toward the MP2 or the DFMP2 modules.

    """
    if PsiMod.get_option("MP2", "MP2_TYPE") == "CONV":
        # PSI3 docs claimed to have an integral direct algorithm
        #   but can't see it in the code.
        return run_mp2_gradient(name, **kwargs)
    else:
        return run_dfmp2_gradient(name, **kwargs)


def run_mp2(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a MP2 calculation.

    """
    optstash = OptionsState(
        ['TRANSQT2', 'WFN'],
        ['CCSORT', 'WFN'],
        ['MP2', 'WFN'])

    # Bypass routine scf if user did something special to get it to converge
    if not (('bypass_scf' in kwargs) and yes.match(str(kwargs['bypass_scf']))):
        scf_helper(name, **kwargs)

        # If the scf type is DF, then the AO integrals were never generated
        if PsiMod.get_option('SCF', 'SCF_TYPE') == 'DF':
            mints = PsiMod.MintsHelper()
            mints.integrals()

    PsiMod.set_local_option('TRANSQT2', 'WFN', 'MP2')
    PsiMod.set_local_option('CCSORT', 'WFN', 'MP2')
    PsiMod.set_local_option('MP2', 'WFN', 'MP2')

    PsiMod.transqt2()
    PsiMod.ccsort()
    returnvalue = PsiMod.mp2()

    optstash.restore()
    return returnvalue


def run_mp2_gradient(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a MP2 gradient calculation.

    """
    optstash = OptionsState(
        ['TRANSQT2', 'WFN'],
        ['CCSORT', 'WFN'],
        ['MP2', 'WFN'],
        ['DERTYPE'])

    PsiMod.set_global_option('DERTYPE', 'FIRST')
    run_mp2(name, **kwargs)

    PsiMod.set_local_option('MP2', 'WFN', 'MP2')
    PsiMod.deriv()

    optstash.restore()


def run_dfmp2_gradient(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a DFMP2 gradient calculation.

    """
    optstash = OptionsState(
        ['DF_BASIS_SCF'],
        ['DF_BASIS_MP2'],
        ['SCF_TYPE'])

    # Alter default algorithm
    if not PsiMod.has_option_changed('SCF', 'SCF_TYPE'):
        #PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')  # insufficient b/c SCF option read in DFMP2
        PsiMod.set_global_option('SCF_TYPE', 'DF')

    if not PsiMod.get_option('SCF', 'SCF_TYPE') == 'DF':
        raise ValidationError('DF-MP2 gradients need DF-SCF reference, for now.')

    if 'restart_file' in kwargs:
        restartfile = kwargs.pop('restart_file')
        # Rename the checkpoint file to be consistent with psi4's file system
        psioh = PsiMod.IOManager.shared_object()
        psio = PsiMod.IO.shared_object()
        filepath = psioh.get_file_path(32)
        namespace = psio.get_default_namespace()
        pid = str(os.getpid())
        prefix = 'psi'
        targetfile = filepath + prefix + '.' + pid + '.' + namespace + '.32'
        if(PsiMod.me() == 0):
            shutil.copy(restartfile, targetfile)
    else:
        # if the df_basis_scf basis is not set, pick a sensible one.
        if PsiMod.get_global_option('DF_BASIS_SCF') == '':
            jkbasis = corresponding_jkfit(PsiMod.get_global_option('BASIS'))
            if jkbasis:
                PsiMod.set_global_option('DF_BASIS_SCF', jkbasis)
                PsiMod.print_out('\nNo DF_BASIS_SCF auxiliary basis selected, defaulting to %s\n\n' % (jkbasis))
            else:
                raise ValidationError('Keyword DF_BASIS_SCF is required.')

        scf_helper(name, **kwargs)

    PsiMod.print_out('\n')
    banner('DFMP2')
    PsiMod.print_out('\n')

    # if the df_basis_mp2 basis is not set, pick a sensible one.
    if PsiMod.get_global_option('DF_BASIS_MP2') == '':
        ribasis = corresponding_rifit(PsiMod.get_global_option('BASIS'))
        if ribasis:
            PsiMod.set_global_option('DF_BASIS_MP2', ribasis)
            PsiMod.print_out('No DF_BASIS_MP2 auxiliary basis selected, defaulting to %s\n' % (ribasis))
        else:
            raise ValidationError('Keyword DF_BASIS_MP2 is required.')

    PsiMod.dfmp2grad()
    e_dfmp2 = PsiMod.get_variable('DF-MP2 ENERGY')
    e_scs_dfmp2 = PsiMod.get_variable('SCS-DF-MP2 ENERGY')

    optstash.restore()

    if (name.upper() == 'SCS-DFMP2') or (name.upper() == 'SCS-DF-MP2'):
        return e_scs_dfmp2
    elif (name.upper() == 'DF-MP2') or (name.upper() == 'DFMP2') or (name.upper() == 'MP2'):
        return e_dfmp2


def run_ccenergy(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a CCSD, CC2, and CC3 calculation.

    """
    lowername = name.lower()

    optstash = OptionsState(
        ['TRANSQT2', 'WFN'],
        ['CCSORT', 'WFN'],
        ['CCENERGY', 'WFN'])

    if (lowername == 'ccsd'):
        PsiMod.set_local_option('TRANSQT2', 'WFN', 'CCSD')
        PsiMod.set_local_option('CCSORT', 'WFN', 'CCSD')
        PsiMod.set_local_option('CCENERGY', 'WFN', 'CCSD')
    elif (lowername == 'ccsd(t)'):
        PsiMod.set_local_option('TRANSQT2', 'WFN', 'CCSD_T')
        PsiMod.set_local_option('CCSORT', 'WFN', 'CCSD_T')
        PsiMod.set_local_option('CCENERGY', 'WFN', 'CCSD_T')
    elif (lowername == 'cc2'):
        PsiMod.set_local_option('TRANSQT2', 'WFN', 'CC2')
        PsiMod.set_local_option('CCSORT', 'WFN', 'CC2')
        PsiMod.set_local_option('CCENERGY', 'WFN', 'CC2')
    elif (lowername == 'cc3'):
        PsiMod.set_local_option('TRANSQT2', 'WFN', 'CC3')
        PsiMod.set_local_option('CCSORT', 'WFN', 'CC3')
        PsiMod.set_local_option('CCENERGY', 'WFN', 'CC3')
    elif (lowername == 'eom-cc2'):
        PsiMod.set_local_option('TRANSQT2', 'WFN', 'EOM_CC2')
        PsiMod.set_local_option('CCSORT', 'WFN', 'EOM_CC2')
        PsiMod.set_local_option('CCENERGY', 'WFN', 'EOM_CC2')
    elif (lowername == 'eom-ccsd'):
        PsiMod.set_local_option('TRANSQT2', 'WFN', 'EOM_CCSD')
        PsiMod.set_local_option('CCSORT', 'WFN', 'EOM_CCSD')
        PsiMod.set_local_option('CCENERGY', 'WFN', 'EOM_CCSD')
    # Call a plain energy('ccenergy') and have full control over options, incl. wfn
    elif(lowername == 'ccenergy'):
        pass

    # Bypass routine scf if user did something special to get it to converge
    if not (('bypass_scf' in kwargs) and yes.match(str(kwargs['bypass_scf']))):
        scf_helper(name, **kwargs)

        # If the scf type is DF, then the AO integrals were never generated
        if PsiMod.get_option('SCF', 'SCF_TYPE') == 'DF':
            mints = PsiMod.MintsHelper()
            mints.integrals()

    PsiMod.transqt2()
    PsiMod.ccsort()
    returnvalue = PsiMod.ccenergy()

    optstash.restore()
    return returnvalue


def run_cc_gradient(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a CCSD and CCSD(T) gradient calculation.

    """
    PsiMod.set_global_option('DERTYPE', 'FIRST')

    run_ccenergy(name, **kwargs)
    if (name.lower() == 'ccsd'):
        PsiMod.set_global_option('WFN', 'CCSD')
    elif (name.lower() == 'ccsd(t)'):
        PsiMod.set_global_option('WFN', 'CCSD_T')

    PsiMod.cchbar()
    PsiMod.cclambda()
    PsiMod.ccdensity()
    PsiMod.deriv()

    if (name.lower() != 'ccenergy'):
        PsiMod.set_global_option('WFN', 'SCF')
        PsiMod.revoke_global_option_changed('WFN')
        PsiMod.set_global_option('DERTYPE', 'NONE')
        PsiMod.revoke_global_option_changed('DERTYPE')


def run_bccd(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a Brueckner CCD calculation.

    """
    if (name.lower() == 'bccd'):
        PsiMod.set_global_option('WFN', 'BCCD')

    # Bypass routine scf if user did something special to get it to
    # converge
    if not (('bypass_scf' in kwargs) and yes.match(str(kwargs['bypass_scf']))):
        scf_helper(name, **kwargs)

        # If the scf type is DF, then the AO integrals were never generated
        if PsiMod.get_option('SCF', 'SCF_TYPE') == 'DF':
            mints = PsiMod.MintsHelper()
            mints.integrals()

    PsiMod.set_global_option('DELETE_TEI', 'false')

    while True:
        PsiMod.transqt2()
        PsiMod.ccsort()
        returnvalue = PsiMod.ccenergy()
        PsiMod.print_out('Brueckner convergence check: %d\n' % PsiMod.get_variable('BRUECKNER CONVERGED'))
        if (PsiMod.get_variable('BRUECKNER CONVERGED') == True):
            break

    return returnvalue


def run_bccd_t(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a Brueckner CCD(T) calculation.

    """
    PsiMod.set_global_option('WFN', 'BCCD_T')
    run_bccd(name, **kwargs)

    return PsiMod.cctriples()


def run_scf_property(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    SCF calculations. This is a simple alias to :py:func:`~proc.run_scf`
    since SCF properties all handled through oeprop.

    """
    optstash = OptionsState(
        ['SCF', 'SCF_TYPE'])

    # Alter default algorithm
    if not PsiMod.has_option_changed('SCF', 'SCF_TYPE'):
        PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')

    returnvalue = run_scf(name, **kwargs)

    optstash.restore()
    return returnvalue


def run_cc_property(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    all CC property calculations.

    """
    oneel_properties = ['dipole', 'quadrupole']
    twoel_properties = []
    response_properties = ['polarizability', 'rotation', 'roa']
    excited_properties = ['oscillator_strength', 'rotational_strength']

    one = []
    two = []
    response = []
    excited = []
    invalid = []

    if 'properties' in kwargs:
        properties = kwargs.pop('properties')
        properties = drop_duplicates(properties)

        for prop in properties:
            if prop in oneel_properties:
                one.append(prop)
            elif prop in twoel_properties:
                two.append(prop)
            elif prop in response_properties:
                response.append(prop)
            elif prop in excited_properties:
                excited.append(prop)
            else:
                invalid.append(prop)
    else:
        raise ValidationError("The \"properties\" keyword is required with the property() function.")

    n_one = len(one)
    n_two = len(two)
    n_response = len(response)
    n_excited = len(excited)
    n_invalid = len(invalid)

    if (n_invalid > 0):
        print("The following properties are not currently supported: %s" % invalid)

    if (n_excited > 0 and (name.lower() != 'eom-ccsd' and name.lower() != 'eom-cc2')):
        raise ValidationError("Excited state CC properties require EOM-CC2 or EOM-CCSD.")

    if ((name.lower() == 'eom-ccsd' or name.lower() == 'eom-cc2') and n_response > 0):
        raise ValidationError("Cannot (yet) compute response properties for excited states.")

    if (n_one > 0 or n_two > 0) and (n_response > 0):
        print("Computing both density- and response-based properties.")

    if (name.lower() == 'ccsd'):
        PsiMod.set_global_option('WFN', 'CCSD')
        run_ccenergy('ccsd', **kwargs)
        PsiMod.set_global_option('WFN', 'CCSD')
    elif (name.lower() == 'cc2'):
        PsiMod.set_global_option('WFN', 'CC2')
        run_ccenergy('cc2', **kwargs)
        PsiMod.set_global_option('WFN', 'CC2')
    elif (name.lower() == 'eom-ccsd'):
        PsiMod.set_global_option('WFN', 'EOM_CCSD')
        run_ccenergy('eom-ccsd', **kwargs)
        PsiMod.set_global_option('WFN', 'EOM_CCSD')
    elif (name.lower() == 'eom-cc2'):
        PsiMod.set_global_option('WFN', 'EOM_CC2')
        run_ccenergy('eom-cc2', **kwargs)
        PsiMod.set_global_option('WFN', 'EOM_CC2')

    # Need cchbar for everything
    PsiMod.cchbar()

    # Need ccdensity at this point only for density-based props
    if (n_one > 0 or n_two > 0):
        if (name.lower() == 'eom-ccsd'):
            PsiMod.set_global_option('WFN', 'EOM_CCSD')
            PsiMod.set_global_option('DERTYPE', 'NONE')
            PsiMod.set_global_option('ONEPDM', 'TRUE')
            PsiMod.cceom()
        elif (name.lower() == 'eom-cc2'):
            PsiMod.set_global_option('WFN', 'EOM_CC2')
            PsiMod.set_global_option('DERTYPE', 'NONE')
            PsiMod.set_global_option('ONEPDM', 'TRUE')
            PsiMod.cceom()
        PsiMod.set_global_option('DERTYPE', 'NONE')
        PsiMod.set_global_option('ONEPDM', 'TRUE')
        PsiMod.cclambda()
        PsiMod.ccdensity()

    # Need ccresponse only for response-type props
    if (n_response > 0):
        PsiMod.set_global_option('DERTYPE', 'RESPONSE')
        PsiMod.cclambda()
        for prop in response:
            PsiMod.set_global_option('PROPERTY', prop)
            PsiMod.ccresponse()

    # Excited-state transition properties
    if (n_excited > 0):
        if (name.lower() == 'eom-ccsd'):
            PsiMod.set_global_option('WFN', 'EOM_CCSD')
        elif (name.lower() == 'eom-cc2'):
            PsiMod.set_global_option('WFN', 'EOM_CC2')
        else:
            raise ValidationError("Unknown excited-state CC wave function.")
        PsiMod.set_global_option('DERTYPE', 'NONE')
        PsiMod.set_global_option('ONEPDM', 'TRUE')
        PsiMod.cceom()
        PsiMod.cclambda()
        PsiMod.ccdensity()

    PsiMod.set_global_option('WFN', 'SCF')
    PsiMod.revoke_global_option_changed('WFN')
    PsiMod.set_global_option('DERTYPE', 'NONE')
    PsiMod.revoke_global_option_changed('DERTYPE')


def run_eom_cc(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    an EOM-CC calculation, namely EOM-CC2, EOM-CCSD, and EOM-CC3.

    """
    if (name.lower() == 'eom-ccsd'):
        PsiMod.set_global_option('WFN', 'EOM_CCSD')
        run_ccenergy('ccsd', **kwargs)
        PsiMod.set_global_option('WFN', 'EOM_CCSD')
    elif (name.lower() == 'eom-cc2'):
        PsiMod.set_global_option('WFN', 'EOM_CC2')
        run_ccenergy('cc2', **kwargs)
        PsiMod.set_global_option('WFN', 'EOM_CC2')
    elif (name.lower() == 'eom-cc3'):
        PsiMod.set_global_option('WFN', 'EOM_CC3')
        run_ccenergy('cc3', **kwargs)
        PsiMod.set_global_option('WFN', 'EOM_CC3')

    PsiMod.cchbar()
    returnvalue = PsiMod.cceom()

    PsiMod.set_global_option('WFN', 'SCF')
    PsiMod.revoke_global_option_changed('WFN')

    return returnvalue


def run_eom_cc_gradient(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    an EOM-CCSD gradient calculation.

    """
    optstash = OptionsState(
        ['CCDENSITY', 'XI'],
        ['CCDENSITY', 'ZETA'],
        ['CCLAMBDA', 'ZETA'],
        ['DERTYPE'],
        ['CCDENSITY', 'WFN'],
        ['CCLAMBDA', 'WFN'])

    PsiMod.set_global_option('DERTYPE', 'FIRST')

    if (name.lower() == 'eom-ccsd'):
        PsiMod.set_global_option('WFN', 'EOM_CCSD')
        energy = run_eom_cc(name, **kwargs)
        PsiMod.set_global_option('WFN', 'EOM_CCSD')

    PsiMod.set_local_option('CCLAMBDA', 'WFN', 'EOM_CCSD')
    PsiMod.set_local_option('CCDENSITY', 'WFN', 'EOM_CCSD')
    PsiMod.set_local_option('CCLAMBDA', 'ZETA', 'FALSE')
    PsiMod.set_local_option('CCDENSITY', 'ZETA', 'FALSE')
    PsiMod.set_local_option('CCDENSITY', 'XI', 'TRUE')
    PsiMod.cclambda()
    PsiMod.ccdensity()
    PsiMod.set_local_option('CCLAMBDA', 'ZETA', 'TRUE')
    PsiMod.set_local_option('CCDENSITY', 'ZETA', 'TRUE')
    PsiMod.set_local_option('CCDENSITY', 'XI', 'FALSE')
    PsiMod.cclambda()
    PsiMod.ccdensity()
    PsiMod.deriv()

    optstash.restore()


def run_adc(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    an algebraic diagrammatic construction calculation.

    .. caution:: Get rid of active molecule lines- should be handled in energy.

    """
    molecule = PsiMod.get_active_molecule()
    if 'molecule' in kwargs:
        molecule = kwargs.pop('molecule')

    if not molecule:
        raise ValueNotSet('no molecule found')

    PsiMod.scf()

    return PsiMod.adc()


def run_dft(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a density-functional-theory calculation.

    """
    optstash = OptionsState(
        ['SCF', 'DFT_FUNCTIONAL'],
        ['SCF', 'REFERENCE'],
        ['SCF', 'SCF_TYPE'],
        ['DF_BASIS_MP2'],
        ['DFMP2', 'MP2_OS_SCALE'],
        ['DFMP2', 'MP2_SS_SCALE'])

    # Alter default algorithm
    if not PsiMod.has_option_changed('SCF', 'SCF_TYPE'):
        PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')

    PsiMod.set_local_option('SCF', 'DFT_FUNCTIONAL', name)

    user_ref = PsiMod.get_option('SCF', 'REFERENCE')
    if (user_ref == 'RHF'):
        PsiMod.set_local_option('SCF', 'REFERENCE', 'RKS')
    elif (user_ref == 'UHF'):
        PsiMod.set_local_option('SCF', 'REFERENCE', 'UKS')
    elif (user_ref == 'ROHF'):
        raise ValidationError('ROHF reference for DFT is not available.')
    elif (user_ref == 'CUHF'):
        raise ValidationError('CUHF reference for DFT is not available.')

    returnvalue = run_scf(name, **kwargs)

    for ssuper in superfunctional_list():
        if ssuper.name().lower() == name.lower():
            dfun = ssuper

    if dfun.is_c_hybrid():

        # if the df_basis_mp2 basis is not set, pick a sensible one.
        if PsiMod.get_global_option('DF_BASIS_MP2') == '':
            ribasis = corresponding_rifit(PsiMod.get_global_option('BASIS'))
            if ribasis:
                PsiMod.set_global_option('DF_BASIS_MP2', ribasis)
                PsiMod.print_out('No DF_BASIS_MP2 auxiliary basis selected, defaulting to %s\n' % (ribasis))
            else:
                raise ValidationError('Keyword DF_BASIS_MP2 is required.')

        if dfun.is_c_scs_hybrid():
            PsiMod.set_local_option('DFMP2', 'MP2_OS_SCALE', dfun.c_os_alpha())
            PsiMod.set_local_option('DFMP2', 'MP2_SS_SCALE', dfun.c_ss_alpha())
            PsiMod.dfmp2()
            vdh = dfun.c_alpha() * PsiMod.get_variable('SCS-DF-MP2 CORRELATION ENERGY')

        else:
            PsiMod.dfmp2()
            vdh = dfun.c_alpha() * PsiMod.get_variable('DF-MP2 CORRELATION ENERGY')

        PsiMod.set_variable('DOUBLE-HYBRID CORRECTION ENERGY', vdh)
        returnvalue += vdh
        PsiMod.set_variable('DFT TOTAL ENERGY', returnvalue)
        PsiMod.set_variable('CURRENT ENERGY', returnvalue)

    optstash.restore()
    return returnvalue


def run_dft_gradient(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a density-functional-theory gradient calculation.

    """
    optstash = OptionsState(
        ['SCF', 'DFT_FUNCTIONAL'],
        ['SCF', 'REFERENCE'],
        ['SCF', 'SCF_TYPE'])

    # Alter default algorithm
    if not PsiMod.has_option_changed('SCF', 'SCF_TYPE'):
        PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')

    PsiMod.set_local_option('SCF', 'DFT_FUNCTIONAL', name)

    user_ref = PsiMod.get_option('SCF', 'REFERENCE')
    if (user_ref == 'RHF'):
        PsiMod.set_local_option('SCF', 'REFERENCE', 'RKS')
    elif (user_ref == 'UHF'):
        PsiMod.set_local_option('SCF', 'REFERENCE', 'UKS')
    elif (user_ref == 'ROHF'):
        raise ValidationError('ROHF reference for DFT is not available.')
    elif (user_ref == 'CUHF'):
        raise ValidationError('CUHF reference for DFT is not available.')

    if (PsiMod.get_option('SCF', 'SCF_TYPE') != 'DF'):
        raise ValidationError('SCF_TYPE must be DF for DFT gradient (for now).')

    returnvalue = run_scf_gradient(name, **kwargs)

    optstash.restore()
    return returnvalue


def run_detci(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a configuration interaction calculation, namely FCI,
    CIn, MPn, and ZAPTn.

    """
    optstash = OptionsState(
        ['TRANSQT2', 'WFN'],
        ['DETCI', 'WFN'],
        ['DETCI', 'MAX_NUM_VECS'],
        ['DETCI', 'MPN_ORDER_SAVE'],
        ['DETCI', 'MPN'],
        ['DETCI', 'FCI'],
        ['DETCI', 'EX_LEVEL'])

    if (name.lower() == 'zapt'):
        PsiMod.set_local_option('TRANSQT2', 'WFN', 'ZAPTN')
        PsiMod.set_local_option('DETCI', 'WFN', 'ZAPTN')
        level = kwargs['level']
        maxnvect = (level + 1) / 2 + (level + 1) % 2
        PsiMod.set_local_option('DETCI', 'MAX_NUM_VECS', maxnvect)
        if ((level + 1) % 2):
            PsiMod.set_local_option('DETCI', 'MPN_ORDER_SAVE', 2)
        else:
            PsiMod.set_local_option('DETCI', 'MPN_ORDER_SAVE', 1)
    elif (name.lower() == 'mp'):
        PsiMod.set_local_option('TRANSQT2', 'WFN', 'DETCI')
        PsiMod.set_local_option('DETCI', 'WFN', 'DETCI')
        PsiMod.set_local_option('DETCI', 'MPN', 'TRUE')

        level = kwargs['level']
        maxnvect = (level + 1) / 2 + (level + 1) % 2
        PsiMod.set_local_option('DETCI', 'MAX_NUM_VECS', maxnvect)
        if ((level + 1) % 2):
            PsiMod.set_local_option('DETCI', 'MPN_ORDER_SAVE', 2)
        else:
            PsiMod.set_local_option('DETCI', 'MPN_ORDER_SAVE', 1)
    elif (name.lower() == 'fci'):
            PsiMod.set_local_option('TRANSQT2', 'WFN', 'DETCI')
            PsiMod.set_local_option('DETCI', 'WFN', 'DETCI')
            PsiMod.set_local_option('DETCI', 'FCI', 'TRUE')
    elif (name.lower() == 'cisd'):
            PsiMod.set_local_option('TRANSQT2', 'WFN', 'DETCI')
            PsiMod.set_local_option('DETCI', 'WFN', 'DETCI')
            PsiMod.set_local_option('DETCI', 'EX_LEVEL', 2)
    elif (name.lower() == 'cisdt'):
            PsiMod.set_local_option('TRANSQT2', 'WFN', 'DETCI')
            PsiMod.set_local_option('DETCI', 'WFN', 'DETCI')
            PsiMod.set_local_option('DETCI', 'EX_LEVEL', 3)
    elif (name.lower() == 'cisdtq'):
            PsiMod.set_local_option('TRANSQT2', 'WFN', 'DETCI')
            PsiMod.set_local_option('DETCI', 'WFN', 'DETCI')
            PsiMod.set_local_option('DETCI', 'EX_LEVEL', 4)
    elif (name.lower() == 'ci'):
        PsiMod.set_local_option('TRANSQT2', 'WFN', 'DETCI')
        PsiMod.set_local_option('DETCI', 'WFN', 'DETCI')
        level = kwargs['level']
        PsiMod.set_local_option('DETCI', 'EX_LEVEL', level)
    # Call a plain energy('detci') and have full control over options
    elif(name.lower() == 'detci'):
        pass

    # Bypass routine scf if user did something special to get it to converge
    if not (('bypass_scf' in kwargs) and yes.match(str(kwargs['bypass_scf']))):
        scf_helper(name, **kwargs)

        # If the scf type is DF, then the AO integrals were never generated
        if PsiMod.get_option('SCF', 'SCF_TYPE') == 'DF':
            PsiMod.MintsHelper().integrals()

    PsiMod.transqt2()
    returnvalue = PsiMod.detci()

    optstash.restore()
    return returnvalue


def run_dfmp2(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a density-fitted MP2 calculation.

    """
    optstash = OptionsState(
        ['DF_BASIS_MP2'],
        ['SCF', 'SCF_TYPE'])

    # Alter default algorithm
    if not PsiMod.has_option_changed('SCF', 'SCF_TYPE'):
        PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')

    if not (PsiMod.get_option('SCF', 'REFERENCE') == 'RHF' or PsiMod.get_option('SCF', 'REFERENCE') == 'RKS'):
        raise ValidationError('Open-shell references not (yet) available for DF-MP2.')

    if 'restart_file' in kwargs:
        restartfile = kwargs.pop('restart_file')
        # Rename the checkpoint file to be consistent with psi4's file system
        psioh = PsiMod.IOManager.shared_object()
        psio = PsiMod.IO.shared_object()
        filepath = psioh.get_file_path(32)
        namespace = psio.get_default_namespace()
        pid = str(os.getpid())
        prefix = 'psi'
        targetfile = filepath + prefix + '.' + pid + '.' + namespace + '.32'
        if(PsiMod.me() == 0):
            shutil.copy(restartfile, targetfile)
    else:
        scf_helper(name, **kwargs)

    PsiMod.print_out('\n')
    banner('DFMP2')
    PsiMod.print_out('\n')

    # if the df_basis_mp2 basis is not set, pick a sensible one.
    if PsiMod.get_global_option('DF_BASIS_MP2') == '':
        ribasis = corresponding_rifit(PsiMod.get_global_option('BASIS'))
        if ribasis:
            PsiMod.set_global_option('DF_BASIS_MP2', ribasis)
            PsiMod.print_out('No DF_BASIS_MP2 auxiliary basis selected, defaulting to %s\n' % (ribasis))
        else:
            raise ValidationError('Keyword DF_BASIS_MP2 is required.')

    e_dfmp2 = PsiMod.dfmp2()
    e_scs_dfmp2 = PsiMod.get_variable('SCS-DF-MP2 ENERGY')

    optstash.restore()

    if (name.upper() == 'SCS-DFMP2') or (name.upper() == 'SCS-DF-MP2'):
        return e_scs_dfmp2
    elif (name.upper() == 'DF-MP2') or (name.upper() == 'DFMP2') or (name.upper() == 'MP2'):
        return e_dfmp2


def run_psimrcc(name, **kwargs):
    """Function encoding sequence of PSI module calls for a PSIMRCC computation
     using a reference from the MCSCF module

    """
    run_mcscf(name, **kwargs)
    PsiMod.psimrcc()
    e_psimrcc = PsiMod.get_variable("Current Energy")
    return e_psimrcc


def run_psimrcc_scf(name, **kwargs):
    """Function encoding sequence of PSI module calls for a PSIMRCC computation
     using a reference from the SCF module

    """

    scf_helper(name, **kwargs)
    PsiMod.psimrcc()
    e_psimrcc = PsiMod.get_variable("Current Energy")
    return e_psimrcc


def run_mp2c(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a coupled MP2 calculation.

    """
    optstash = OptionsState(
        ['DF_BASIS_MP2'])

    molecule = PsiMod.get_active_molecule()
    molecule.update_geometry()
    monomerA = molecule.extract_subsets(1, 2)
    monomerA.set_name('monomerA')
    monomerB = molecule.extract_subsets(2, 1)
    monomerB.set_name('monomerB')

    # if the df_basis_mp2 basis is not set, pick a sensible one.
    if PsiMod.get_global_option('DF_BASIS_MP2') == '':
        ribasis = corresponding_rifit(PsiMod.get_global_option('BASIS'))
        if ribasis:
            PsiMod.set_global_option('DF_BASIS_MP2', ribasis)
            PsiMod.print_out('No DF_BASIS_MP2 auxiliary basis selected, defaulting to %s\n' % (ribasis))
        else:
            raise ValidationError('Keyword DF_BASIS_MP2 is required.')

    ri = PsiMod.get_option('SCF', 'SCF_TYPE')
    df_ints_io = PsiMod.get_option('SCF', 'DF_INTS_IO')
    # inquire if above at all applies to dfmp2

    PsiMod.IO.set_default_namespace('dimer')
    PsiMod.set_local_option('SCF', 'SAPT', '2-dimer')
    PsiMod.print_out('\n')
    banner('Dimer HF')
    PsiMod.print_out('\n')
    PsiMod.set_global_option('DF_INTS_IO', 'SAVE')
    e_dimer = scf_helper('RHF', **kwargs)
    PsiMod.print_out('\n')
    banner('Dimer DFMP2')
    PsiMod.print_out('\n')
    e_dimer_mp2 = PsiMod.dfmp2()
    PsiMod.set_global_option('DF_INTS_IO', 'LOAD')

    activate(monomerA)
    if (ri == 'DF'):
        PsiMod.IO.change_file_namespace(97, 'dimer', 'monomerA')
    PsiMod.IO.set_default_namespace('monomerA')
    PsiMod.set_local_option('SCF', 'SAPT', '2-monomer_A')
    PsiMod.print_out('\n')
    banner('Monomer A HF')
    PsiMod.print_out('\n')
    e_monomerA = scf_helper('RHF', **kwargs)
    PsiMod.print_out('\n')
    banner('Monomer A DFMP2')
    PsiMod.print_out('\n')
    e_monomerA_mp2 = PsiMod.dfmp2()

    activate(monomerB)
    if (ri == 'DF'):
        PsiMod.IO.change_file_namespace(97, 'monomerA', 'monomerB')
    PsiMod.IO.set_default_namespace('monomerB')
    PsiMod.set_local_option('SCF', 'SAPT', '2-monomer_B')
    PsiMod.print_out('\n')
    banner('Monomer B HF')
    PsiMod.print_out('\n')
    e_monomerB = scf_helper('RHF', **kwargs)
    PsiMod.print_out('\n')
    banner('Monomer B DFMP2')
    PsiMod.print_out('\n')
    e_monomerB_mp2 = PsiMod.dfmp2()
    PsiMod.set_global_option('DF_INTS_IO', df_ints_io)

    PsiMod.IO.change_file_namespace(121, 'monomerA', 'dimer')
    PsiMod.IO.change_file_namespace(122, 'monomerB', 'dimer')

    activate(molecule)
    PsiMod.IO.set_default_namespace('dimer')
    PsiMod.set_local_option('SAPT', 'E_CONVERGENCE', 10e-10)
    PsiMod.set_local_option('SAPT', 'D_CONVERGENCE', 10e-10)
    PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'MP2C')
    PsiMod.print_out('\n')
    banner('MP2C')
    PsiMod.print_out('\n')

    PsiMod.set_variable('MP2C DIMER MP2 ENERGY', e_dimer_mp2)
    PsiMod.set_variable('MP2C MONOMER A MP2 ENERGY', e_monomerA_mp2)
    PsiMod.set_variable('MP2C MONOMER B MP2 ENERGY', e_monomerB_mp2)

    e_sapt = PsiMod.sapt()

    optstash.restore()
    return e_sapt


def run_sapt(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a SAPT calculation of any level.

    """
    optstash = OptionsState(
        ['SCF', 'SCF_TYPE'])

    # Alter default algorithm
    if not PsiMod.has_option_changed('SCF', 'SCF_TYPE'):
        PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')

    molecule = PsiMod.get_active_molecule()
    user_pg = molecule.schoenflies_symbol()
    molecule.reset_point_group('c1')
    molecule.fix_orientation(True)
    molecule.update_geometry()

    nfrag = molecule.nfragments()
    if nfrag != 2:
        raise ValidationError('SAPT requires active molecule to have 2 fragments, not %s.' % (nfrag))

    sapt_basis = 'dimer'
    if 'sapt_basis' in kwargs:
        sapt_basis = kwargs.pop('sapt_basis')
    sapt_basis = sapt_basis.lower()

    if (sapt_basis == 'dimer'):
        molecule.update_geometry()
        monomerA = molecule.extract_subsets(1, 2)
        monomerA.set_name('monomerA')
        monomerB = molecule.extract_subsets(2, 1)
        monomerB.set_name('monomerB')
    elif (sapt_basis == 'monomer'):
        molecule.update_geometry()
        monomerA = molecule.extract_subsets(1)
        monomerA.set_name('monomerA')
        monomerB = molecule.extract_subsets(2)
        monomerB.set_name('monomerB')

    ri = PsiMod.get_option('SCF', 'SCF_TYPE')
    df_ints_io = PsiMod.get_option('SCF', 'DF_INTS_IO')
    # inquire if above at all applies to dfmp2

    PsiMod.IO.set_default_namespace('dimer')
    PsiMod.set_local_option('SCF', 'SAPT', '2-dimer')
    PsiMod.print_out('\n')
    banner('Dimer HF')
    PsiMod.print_out('\n')
    if (sapt_basis == 'dimer'):
        PsiMod.set_global_option('DF_INTS_IO', 'SAVE')
    e_dimer = scf_helper('RHF', **kwargs)
    if (sapt_basis == 'dimer'):
        PsiMod.set_global_option('DF_INTS_IO', 'LOAD')

    activate(monomerA)
    if (ri == 'DF' and sapt_basis == 'dimer'):
        PsiMod.IO.change_file_namespace(97, 'dimer', 'monomerA')
    PsiMod.IO.set_default_namespace('monomerA')
    PsiMod.set_local_option('SCF', 'SAPT', '2-monomer_A')
    PsiMod.print_out('\n')
    banner('Monomer A HF')
    PsiMod.print_out('\n')
    e_monomerA = scf_helper('RHF', **kwargs)

    activate(monomerB)
    if (ri == 'DF' and sapt_basis == 'dimer'):
        PsiMod.IO.change_file_namespace(97, 'monomerA', 'monomerB')
    PsiMod.IO.set_default_namespace('monomerB')
    PsiMod.set_local_option('SCF', 'SAPT', '2-monomer_B')
    PsiMod.print_out('\n')
    banner('Monomer B HF')
    PsiMod.print_out('\n')
    e_monomerB = scf_helper('RHF', **kwargs)
    PsiMod.set_global_option('DF_INTS_IO', df_ints_io)

    PsiMod.IO.change_file_namespace(121, 'monomerA', 'dimer')
    PsiMod.IO.change_file_namespace(122, 'monomerB', 'dimer')

    activate(molecule)
    PsiMod.IO.set_default_namespace('dimer')
    PsiMod.set_local_option('SAPT', 'E_CONVERGENCE', 10e-10)
    PsiMod.set_local_option('SAPT', 'D_CONVERGENCE', 10e-10)
    if (name.lower() == 'sapt0'):
        PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'SAPT0')
    elif (name.lower() == 'sapt2'):
        PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'SAPT2')
    elif (name.lower() == 'sapt2+'):
        PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'SAPT2+')
    elif (name.lower() == 'sapt2+(3)'):
        PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'SAPT2+3')
        PsiMod.set_local_option('SAPT', 'DO_THIRD_ORDER', False)
    elif (name.lower() == 'sapt2+3'):
        PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'SAPT2+3')
        PsiMod.set_local_option('SAPT', 'DO_THIRD_ORDER', True)

    # if the df_basis_sapt basis is not set, pick a sensible one.
    if PsiMod.get_global_option('DF_BASIS_SAPT') == '':
        ribasis = corresponding_rifit(PsiMod.get_global_option('BASIS'))
        if ribasis:
            PsiMod.set_global_option('DF_BASIS_SAPT', ribasis)
            PsiMod.print_out('No DF_BASIS_SAPT auxiliary basis selected, defaulting to %s\n' % (ribasis))
        else:
            raise ValidationError('Keyword DF_BASIS_SAPT is required.')

    PsiMod.print_out('\n')
    banner(name.upper())
    PsiMod.print_out('\n')
    e_sapt = PsiMod.sapt()

    molecule.reset_point_group(user_pg)
    molecule.update_geometry()

    optstash.restore()
    return e_sapt


def run_sapt_ct(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a charge-transfer SAPT calcuation of any level.

    """
    optstash = OptionsState(
        ['SCF', 'SCF_TYPE'])

    # Alter default algorithm
    if not PsiMod.has_option_changed('SCF', 'SCF_TYPE'):
        PsiMod.set_local_option('SCF', 'SCF_TYPE', 'DF')

    molecule = PsiMod.get_active_molecule()
    user_pg = molecule.schoenflies_symbol()
    molecule.reset_point_group('c1')
    molecule.fix_orientation(True)
    molecule.update_geometry()

    nfrag = molecule.nfragments()
    if nfrag != 2:
        raise ValidationError('SAPT requires active molecule to have 2 fragments, not %s.' % (nfrag))

    monomerA = molecule.extract_subsets(1, 2)
    monomerA.set_name('monomerA')
    monomerB = molecule.extract_subsets(2, 1)
    monomerB.set_name('monomerB')
    molecule.update_geometry()
    monomerAm = molecule.extract_subsets(1)
    monomerAm.set_name('monomerAm')
    monomerBm = molecule.extract_subsets(2)
    monomerBm.set_name('monomerBm')

    ri = PsiMod.get_option('SCF', 'SCF_TYPE')
    df_ints_io = PsiMod.get_option('SCF', 'DF_INTS_IO')
    # inquire if above at all applies to dfmp2

    PsiMod.IO.set_default_namespace('dimer')
    PsiMod.set_local_option('SCF', 'SAPT', '2-dimer')
    PsiMod.print_out('\n')
    banner('Dimer HF')
    PsiMod.print_out('\n')
    PsiMod.set_global_option('DF_INTS_IO', 'SAVE')
    e_dimer = scf_helper('RHF', **kwargs)
    PsiMod.set_global_option('DF_INTS_IO', 'LOAD')

    activate(monomerA)
    if (ri == 'DF'):
        PsiMod.IO.change_file_namespace(97, 'dimer', 'monomerA')
    PsiMod.IO.set_default_namespace('monomerA')
    PsiMod.set_local_option('SCF', 'SAPT', '2-monomer_A')
    PsiMod.print_out('\n')
    banner('Monomer A HF (Dimer Basis)')
    PsiMod.print_out('\n')
    e_monomerA = scf_helper('RHF', **kwargs)

    activate(monomerB)
    if (ri == 'DF'):
        PsiMod.IO.change_file_namespace(97, 'monomerA', 'monomerB')
    PsiMod.IO.set_default_namespace('monomerB')
    PsiMod.set_local_option('SCF', 'SAPT', '2-monomer_B')
    PsiMod.print_out('\n')
    banner('Monomer B HF (Dimer Basis)')
    PsiMod.print_out('\n')
    e_monomerB = scf_helper('RHF', **kwargs)
    PsiMod.set_global_option('DF_INTS_IO', df_ints_io)

    activate(monomerAm)
    PsiMod.IO.set_default_namespace('monomerAm')
    PsiMod.set_local_option('SCF', 'SAPT', '2-monomer_A')
    PsiMod.print_out('\n')
    banner('Monomer A HF (Monomer Basis)')
    PsiMod.print_out('\n')
    e_monomerA = scf_helper('RHF', **kwargs)

    activate(monomerBm)
    PsiMod.IO.set_default_namespace('monomerBm')
    PsiMod.set_local_option('SCF', 'SAPT', '2-monomer_B')
    PsiMod.print_out('\n')
    banner('Monomer B HF (Monomer Basis)')
    PsiMod.print_out('\n')
    e_monomerB = scf_helper('RHF', **kwargs)

    activate(molecule)
    PsiMod.IO.set_default_namespace('dimer')
    PsiMod.set_local_option('SAPT', 'E_CONVERGENCE', 10e-10)
    PsiMod.set_local_option('SAPT', 'D_CONVERGENCE', 10e-10)
    if (name.lower() == 'sapt0-ct'):
        PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'SAPT0')
    elif (name.lower() == 'sapt2-ct'):
        PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'SAPT2')
    elif (name.lower() == 'sapt2+-ct'):
        PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'SAPT2+')
    elif (name.lower() == 'sapt2+(3)-ct'):
        PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'SAPT2+3')
        PsiMod.set_local_option('SAPT', 'DO_THIRD_ORDER', False)
    elif (name.lower() == 'sapt2+3-ct'):
        PsiMod.set_local_option('SAPT', 'SAPT_LEVEL', 'SAPT2+3')
        PsiMod.set_local_option('SAPT', 'DO_THIRD_ORDER', True)
    PsiMod.print_out('\n')
    banner('SAPT Charge Transfer')
    PsiMod.print_out('\n')

    # if the df_basis_sapt basis is not set, pick a sensible one.
    if PsiMod.get_global_option('DF_BASIS_SAPT') == '':
        ribasis = corresponding_rifit(PsiMod.get_global_option('BASIS'))
        if ribasis:
            PsiMod.set_global_option('DF_BASIS_SAPT', ribasis)
            PsiMod.print_out('No DF_BASIS_SAPT auxiliary basis selected, defaulting to %s\n' % (ribasis))
        else:
            raise ValidationError('Keyword DF_BASIS_SAPT is required.')

    PsiMod.print_out('\n')
    banner('Dimer Basis SAPT')
    PsiMod.print_out('\n')
    PsiMod.IO.change_file_namespace(121, 'monomerA', 'dimer')
    PsiMod.IO.change_file_namespace(122, 'monomerB', 'dimer')
    e_sapt = PsiMod.sapt()
    CTd = PsiMod.get_variable('SAPT CT ENERGY')

    PsiMod.print_out('\n')
    banner('Monomer Basis SAPT')
    PsiMod.print_out('\n')
    PsiMod.IO.change_file_namespace(121, 'monomerAm', 'dimer')
    PsiMod.IO.change_file_namespace(122, 'monomerBm', 'dimer')
    e_sapt = PsiMod.sapt()
    CTm = PsiMod.get_variable('SAPT CT ENERGY')
    CT = CTd - CTm

    PsiMod.print_out('\n\n')
    PsiMod.print_out('    SAPT Charge Transfer Analysis\n')
    PsiMod.print_out('  -----------------------------------------------------------------------------\n')
    line1 = '    SAPT Induction (Dimer Basis)      %10.4lf mH    %10.4lf kcal mol^-1\n' % (CTd * 1000.0, CTd * physconst.psi_hartree2kcalmol)
    line2 = '    SAPT Induction (Monomer Basis)    %10.4lf mH    %10.4lf kcal mol^-1\n' % (CTm * 1000.0, CTm * physconst.psi_hartree2kcalmol)
    line3 = '    SAPT Charge Transfer              %10.4lf mH    %10.4lf kcal mol^-1\n\n' % (CT * 1000.0, CT * physconst.psi_hartree2kcalmol)
    PsiMod.print_out(line1)
    PsiMod.print_out(line2)
    PsiMod.print_out(line3)
    PsiMod.set_variable('SAPT CT ENERGY', CT)

    molecule.reset_point_group(user_pg)
    molecule.update_geometry()

    optstash.restore()
    return e_sapt


def run_mrcc(name, **kwargs):
    """Function that prepares environment and input files
    for a calculation calling Kallay's MRCC code.

    """
    # TODO: Check to see if we really need to run the SCF code.
    scf_helper(name, **kwargs)
    vscf = PsiMod.get_variable('SCF TOTAL ENERGY')

    # The parse_arbitrary_order method provides us the following information
    # We require that level be provided. level is a dictionary
    # of settings to be passed to PsiMod.mrcc
    if not('level' in kwargs):
        raise ValidationError('level parameter was not provided.')

    level = kwargs['level']

    # Fullname is the string we need to search for in iface
    fullname = level['fullname']

    # User can provide 'keep' to the method.
    # When provided, do not delete the MRCC scratch directory.
    keep = False
    if 'keep' in kwargs:
        keep = kwargs['keep']

    # Save current directory location
    current_directory = os.getcwd()

    # Need to move to the scratch directory, perferrably into a separate directory in that location
    psi_io = PsiMod.IOManager.shared_object()
    os.chdir(psi_io.get_default_path())

    # Make new directory specifically for mrcc
    mrcc_tmpdir = 'mrcc_' + str(os.getpid())
    if 'path' in kwargs:
        mrcc_tmpdir = kwargs['path']

    # Check to see if directory already exists, if not, create.
    if os.path.exists(mrcc_tmpdir) == False:
        os.mkdir(mrcc_tmpdir)

    # Move into the new directory
    os.chdir(mrcc_tmpdir)

    # Generate integrals and input file (dumps files to the current directory)
    PsiMod.mrcc_generate_input(level)

    # Load the fort.56 file
    # and dump a copy into the outfile
    PsiMod.print_out('\n===== Begin fort.56 input for MRCC ======\n')
    PsiMod.print_out(open('fort.56', 'r').read())
    PsiMod.print_out('===== End   fort.56 input for MRCC ======\n')

    # Close output file
    PsiMod.close_outfile()

    # Modify the environment:
    #    PGI Fortan prints warning to screen if STOP is used
    os.environ['NO_STOP_MESSAGE'] = '1'

    # Obtain user's OMP_NUM_THREADS so that we don't blow it away.
    omp_num_threads_found = 'OMP_NUM_THREADS' in os.environ
    if omp_num_threads_found == True:
        omp_num_threads_user = os.environ['OMP_NUM_THREADS']

    # If the user provided MRCC_OMP_NUM_THREADS set the environ to it
    if PsiMod.has_option_changed('MRCC', 'MRCC_OMP_NUM_THREADS') == True:
        os.environ['OMP_NUM_THREADS'] = str(PsiMod.get_option('MRCC', 'MRCC_OMP_NUM_THREADS'))

    # Call dmrcc, directing all screen output to the output file
    try:
        if PsiMod.outfile_name() == 'stdout':
            retcode = subprocess.call('dmrcc', shell=True)
        else:
            retcode = subprocess.call('dmrcc >> ' + current_directory + '/' + PsiMod.outfile_name(), shell=True)

        if retcode < 0:
            print('MRCC was terminated by signal %d' % -retcode, file=sys.stderr)
            exit(1)
        elif retcode > 0:
            print('MRCC errored %d' % retcode, file=sys.stderr)
            exit(1)

    except OSError as e:
        print('Execution failed: %s' % e, file=sys.stderr)
        exit(1)

    # Restore the OMP_NUM_THREADS that the user set.
    if omp_num_threads_found == True:
        if PsiMod.has_option_changed('MRCC', 'MRCC_OMP_NUM_THREADS') == True:
            os.environ['OMP_NUM_THREADS'] = omp_num_threads_user

    # Scan iface file and grab the file energy.
    e = 0.0
    for line in file('iface'):
        fields = line.split()
        m = fields[1]
        try:
            e = float(fields[5])
            if m == "MP(2)":
                m = "MP2"
            PsiMod.set_variable(m + ' TOTAL ENERGY', e)
            PsiMod.set_variable(m + ' CORRELATION ENERGY', e - vscf)
        except ValueError:
            continue

    # The last 'e' in iface is the one the user requested.
    PsiMod.set_variable('CURRENT ENERGY', e)
    PsiMod.set_variable('CURRENT CORRELATION ENERGY', e - vscf)

    # Load the iface file
    iface = open('iface', 'r')
    iface_contents = iface.read()

    # Delete mrcc tempdir
    os.chdir('..')
    try:
        # Delete unless we're told not to
        if (keep == False and not('path' in kwargs)):
            shutil.rmtree(mrcc_tmpdir)
    except OSError as e:
        print('Unable to remove MRCC temporary directory %s' % e, file=sys.stderr)
        exit(1)

    # Revert to previous current directory location
    os.chdir(current_directory)

    # Reopen output file
    PsiMod.reopen_outfile()

    # If we're told to keep the files or the user provided a path, do nothing.
    if (keep != False or ('path' in kwargs)):
        PsiMod.print_out('\nMRCC scratch files have been kept.\n')
        PsiMod.print_out('They can be found in ' + mrcc_tmpdir)

    # Dump iface contents to output
    PsiMod.print_out('\n')
    banner('Full results from MRCC')
    PsiMod.print_out('\n')
    PsiMod.print_out(iface_contents)

    return e


def run_cepa(name, **kwargs):
    """Function encoding sequence of PSI module calls for
    a cepa-like calculation.

    >>> energy('cepa(1)')

    """
    lowername = name.lower()
    kwargs = kwargs_lower(kwargs)

    # save user options
    optstash = OptionsState(
        ['TRANSQT2', 'WFN'],
        ['CEPA', 'CEPA_NO_SINGLES'])

    # override symmetry if integral direct
    if PsiMod.get_option('CEPA', 'CEPA_VABCD_DIRECT'):
        molecule = PsiMod.get_active_molecule()
        molecule.update_geometry()
        molecule.reset_point_group('c1')
        #molecule.fix_orientation(1)
        #molecule.update_geometry()

    # throw an exception for open-shells
    if (PsiMod.get_option('SCF', 'REFERENCE') != 'RHF'):
        raise ValidationError("Error: %s requires \"reference rhf\"." % lowername)

    # what type of cepa?
    if (lowername == 'cepa(0)'):
        PsiMod.set_local_option('CEPA', 'CEPA_LEVEL', 'CEPA0')
    if (lowername == 'cepa(1)'):
        PsiMod.set_local_option('CEPA', 'CEPA_LEVEL', 'CEPA1')
    if (lowername == 'cepa(2)'):
        #PsiMod.set_local_option('CEPA', 'CEPA_LEVEL', 'CEPA2')
        # throw an exception for cepa(2)
        PsiMod.print_out("\n")
        PsiMod.print_out("Error: %s not implemented\n" % lowername)
        PsiMod.print_out("\n")
    if (lowername == 'cepa(3)'):
        PsiMod.set_local_option('CEPA', 'CEPA_LEVEL', 'CEPA3')
    if (lowername == 'sdci'):
        PsiMod.set_local_option('CEPA', 'CEPA_LEVEL', 'CISD')
    if (lowername == 'dci'):
        PsiMod.set_local_option('CEPA', 'CEPA_LEVEL', 'CISD')
        PsiMod.set_local_option('CEPA', 'CEPA_NO_SINGLES', True)
    if (lowername == 'acpf'):
        PsiMod.set_local_option('CEPA', 'CEPA_LEVEL', 'ACPF')
    if (lowername == 'aqcc'):
        PsiMod.set_local_option('CEPA', 'CEPA_LEVEL', 'AQCC')

    PsiMod.set_local_option('TRANSQT2', 'WFN', 'CCSD')
    scf_helper(name, **kwargs)

    # If the scf type is DF, then the AO integrals were never generated
    if PsiMod.get_option('SCF', 'SCF_TYPE') == 'DF':
        mints = PsiMod.MintsHelper()
        mints.integrals()

    # only call transqt2() if (ac|bd) is not integral direct
    if PsiMod.get_option('CEPA', 'CEPA_VABCD_DIRECT') == False:
        PsiMod.transqt2()

    PsiMod.cepa()

    # restore options ( transqt2/wfn, cepa/cepa_no_singles )
    optstash.restore()

    return PsiMod.get_variable("CURRENT ENERGY")
