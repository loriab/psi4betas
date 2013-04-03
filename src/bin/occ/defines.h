#ifndef DEFINES_
#define DEFINES_

#define PSIF_OCC_DPD 100
#define PSIF_OCC_DENSITY 101
#define PSIF_OCC_IABC 102

#define ID(x) ints->DPD_ID(x)

#define index2(i,j) ((i>j) ? ((i*(i+1)/2)+j) : ((j*(j+1)/2)+i))
#define index4(i,j,k,l) index2(index2(i,j),index2(k,l))

#define MIN0(a,b) (((a)<(b)) ? (a) : (b))
#define MAX0(a,b) (((a)>(b)) ? (a) : (b))

#define DIIS_MIN_DET 1.0E-16
#define DIVERGE 1.0E+3

#endif /* DEFINES_ */

