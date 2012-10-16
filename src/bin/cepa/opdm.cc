#include<libmints/mints.h>
#include<psifiles.h>
#include"blas.h"
#include"coupledpair.h"

using namespace psi;

namespace psi{ namespace cepa{
//void OPDM(boost::shared_ptr<psi::cepa::CoupledPair>cepa,Options&options);

double Normalize(long int o,long int v,double*t1,double*t2,int cepa_level);
void BuildD1(long int nfzc,long int o,long int v,long int nfzv,double*t1,double*ta,double*tb,double c0,double*D1);

void CoupledPair::OPDM(){

  long int o = ndoccact;
  long int v = nvirt;

  // if t2 was stored on disk, grab it.
  if (t2_on_disk){
     boost::shared_ptr<PSIO> psio(new PSIO());
     psio->open(PSIF_DCC_T2,PSIO_OPEN_OLD);
     psio->read_entry(PSIF_DCC_T2,"t2",(char*)&tempv[0],o*o*v*v*sizeof(double));
     psio->close(PSIF_DCC_T2,1);
     tb = tempv;
  }

  // Normalize wave function and return leading coefficient 
  double c0 = Normalize(o,v,t1,tb,cepa_level);

  // Build 1-RDM
  int nmo = o+v+nfzc+nfzv;
  double*D1 = (double*)malloc(nmo*nmo*sizeof(double));
  BuildD1(nfzc,o,v,nfzv,t1,integrals,tb,c0,D1);
  // Call oeprop
  boost::shared_ptr<OEProp> oe(new OEProp());
  boost::shared_ptr<Matrix> Ca = reference_wavefunction_->Ca();

  std::stringstream ss;
  ss << cepa_type;
  std::stringstream ss_a;
  ss_a << ss.str() << " alpha";

  // pass opdm to oeprop
  SharedMatrix opdm_a(new Matrix(ss_a.str(), Ca->colspi(), Ca->colspi()));

  // mapping array for D1(c1) -> D1(symmetry)
  int *irrepoffset = (int*)malloc(nirrep_*sizeof(double));
  irrepoffset[0] = 0;
  for (int h=1; h<nirrep_; h++){
      irrepoffset[h] = irrepoffset[h-1] + nmopi_[h-1];
  }
  int *reorder = (int*)malloc(nmo*sizeof(int));
  int mo_offset = 0;
  int count = 0;

  // frozen core
  for (int h=0; h<nirrep_; h++){
      int norbs = frzcpi_[h];
      for (int i=0; i<norbs; i++){
          reorder[irrepoffset[h] + i] = count++;
      }
  }
  // active doubly occupied
  for (int h=0; h<nirrep_; h++){
      int norbs = doccpi_[h]-frzcpi_[h];
      for (int i=0; i<norbs; i++){
          reorder[irrepoffset[h] + i + frzcpi_[h]] = count++;
      }
  }
  // active virtual
  for (int h=0; h<nirrep_; h++){
      int norbs = nmopi_[h]-frzvpi_[h]-doccpi_[h];
      for (int i=0; i<norbs; i++){
          reorder[irrepoffset[h] + i + doccpi_[h]] = count++;
      }
  }
  // frozen virtual
  for (int h=0; h<nirrep_; h++){
      int norbs = frzvpi_[h];
      for (int i=0; i<norbs; i++){
          reorder[irrepoffset[h] + i + nmopi_[h]-frzvpi_[h]] = count++;
      }
  }

  // pass opdm to oeprop (should be symmetry-tolerant)
  for (int h=0; h<nirrep_; h++){
      double** opdmap = opdm_a->pointer(h);
      for (int i=0; i<nmopi_[h]; i++) {
          int ii = reorder[irrepoffset[h]+i];
          for (int j=0; j<nmopi_[h]; j++){
              int jj = reorder[irrepoffset[h]+j];
              opdmap[i][j] = D1[ii*nmo+jj];
          }
      }
  }
  free(reorder);
  free(irrepoffset);

  oe->set_Da_mo(opdm_a);

  std::stringstream oeprop_label;
  oeprop_label << cepa_type;
  oe->set_title(oeprop_label.str());
  oe->add("DIPOLE");
  oe->add("MULLIKEN_CHARGES");
  oe->add("NO_OCCUPATIONS");
  if (options_.get_int("PRINT") > 1) {
      oe->add("QUADRUPOLE");
  }
   
  fprintf(outfile, "\n");
  fprintf(outfile, "  ==> Properties %s <==\n", ss.str().c_str());
  oe->compute();

  char*line = (char*)malloc(100*sizeof(char));

  std::stringstream ss2;
  ss2 << oeprop_label.str() << " DIPOLE X";
  if (cepa_level == 0){
     Process::environment.globals["CEPA(0) DIPOLE X"] =
       Process::environment.globals[ss2.str()];
  }
  if (cepa_level == -1){
     Process::environment.globals["CISD DIPOLE X"] =
       Process::environment.globals[ss2.str()];
  }
  else if (cepa_level == -2){
     Process::environment.globals["ACPF DIPOLE X"] =
       Process::environment.globals[ss2.str()];
  }
  else if (cepa_level == -3){
     Process::environment.globals["AQCC DIPOLE X"] =
       Process::environment.globals[ss2.str()];
  }

  ss2.str(std::string());
  ss2 << oeprop_label.str() << " DIPOLE Y";
  if (cepa_level == 0){
     Process::environment.globals["CEPA(0) DIPOLE Y"] =
       Process::environment.globals[ss2.str()];
  }
  if (cepa_level == -1){
     Process::environment.globals["CISD DIPOLE Y"] =
       Process::environment.globals[ss2.str()];
  }
  else if (cepa_level == -2){
     Process::environment.globals["ACPF DIPOLE Y"] =
       Process::environment.globals[ss2.str()];
  }
  else if (cepa_level == -3){
     Process::environment.globals["AQCC DIPOLE Y"] =
       Process::environment.globals[ss2.str()];
  }

  ss2.str(std::string());
  ss2 << oeprop_label.str() << " DIPOLE Z";
  if (cepa_level == 0){
     Process::environment.globals["CEPA(0) DIPOLE Z"] =
       Process::environment.globals[ss2.str()];
  }
  if (cepa_level == -1){
     Process::environment.globals["CISD DIPOLE Z"] =
       Process::environment.globals[ss2.str()];
  }
  else if (cepa_level == -2){
     Process::environment.globals["ACPF DIPOLE Z"] =
       Process::environment.globals[ss2.str()];
  }
  else if (cepa_level == -3){
     Process::environment.globals["AQCC DIPOLE Z"] =
       Process::environment.globals[ss2.str()];
  }

  if (options_.get_int("PRINT")>1){

     ss2.str(std::string());
     ss2 << oeprop_label.str() << " QUADRUPOLE XX";
     if (cepa_level == 0){
        Process::environment.globals["CEPA(0) QUADRUPOLE XX"] =
          Process::environment.globals[ss2.str()];
     }
     if (cepa_level == -1){
        Process::environment.globals["CISD QUADRUPOLE XX"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -2){
        Process::environment.globals["ACPF QUADRUPOLE XX"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -3){
        Process::environment.globals["AQCC QUADRUPOLE XX"] =
          Process::environment.globals[ss2.str()];
     }

     ss2.str(std::string());
     ss2 << oeprop_label.str() << " QUADRUPOLE YY";
     if (cepa_level == 0){
        Process::environment.globals["CEPA(0) QUADRUPOLE YY"] =
          Process::environment.globals[ss2.str()];
     }
     if (cepa_level == -1){
        Process::environment.globals["CISD QUADRUPOLE YY"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -2){
        Process::environment.globals["ACPF QUADRUPOLE YY"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -3){
        Process::environment.globals["AQCC QUADRUPOLE YY"] =
          Process::environment.globals[ss2.str()];
     }

     ss2.str(std::string());
     ss2 << oeprop_label.str() << " QUADRUPOLE ZZ";
     if (cepa_level == 0){
        Process::environment.globals["CEPA(0) QUADRUPOLE ZZ"] =
          Process::environment.globals[ss2.str()];
     }
     if (cepa_level == -1){
        Process::environment.globals["CISD QUADRUPOLE ZZ"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -2){
        Process::environment.globals["ACPF QUADRUPOLE ZZ"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -3){
        Process::environment.globals["AQCC QUADRUPOLE ZZ"] =
          Process::environment.globals[ss2.str()];
     }


     ss2.str(std::string());
     ss2 << oeprop_label.str() << " QUADRUPOLE XY";
     if (cepa_level == 0){
        Process::environment.globals["CEPA(0) QUADRUPOLE XY"] =
          Process::environment.globals[ss2.str()];
     }
     if (cepa_level == -1){
        Process::environment.globals["CISD QUADRUPOLE XY"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -2){
        Process::environment.globals["ACPF QUADRUPOLE XY"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -3){
        Process::environment.globals["AQCC QUADRUPOLE XY"] =
          Process::environment.globals[ss2.str()];
     }


     ss2.str(std::string());
     ss2 << oeprop_label.str() << " QUADRUPOLE XZ";
     if (cepa_level == 0){
        Process::environment.globals["CEPA(0) QUADRUPOLE XZ"] =
          Process::environment.globals[ss2.str()];
     }
     if (cepa_level == -1){
        Process::environment.globals["CISD QUADRUPOLE XZ"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -2){
        Process::environment.globals["ACPF QUADRUPOLE XZ"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -3){
        Process::environment.globals["AQCC QUADRUPOLE XZ"] =
          Process::environment.globals[ss2.str()];
     }

     ss2.str(std::string());
     ss2 << oeprop_label.str() << " QUADRUPOLE YZ";
     if (cepa_level == 0){
        Process::environment.globals["CEPA(0) QUADRUPOLE YZ"] =
          Process::environment.globals[ss2.str()];
     }
     if (cepa_level == -1){
        Process::environment.globals["CISD QUADRUPOLE YZ"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -2){
        Process::environment.globals["ACPF QUADRUPOLE YZ"] =
          Process::environment.globals[ss2.str()];
     }
     else if (cepa_level == -3){
        Process::environment.globals["AQCC QUADRUPOLE YZ"] =
          Process::environment.globals[ss2.str()];
     }

  }

  free(D1);
}

// build the 1-electron density
void BuildD1(long int nfzc,long int o,long int v,long int nfzv,double*t1,double*ta,double*tb,double c0,double*D1){
  long int id,i,j,k,a,b,c,sg,p,count,sg2,nmo=o+v+nfzc+nfzv;
  double sum;
  memset((void*)D1,'\0',nmo*nmo*sizeof(double));
  double*tempd = (double*)malloc(v*v*sizeof(double));

  for (i=0; i<nfzc; i++){
      D1[i*nmo+i] = 1.0;
  }

  F_DCOPY(o*o*v*v,tb,1,ta,1);
  for (a=0,id=0; a<v; a++){
      for (b=0; b<v; b++){
          for (i=0; i<o; i++){
              for (j=0; j<o; j++){
                  ta[id++] -= tb[b*o*o*v+a*o*o+i*o+j];
              }
          }
      }
  }

  // D(a,b)
  F_DGEMM('t','n',v,v,o*o*v,1.0,tb,o*o*v,tb,o*o*v,0.0,tempd,v);
  F_DGEMM('t','n',v,v,o*o*v,0.5,ta,o*o*v,ta,o*o*v,1.0,tempd,v);
  F_DGEMM('t','n',v,v,o,1,t1,o,t1,o,1.0,tempd,v);
  for (a=0; a<v; a++){
      for (b=0; b<v; b++){
          D1[(a+o+nfzc)*nmo+(b+o+nfzc)] = tempd[a*v+b];
      }
  }
 
  // D(i,j)
  F_DGEMM('n','t',o,o,o*v*v,-1.0,tb,o,tb,o,0.0,tempd,o);
  F_DGEMM('n','t',o,o,o*v*v,-0.5,ta,o,ta,o,1.0,tempd,o);
  F_DGEMM('n','t',o,o,v,-1.0,t1,o,t1,o,1.0,tempd,o);
  for (i=0; i<o; i++){
      for (j=0; j<o; j++){
          D1[(i+nfzc)*nmo + j+nfzc] = tempd[i*o+j];
      }
      D1[(i+nfzc)*nmo+i+nfzc] += 1.0;
  }

  // hmmm ... i could do this as a dgemv, but i'd have sort ta and tb
  for (i=0; i<o; i++){
      for (a=0; a<v; a++){
          sum = t1[a*o+i]*c0;
          for (j=0; j<o; j++){
              for (b=0; b<v; b++){
                  sum += t1[b*o+j]*tb[a*o*o*v+b*o*o+i*o+j];
                  sum += t1[b*o+j]*ta[a*o*o*v+b*o*o+i*o+j];
              }
          }
          D1[(i+nfzc)*nmo+a+o+nfzc] = D1[(a+o+nfzc)*nmo+i+nfzc] = sum;
      }
  }

  free(tempd);
}

// normalize the wave function and return the leading coefficient
double Normalize(long int o,long int v,double*t1,double*t2,int cepa_level){

  if (cepa_level == 0) return 1.0;

  double nrm = 1.0;
  double fac = 1.0;
  if (cepa_level == -2)      fac = 1.0/o;
  else if (cepa_level == -3) fac = 1.0-(2.0*o-2.0)*(2.0*o-3.0) / (2.0*o*(2.0*o-1.0));

  long int i,j,a,b,id;
  double dum,sum=0.;

  for (a=0,id=0; a<v; a++){
      for (b=0; b<v; b++){
          for (i=0; i<o; i++){
              for (j=0; j<o; j++){
                  dum  = t2[id++];
                  sum -= dum*dum;
                  dum -= t2[b*o*o*v+a*o*o+i*o+j];
                  sum -= .5*dum*dum;
              }
          }
      }
  }
  for (a=0,id=0; a<v; a++){
     for (i=0; i<o; i++){
          dum  = t1[id++];
          sum -= 2.*dum*dum;
      }
  }
  nrm = sqrt(1.0-fac*sum);

  for (i=0; i<o*o*v*v; i++) t2[i] /= nrm;
  for (i=0; i<o*v; i++)     t1[i] /= nrm;

  return 1.0/nrm;
}


}}// end of namespace psi

