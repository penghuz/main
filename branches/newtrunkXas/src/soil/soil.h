#ifndef SOIL
 #define SOIL

#include "../base/base.h"
#include "soilProfile.h"
#include "soilLayer.h"
#include "soilSurface.h"
#include "../products/organicProduct.h"
#include "../products/decomposable.h"
#include "../base/budget.h"


class soil : public base
{
 private:
 //!assignment operator
 soil& operator=(const soil& s); // Dissable the compilers generation of default assignment operator.
 soil();
 protected:
  //!file streams for details of soil variables.
   fstream f;
   int depth;
 //! object describing compounds on the soil surface (e.g. undissolved fertiliser)
 soilSurface       * Surface;      		// Soil surface
 //! object describing the soil profile
 soilProfile       * Profile;       	// Soil profile
 double              Potassium;     	// Potassium (K) [g/m2] - presently unused
 double              Phosphorus;    	// Phosphorus (P) [g/m2] - presently unused
 // Parameters
 double evaporationDepth;           	// Evaporation extraction depth [mm]
// state variables
 //double evapotranspiration;         	// Actual evapotranspiration [mm/d]
 double totalTranspiration;         	// the total water uptake by plant [mm/d]
 double soilEvaporation;               // Soil evaporation [mm/d]
 double AccumulatedWaterDrainage;      // Accumulated water drainage (mm)
 double AccumulatedNitrateDrainage;    // Accumulated nitrate drainage [g N/m2]
 double CO2Evolution;                  // Daily CO2 evolution (g-C m-2)

 nitrogen nitrateLeached;             	// Nitrate leached from soil profile [g N/m�/d]
 nitrogen ammoniumLeached;            	// Ammonium leached from soil profile [g N/m�/d]
 nitrogen denitrification;            	// Nitrogen lost by denitrification [g N/m�/d]
 nitrogen nitrification;               // Daily nitrification [g N/m�/d]
 nitrogen NitrogenNetMineralisation;  	// net nitrogen mineralisation [g/m2]
 nitrogen* nitrogenUptake;             // Nitrogen uptake by plant roots

 int ObsDate;
 double ObsStartDepth,ObsEndDepth,ObsNmin; // Observations (Note only one single Nmin observations allowed!)
 //!pointer to a soil object. Only used when simulating multiple patches within a field
 base* MasterSoil;
 //! soil nitrogen budget
 budget Nbudget,
 //!soil water budget
	WaterBudget;            // budgets

 public:
 //!constructor with arguments
 soil(const char * Pname,const int Index,const base * owner);
 //! destructor
 ~soil();
 //!copy constructor
 soil(const soil& Soil);
 //!set pointer to master soil object
 void SetMasterSoil(base* aMasterSoil) {MasterSoil = aMasterSoil;};
 //! initialise soil module
 virtual void   Initialize(char * soilFileName);
 //!add one soil object to another
 /*!
 \param addSoil pointer to the soil object to be added
 \param  fraction weighting factor to be used when doing the addition
 */
 virtual void Add(soil* addSoil, double fraction);
 //!adds nitrogen to the soil
 /*!adds nitrogen to the soil in the form of nitrate and ammonium, with partitioning between the soil surface
 and the top layer of the soil.
 \param  surfNitrate  Nitrate to be added to soil surface (grams of nitrogen per metre squared)
 \param surfAmmonium  Ammonium to be added to soil surface (grams of nitrogen per metre squared)
 \param soilNitrate Nitrate to be added to top of soil profile (grams of nitrogen per metre squared)
 \param soilAmmonium Ammonium to be added to top of soil profile (grams of nitrogen per metre squared)
 */
 virtual void   AddNutrient(nitrogen surfNitrate,
									 nitrogen surfAmmonium,
									 nitrogen soilNitrate,
									 nitrogen soilAmmonium);
 //! and chloride to the soil
 virtual void   AddChloride(double amount) {Surface->AddChloride(amount);};
 //!returns the temperature at a given depth in the profile
 virtual double GetTemperature(double depth) {return Profile->GetTemperature(depth);};
 //!daily update for the soil
 /*!
 \param    EPotToSoil      - potential evapotranspiration allocated to soil
 \param   surplus         - water added to soil [mm/d]
  \param  airTemperature  - air temperature [C]
  \param  snowContent     - snow content
  \param  roots           - root descriptor list
  */
 virtual void Update(double surplus,
   						double airTemperature,
	   				   double EPotToSoil,
                     double snowContent,
					 linkList<rootStructure>* roots);
 //!returns number of soil layers and
 int    GetLayers() {return Profile->GetNumberOfLayers();};
 //!returns starting depths of a given layer (millimetres)
 double GetLayerStartDepth(int No) {return Profile->GetLayerStartDepth(No);};
 //! returned to the thickness of a given layer ( millimetres)
 double GetLayerThickness(int No)
 {
  if (No<Profile->GetNumberOfLayers())
   return Profile->GetLayerThickness(No);
  else
   return 0.0;
 };
 //!adds decomposable organic matter to the soil
 void AddDecomposable(decomposable* produc,
                      double * rootLengthList = nullptr);
 // Water functions

