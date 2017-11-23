/****************************************************************************\
 $URL$
 $LastChangedDate$
 $LastChangedRevision$
 $LastChangedBy$
\****************************************************************************/
/****************************************************************************\
Soil column stored as list of layers
(c) J�rgen E. Olesen, Statens Planteavlsfors�g
Changes:
   JEO 23-06-1997: GetPoolCarbon and GetPoolNitrogen added
   JEO 08-07-1997: GetAmmoniumLeaching and GetNitrateLeaching added
   JEO 27-10-1997: New soil evaporation formula
   JB  04_02-1998: RunOff added
\****************************************************************************/

#ifndef SOILPROF
   #define SOILPROF

#include "../base/base.h"
#include "soilLayer.h"
#include "../base/linklist.h"
#include "../tools/convert.h"
class soilProfile : public base
{
 private:
	 convert conv;
 enum {ExtraLayers = 3};

 soilLayer * first; // V�K SENERE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 soilLayer * last;  // V�K SENERE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

 double minTemp[12];
 int lastErrorYear;
 int NumOfSoilLayers;
 soilLayer * soilLayerArray[MaxSoilLayers];
 fstream SoilTempFile;
 nitrogen nitrogenUptake[MaxPlants];
 double maxRootDepth;    // Maximum root depth [mm]
 double distanceToPreviousLayer[MaxSoilLayers], distanceToNextLayer[MaxSoilLayers];
 double DistPreviousLayerInv[MaxSoilLayers], DistNextLayerInv[MaxSoilLayers];                  // Only used for speed optimization
 double temperature[MaxSoilLayers];
 double heatConductivity[MaxSoilLayers];
 double heatConductivitySlope[MaxSoilLayers];
 double heatCapacity[MaxSoilLayers];
 double SpecWaterCapacity[MaxSoilLayers];
 double infiltration;
 double cum_infiltration;
 // Parameters
 double EvapExtCoef;          // Extinction coefficient for evaporation [/mm]
 double EvaporationCapacity;  // Capacity [mm] of evaporation layer

 // State variables
 double EvaporationContent;   // Content [mm] of evaporation layer
 double depthDampning;        // Dampning depth for calculation of laower boundary condition of the temperature solution
 bool calculateAverage;       // To calculate initial average thermal conductivity and heat capacity - but only once. Used in lower boundary condition.

 void EstimateTotalRoot(rootStructure* TotalRoot,linkList<rootStructure>* rootList);
 void ResourceCompetition(int ResourceNumber, int AttackerNumber, double* Demand, double* Apparancy, double* Resource, double* ResourceRemoved);
 void AssignMaxRootDepth();

 void CalculateDeltaTemp(double* temperature,double *waterFluxSlope,
				double *waterFlux, double* deltaTemp);


 public:
 soilProfile(const char * Pname,const int Index,const base * owner);
 soilProfile(const soilProfile& Profile);
 ~soilProfile();

 void CheckCoherence(int checkpoint); // TEST - REMOVE !!!!!!!!!!!!!
 virtual void    Initialize(char * soilFileName);
 virtual void    Add(soilProfile* Profile,double fraction);
 virtual void    AddItem(soilLayer * data);
 virtual void    AddNutrient(nitrogen soilNitrate, nitrogen soilAmmonium);
 virtual void    UpdateTemperature(double temp, double snowContent);
 virtual void    UpdateFinDiffTemperature(double temp, double snowContent);

 virtual void	  Tridag(double a[], double b[], double c[], double r[], double u[], unsigned long n);
 virtual double  UpdateOrganicMatter();

 // Water functions
 virtual double  MaxTranspiration(linkList<rootStructure>* rootList);
 virtual double  GetMaximumEvaporation();
 virtual void    UpdateInfiltration(double * surplus,nitrogen * surfaceNitrate,
                                    nitrogen * surfaceAmmonium,nitrogen *NitrateLeached,
                                    nitrogen *AmmoniumLeached, double Chloride);
 virtual void 	  UpdateInfiltrationSWAT(double *surplus, nitrogen * surfaceNitrate,
                                     nitrogen * surfaceAmmonium, nitrogen *NitrateLeached,
                                     nitrogen *AmmoniumLeached, double Chloride);
 virtual void    SubtractTranspiration(linkList <rootStructure>* rootList);
 virtual void    SubtractEvaporation(double soilEvaporation);
 virtual double Getinfiltration(){ return  cum_infiltration;}

