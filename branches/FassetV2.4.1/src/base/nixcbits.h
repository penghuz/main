#ifndef NixC
	#define NixC

typedef double temp_double_array[2000];
typedef int temp_int_array[2000];
typedef void * temp_ptr_array[2000];



double GetFrictionVelocity(double height, double displacementHt, double zeroPlane, double windVelocity);
//! returns density of dry air in kg per cubic meter
/*
*\param air pressure in Pa 
*\param temperature in K
*/
double GetdensityDryAir(double airPressure, double temperature); 
//! returns density of dry air in kg per cubic meter
/*
*\param air pressure in Pa 
*\param temperature in K
*\param vapourPressure in Pa
*/
double GetdensityAir(double airPressure, double temperature, double vapourPressure);

double GetsaturatedWaterVapourPressure(double airTemperature);

void swapdoub(double& a , double& b);
void swappoint(void * &a, void * &b);


void quicksort2(temp_double_array a, temp_ptr_array b, int left, int right);
void Spline(double * x, double * y, int n, double * y2);
double Splint(double * xa, double * ya, double * y2a, int n, double x);
double LinearInterpolation(double *x, double *y,int n, double x0);

void SolveCubic(double  a,
                double  b,
                double  c,
                double  d,
                int    *solutions,
                double *x);

//various meteorological functions
 //! returns the specific heat capacity of dry air in J per K 
double GetspecificHeatCapAir();

//! Return the downward longwave radiation for a given sky temperature
   /*!
    * \param skyTemp sky temperature in Kelvin
    * 
	*/
double GetlongWaveDown(double skyTemp);
//!Return the upward longwave radiation from a body
/*!
    * \param aTemperature temperature of body surface in Kelvin
	* \param emissivity body surface emissivity
    * 
*/
double GetlongWaveUp(double emissivity, double aTemperature);

double GetdayLength(double latitude, int dayNo);
double GettimeFromDarkness(double latitude, int dayNo, int hour, int mins, double offset);
double GetDeclination(int dayNo);
bool SolveQuadratic(bool posRoot, double a, double b, double c, double *x);


int newGap(int gap);
void Comsort(temp_double_array double_array, temp_ptr_array ptr_array, int Hi);
double N_to_FE(double orgNcontent);
double GetWindAtHeight(double origHt, double origWind, double newHt, double roughnessLength);
double GetGrossEnergyinDryMatter(); //MJ per kg

#endif