 double GetWater() {return Profile->GetTotalWater();};
 double GetWater(double startDepth,double thickness)         {return Profile->GetTotalWater(startDepth,thickness);};
 double GetSurfaceWater()                                    {return Surface->GetWater();};
 double GetAvailWater(double startDepth,double thickness)    {return Profile->GetAvailWater(startDepth,thickness);};
 double GetAvailCapacity(double startDepth,double thickness) {return Profile->GetAvailCapacity(startDepth,thickness);};
 double GetFieldCapacity(double startDepth,double thickness) {return Profile->GetFieldCapacity(startDepth,thickness);};
 double GetWiltCapacity(double startDepth,double thickness)  {return Profile->GetWiltCapacity(startDepth,thickness);};
 double GetSoilEvaporation() {return soilEvaporation;};
 double GetDrainage(double depth) {return Profile->GetDrainage(depth);};

 // Nitrogen functions
 nitrogen GetMineralNitrogenSurface(){return Surface->GetAmmoniumDisolved()+Surface->GetNitrateDisolved()
                                       +Surface->GetAmmoniumSolid()+Surface->GetNitrateSolid();};
 nitrogen GetOrganicNitrogenSurface(){return Surface->GetOrganicNitrogen();};
 nitrogen GetMineralNitrogenProfile(){return Profile->GetAmmonium()+Profile->GetNitrate();};
 nitrogen GetOrganicNitrogenProfile(){return Profile->GetOrganicNitrogen();};
 nitrogen* GetNitrogenUptake() {return nitrogenUptake;};
 nitrogen GetNitrateLeached() {return nitrateLeached;};
 nitrogen GetAmmoniumLeached() {return ammoniumLeached;};
 nitrogen GetAmmonium();
 nitrogen GetAmmonium(double startDep, double thick)
							 {return Profile->GetAmmonium(startDep,thick);};
 double GetAmmoniumN(double startDep, double thick)
 							 {return Profile->GetAmmoniumN(startDep,thick);};
 double GetAmmoniumN15(double startDep, double thick)
 							 {return Profile->GetAmmoniumN15(startDep,thick);};
 nitrogen GetNitrate();
 nitrogen GetNitrate(double startDep, double thick) {return Profile->GetNitrate(startDep,thick);};
 double GetNitrateN(double startDep, double thick) {return Profile->GetNitrateN(startDep,thick);};
 double GetNitrateN15(double startDep, double thick) {return Profile->GetNitrateN15(startDep,thick);};
 nitrogen GetMinNitrogen();
 nitrogen GetMinNitrogen(double startDep, double thick);
 double GetMinNitrogenN(double startDep, double thick);
 double GetMinNitrogenN15(double startDep, double thick);
 nitrogen GetDenitrification() {return denitrification;};
 nitrogen GetN2OFromNitrification() {return Profile->GetN2OFromNitrification();};
 nitrogen GetN2OFromDenitrification() {return Profile->GetN2OFromDenitrification();};
 nitrogen GetTotalNitrogen();
 double GetTotalNitrogenN();
 nitrogen GetTotalNitrogen(double startDep, double thick);
 double GetTotalNitrogenN(double startDep, double thick);
 double GetTotalNitrogenN15(double startDep, double thick);
 nitrogen GetTotalNitrogen(double depth);
 double GetTotalNitrogenN(double depth);
 double GetTotalNitrogenN15(double depth);
 double GetPoolNitrogenN(int Name, double startDep, double thick)
							 {return Profile->GetPoolNitrogenN(Name,startDep,thick);};
 double GetPoolNitrogenN15(int Name, double startDep, double thick)
 							 {return Profile->GetPoolNitrogenN15(Name,startDep,thick);};
 nitrogen GetNitrogenNetMineralisation();
 double GettotalTranspiration(){return  totalTranspiration;}
 // Carbon functions
 double GetTotalCarbon() {return Surface->GetCarbon()+
											Profile->GetCarbon();};
 double GetTotalCarbon(double startDep, double thick)
 								{return Profile->GetOrganicCarbon(startDep,thick);};


