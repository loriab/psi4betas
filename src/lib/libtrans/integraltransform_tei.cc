#include "integraltransform.h"
#include <libchkpt/chkpt.hpp>
#include <libpsio/psio.hpp>
#include <libciomr/libciomr.h>
#include <libiwl/iwl.hpp>
#include <libqt/qt.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include "psifiles.h"
#include "mospace.h"
#define EXTERN
#include <libdpd/dpd.gbl>

using namespace boost;
using namespace psi;

/**
 * Transform the two-electron integrals from the SO to the MO basis in the spaces specified
 *
 * @param s1 - the MO space for the first index
 * @param s2 - the MO space for the second index
 * @param s3 - the MO space for the third index
 * @param s4 - the MO space for the fourth index
 */
void
IntegralTransform::transform_tei(const shared_ptr<MOSpace> s1, const shared_ptr<MOSpace> s2,
                                 const shared_ptr<MOSpace> s3, const shared_ptr<MOSpace> s4,
                                 HalfTrans ht)
{
    check_initialized();
    // Only do the first half if the "make" flag is set
    if(ht == MakeAndKeep || ht == MakeAndNuke)
        transform_tei_first_half(s1, s2);

    if(ht == ReadAndNuke || ht == MakeAndNuke){
        keepHtInts_ = false;
    }else{
        keepHtInts_ = true;
    }
    transform_tei_second_half(s1, s2, s3, s4);
}