 // Nitrogen functions
 nitrogen* SubtractNitrogenUptake(linkList<rootStructure>* rootList);
 nitrogen  UpdateNitrification();
 nitrogen  UpdateDenitrification(double TotalCO2Emission);
 nitrogen  GetOrganicNitrogen();
 double  GetOrganicNitrogenN();
 nitrogen  GetOrganicNitrogen(double startDep, double thick);
 double  GetOrganicNitrogenN(double startDep, double thick);
 double  GetOrganicNitrogenN15(double startDep, double thick);

 double  GetPoolNitrogenN(int Name,double startDep,double thick);
 double  GetPoolNitrogenN15(int Name,double startDep,double thick);
 nitrogen  GetAmmoniumLeaching(double depth);
 nitrogen  GetTotalNitrogen();
 double  GetTotalNitrogenN();
 nitrogen  GetNitrateLeaching(double depth);
 nitrogen  GetAmmonium();
 double  GetAmmoniumN();
 nitrogen  GetNitrate();
 double  GetNitrateN();
 nitrogen  GetAmmonium(double startDep,double thick);
 double  GetAmmoniumN(double startDep,double thick);
 double  GetAmmoniumN15(double startDep,double thick);
 nitrogen  GetNitrate(double startDep,double thick);
 double  GetNitrateN(double startDep,double thick);
 double  GetNitrateN15(double startDep,double thick);
 nitrogen  GetN2OFromNitrification();
 nitrogen  GetN2OFromDenitrification();
 double  GetChloride(double startDep,double thick);

 // Get functions
 virtual double  GetDiscreteDepth(double depth); // Returns a number >= depth, s� that the returned depth does not "divide" two layers
 soilLayer* GetLayer(int i) {return soilLayerArray[i];};
 virtual void    UpdateT(double * soilTemperature,double airTemperature,double depth,double meanAirTemperature);
 virtual void    UpdateTemperature(const base* aOwner);
 virtual double  GetTemperature(double depth);
 virtual double  GetPressurePotential(double depth);
 virtual double  GetTotalWater();
 virtual double  GetTotalWater(double startDepth,double thickness);
 virtual double  GetAvailWater(double startDepth,double thickness);
 virtual double  GetAvailCapacity(double startDepth,double thickness);
 virtual double  GetFieldCapacity(double startDepth,double thickness);
 virtual double  GetWiltCapacity(double startDepth,double thickness);
 int             GetNumberOfLayers();
 double 			  GetMaximumDepth();
 double          GetLayerStartDepth(int No);
 double          GetLayerThickness(int No);
 double          GetCarbon();
 double          GetOrganicCarbon(double startDep, double thick);

 double          GetPoolCarbon(int Name,double startDep,double thick);
  double          GetSoilMass(double startDep, double thick);

 double          GetDrainage(double depth);

 double          GetEvaporationContent(){return EvaporationContent;};
 double          GetMaxRootDepth() {return maxRootDepth;};
 double          GetClayContent(double startdepth, double thick);

 // Misc. functions
 void            AddProduct(const organicProduct& product,double * rootLengthList);
 void            StartBudget();
 void            EndBudget();
  void			WriteC();
 void				WriteCarbon(int Name,bool usedBefores);
void				WriteN(int Name,bool usedBefores);
void 				WriteCN(int Name,bool usedBefores);
 void	WriteDecompositionRate(int Name,bool usedBefores);
 void			changeLayer(int starLayer,int endLayer);

  void	BeforeAmmonium();
 void PrintAmmonium();
  void  BeforeNitrate();
  void PrintNitrate();
  string IntToName(int name);
};

#endif