 double GetPoolCarbon(int Name, double startDep, double thick)
							 {return Profile->GetPoolCarbon(Name,startDep,thick);};

 double GetCO2Evolution() {return CO2Evolution;};

 // Diverse get functions
 double GetSoilMass(double startDep, double thick) // unit: g dry soil per m2
 								{return Profile->GetSoilMass(startDep,thick);};
 soilSurface* GetSoilSurface(){return  Surface;};
 soilProfile* GetSoilProfile() {return Profile;};

 base* GetMasterSoil() {return MasterSoil;};
 double GetMaximumDepth() {return Profile->GetMaximumDepth();};
 double GetPotassium() {return Potassium;};
 double GetPhosporus() {return Phosphorus;};
 double GetMaxRootDepth() {return Profile->GetMaxRootDepth();};

 // N functions
 nitrogen GetAmmoniumLeaching(double depth) {return Profile->GetAmmoniumLeaching(depth);};
 nitrogen GetNitrateLeaching(double depth) {return Profile->GetNitrateLeaching(depth);};
 nitrogen GetRunOff_N() {return Surface->GetRunOff_N();};

 double GetRunOff() {return Surface->GetRunOff();};
 double GetDiscreteDepth(double depth) {return Profile->GetDiscreteDepth(depth);};
 double GetWaterInfiltration() {return Profile->Getinfiltration();};
 //!incorporate compounds that are on the soil surface into the soil
 /*!
 \param depth maximum depth to which the compounds should be incorporated
 \param  frac  fraction of the compounds on the soil surface that should be incorporated
 */
 void   Incorporate(double MaxDepth, double frac, bool tillage, const char * tillagetype, double MinDepth=0, string plantName="all");
 //!adds an organic product to the soil surface or to the soil profile
 void   AddOrganicProduct(const organicProduct* product,
								  double * rootLengthList = nullptr);

  void TillageEffect(double depth, double effect);
 void WriteOutput();
 void ReadObservations(string FileName);
 void UseObservations();
 void TillageEffect(int Type,double depth, double effect);

   void WriteCarbon(int Name,bool usedBefores2)
							 {Profile->WriteCarbon(Name,usedBefores2);};
  void WriteN(int Name,bool usedBefores2)
							 {Profile->WriteN(Name,usedBefores2);};
    void WriteCN(int Name,bool usedBefores2)
							 {Profile->WriteCN(Name,usedBefores2);};
	    void WriteDecompositionRate(int Name,bool usedBefores2)
							 {Profile->WriteDecompositionRate(Name,usedBefores2);};

 void KillSMB(double Fraction,double Depth);
 void ModifyAOM(double Fraction,double TurnoverFactor, double depth);

 // Budgets
 void StartBudget();
 bool EndBudget(double *N, double *w);
budget* GetWaterBudget() {return &WaterBudget;};
 budget* GetNbudget() {return &Nbudget;};
  void  WriteC()
	 {return Profile->WriteC();};
   void Before();
 void print();
 void BeforeNitrate();
 void PrintNitrate();
 void BeforeAmmonium();
 void PrintAmmonium();

};

#endif

