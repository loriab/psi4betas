#ifndef HCTH120_functional_h
#define HCTH120_functional_h
/**********************************************************
* HCTH120_functional.h: declarations for HCTH120_functional for KS-DFT
* Robert Parrish, robparrish@gmail.com
* Autogenerated by MATLAB Script on 06-Mar-2012
*
***********************************************************/
#include "functional.h"

namespace psi { namespace functional {

class HCTH120_Functional : public Functional {
public:
    HCTH120_Functional(int npoints, int deriv);
    virtual ~HCTH120_Functional();
    virtual void computeRKSFunctional(boost::shared_ptr<RKSFunctions> prop);
    virtual void computeUKSFunctional(boost::shared_ptr<UKSFunctions> prop);
};
}}
#endif

