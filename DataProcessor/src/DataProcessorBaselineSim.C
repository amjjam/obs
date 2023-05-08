#include "../include/DataProcessorBaselineSim.H"

DataProcessorBaselineSim::DataProcessorBaselineSim
(std::string &name, int n, float L0, float L1,
 float A, float S, float d, long idum):
  name(name),L0(L0),L1(L1),A(A),S(S),d(d),phasors(n),idum(idum){
  phasors.rename(name);
  beta0=2*M_PI/L0;
  beta1=2*M_PI/L1;
  dbeta=(beta1-beta0)/(n-1);  
}

PhasorSet &DataProcessorBaselineSim::makePhasorSet(float dd){
  if(dd<-1e30)
    dd=d;
  float arg;
  for(unsigned int i=0;i<phasors.size();i++){
    arg=(beta0+dbeta*i)*dd;
    phasors[i].real(A*cos(arg)+S*gasdev(&idum));
    phasors[i].imag(A*sin(arg)+S*gasdev(&idum));
  }
  
  return phasors;
}

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

float ran1(long *idum)
{
	int j;
	long k;
	static long iy=0;
	static long iv[NTAB];
	float temp;

	if (*idum <= 0 || !iy) {
		if (-(*idum) < 1) *idum=1;
		else *idum = -(*idum);
		for (j=NTAB+7;j>=0;j--) {
			k=(*idum)/IQ;
			*idum=IA*(*idum-k*IQ)-IR*k;
			if (*idum < 0) *idum += IM;
			if (j < NTAB) iv[j] = *idum;
		}
		iy=iv[0];
	}
	k=(*idum)/IQ;
	*idum=IA*(*idum-k*IQ)-IR*k;
	if (*idum < 0) *idum += IM;
	j=iy/NDIV;
	iy=iv[j];
	iv[j] = *idum;
	if ((temp=AM*iy) > RNMX) return RNMX;
	else return temp;
}
#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX
/* (C) Copr. 1986-92 Numerical Recipes Software 1)0. */

#include <math.h>

float gasdev(long *idum)
{
	float ran1(long *idum);
	static int iset=0;
	static float gset;
	float fac,rsq,v1,v2;

	if  (iset == 0) {
		do {
			v1=2.0*ran1(idum)-1.0;
			v2=2.0*ran1(idum)-1.0;
			rsq=v1*v1+v2*v2;
		} while (rsq >= 1.0 || rsq == 0.0);
		fac=sqrt(-2.0*log(rsq)/rsq);
		gset=v1*fac;
		iset=1;
		return v2*fac;
	} else {
		iset=0;
		return gset;
	}
}
/* (C) Copr. 1986-92 Numerical Recipes Software 1)0. */
