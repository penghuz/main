/****************************************************************************\
 $URL$
 $LastChangedDate$
 $LastChangedRevision$
 $LastChangedBy$
\****************************************************************************/
#include "../base/common.h"
#include "../base/bstime.h"
#include "../data/outputData.h"
#include "../data/systemData.h"
#include "ecosystem.h"
#include "../base/message.h"
#include "crops/cropWwheat.h"
#include "crops/cropSpringWheat.h"
#include "crops/cropBare.h"
#include "crops/cropWrape.h"
#include "crops/cropSrape.h"
#include "crops/cropSbarley.h"
#include "crops/cropWbarley.h"
#include "crops/cropRyegrass.h"
#include "crops/cropPea.h"
#include "crops/cropBeet.h"
#include "crops/cropMaize.h"
#include "crops/cropClover.h"
#include "crops/cropSeedgrass.h"
#include "crops/cropRyegrass.h"
#include "crops/cropRye.h"
#include "crops/cropOat.h"
#include "crops/cropOilRadish.h"
#include "crops/cropPotato.h"
#include "crops/cropItalRyegrass.h"
#include "crops/cropSoybean.h"
#include "crops/cropSunflower.h"
#include "crops/cropLucerne.h"
#include "crops/cropChickory.h"
#include "crops/cropDyerswoad.h"
#include "crops/cropHairyvetch.h"
#include "crops/cropDyerswoad.h"
#include "crops/cropTriticale.h"
#include "crops/cropFabaBean.h"
#include "crops/cropCrimsonClover.h"

#include "../products/organicProduct.h"
#include "../products/products.h"
#include "../base/climate.h"
#include "fieldOperation/animalFertFields.h"
#include "fieldOperation/harvestFields.h"
#include "fieldOperation/irrigateFields.h"
#include "fieldOperation/mineralFertFields.h"
#include "fieldOperation/manualLabourFields.h"
#include "fieldOperation/sowFields.h"
#include "fieldOperation/soilTreatFields.h"
#include "fieldOperation/plantCareFields.h"
#include "fieldOperation/makeGrazable.h"
#include "../base/timeUse.h"
#include "../base/controlParameters.h"

#include "../products/nitrogen.h"
#include "../technics/technics.h"
#include "../products/feedResource.h"


/****************************************************************************\
Constructor with arguments
\****************************************************************************/
ecosystem::ecosystem(const char * aName, const int aIndex, const base * aOwner)
   :base(aName,aIndex,aOwner)
{
	int run=theSystemData->getCurrentRun();
	if(scenarielogNumber!=run)
	{
		string currentDir=scenarielog.getCurrentPath();

		scenarielog.changeDir(theSystemData->getOutputDirectory());
		waterDetails.changeDir(theSystemData->getOutputDirectory());
		if(scenarielogNumber!=-1)
		{
			scenarielog.closeFile();
			waterDetails.closeFile();
		}
		scenarielog.openFileWrite("scenarie.txt");
		waterDetails.openFileWrite("waterDetails.txt");
		scenarielog.changeDir(currentDir);
		scenarielogNumber=run;
	}
	aSoil                   = new soil("Soil",aIndex,this);
	theCrops                = new cloneList<crop>;
   cutPlantProducts        = new linkList<plantItem>;
   crop * p                = new cropbare("cropbare",0,this);
   theCrops->InsertLast(p);
   irrigationWater.Setamount(0.0);
	k1                      = 0.60; // Ext.coeff. for leaf area, evaporation fraction.
   area                    = 0.0;
   waterContent            = 0.0;
   snowContent             = 0.0;
   tempSnowFall1           = -2.0;
   tempSnowFall2           = 2.0;
   snowMeltRate1           = 2.0;
   snowMeltRate2           = 0.15;
   balanceDepth            = 0.0;
   leachingDepth           = 1000;
   fingerFlow              = 0;
   autoGraze               = false;
   grazingEffiency         = 0.0;
   nLoad                   = 0.0;
   WaterBudget.SetNames("ecosystem","Water");
   totalgLAI               = 0.0;
   type                    = 0;
	totalGrazed_DM				= 0.0;
   totalGrazed_N				= 0.0;
}

/****************************************************************************\
Copy constructor.
\****************************************************************************/
ecosystem::ecosystem(const ecosystem& p)
{
 	aSoil=new soil(*p.aSoil);
   aSoil->SetOwner(this);
   theCrops = NULL;
   if (p.theCrops)
      theCrops=p.theCrops->clone();
   cutPlantProducts = NULL;
   if (p.cutPlantProducts)
   	for (int i=0;i<p.cutPlantProducts->NumOfNodes();i++)
      	cutPlantProducts->InsertLast(p.cutPlantProducts->ElementAtNumber(i));
   k1                  = p.k1;
   area                = p.area;
   irrigationWater     = p.irrigationWater;
   waterContent        = p.waterContent;
   snowContent         = p.snowContent;
   tempSnowFall1       = p.tempSnowFall1;
   tempSnowFall2       = p.tempSnowFall2;
   snowMeltRate1       = p.snowMeltRate1;
   snowMeltRate2       = p.snowMeltRate2;
   WaterBudget         = budget(p.WaterBudget);
   WaterBudget.Reset();  // History ignored
   balanceDepth        = p.balanceDepth;
   leachingDepth       = p.leachingDepth;
   fingerFlow          = p.fingerFlow;
   totalgLAI			  = p.totalgLAI;   //added by NJH 16.3.05

   if (theCrops) // Set soil pointer in each crop
   	for (int i=0;i<theCrops->NumOfNodes();i++)
	   	theCrops->ElementAtNumber(i)->SetSoilPointer(aSoil);

	totalGrazed_DM				= 0.0;
   totalGrazed_N				= 0.0;
}

/****************************************************************************\
Destructor.
\****************************************************************************/
ecosystem::~ecosystem()
{
scenarielog.closeFile();
waterDetails.closeFile();
	delete aSoil;
   if (theCrops)
      delete theCrops;
   if (cutPlantProducts)
   	delete cutPlantProducts;
}

/****************************************************************************\
Returns a copy of this instance at the same inheritance level.
Used in connection with 'cloneList'
\****************************************************************************/
ecosystem* ecosystem::clone() const
{
	ecosystem* p = new ecosystem(*this);
	return p;
}

/****************************************************************************\
\****************************************************************************/
double ecosystem::fPAR(double k, double gLAI)
{
	return 1-exp(-k*gLAI);
}

/****************************************************************************\
\****************************************************************************/
double ecosystem::FractionEPotToSoil(double gLAI, double yLAI)
{
   double ret=1.0-FractionEPotToGreenLeaves(gLAI)-FractionEPotToYellowLeaves(gLAI,yLAI);
   if (ret<0.0)
      ret=0.0;
   return ret;
}

/****************************************************************************\
\****************************************************************************/
double ecosystem::FractionEPotToGreenLeaves(double gLAI)
{
	return 1.0-exp(-k1*gLAI);
}

/****************************************************************************\
\****************************************************************************/
double ecosystem::FractionEPotToYellowLeaves(double gLAI, double yLAI)
{
	return max(0.0,1.0-exp(-k1*(gLAI+yLAI))-FractionEPotToGreenLeaves(gLAI));
}

/****************************************************************************\
Perform addition of water and evaporation from interception layer
surplus           - Water to be added to interception layer and resulting
                    drainage from the layer [mm]
temperature       - Mean air temperature [�C]
globalRadiation   - Global solar radiation [MJ/m�]
ePotGreenLeaves   - Potential evapotranspiration from green leaves [mm]
ePotDeadLeaves    - Potential evapotranspiration from dead leaves [mm]
ePotSoil          - Potential evapotranspiration from soil [mm]
interCeptCapacity - Interception capacity [mm]
\****************************************************************************/
void ecosystem::UpdateInterception(double * surplus,
              						 double   temperature,
			 	            		 double   globalRadiation,
			 			             double * ePotGreenLeaves,
			 			             double   ePotDeadLeaves,
			 			             double * ePotSoil,
			 			             double   interceptCapacity,
                               double * greenEvap)
{
   // Calculate snow accumulation and melting
   if (*surplus>0)
   {
      double incSnow = 0.0;
      if (temperature<tempSnowFall1)
         incSnow = *surplus;
      else
         if (temperature<tempSnowFall2)
  	         incSnow = *surplus*(tempSnowFall2-temperature)/(tempSnowFall2-tempSnowFall1);
      snowContent += incSnow;
      *surplus -= incSnow;
   }
   if (snowContent>0)
   {
      double t = max(temperature,0.0);
      double snowMelt = min(snowContent,max(snowMeltRate1*t+snowMeltRate2*globalRadiation,0.0));
      snowContent -= snowMelt;
      *surplus += snowMelt;
   }
   waterContent += (*surplus);
   // Calculate evaporation

   double ePot = *ePotGreenLeaves+ePotDeadLeaves+(*ePotSoil);
   double snowEvap = 0.0;
   double evap = 0.0;

   if (ePot>0)
   {
      snowEvap = min(snowContent,ePot);
      snowContent -= snowEvap;
      WaterBudget.AddOutput(snowEvap);
      theOutput->AddIndicator(environmentalIndicator,"70.12 Evapotranspiration","cubic metre",snowEvap*area*10);

      evap = min(waterContent,(*ePotGreenLeaves+ePotDeadLeaves)*(1-(snowEvap/ePot)));
      *ePotSoil = *ePotSoil*(1.0-snowEvap/ePot);
      *ePotGreenLeaves = *ePotGreenLeaves*(1.0-snowEvap/ePot);

      if (evap>0)
      {
	      *greenEvap = evap*(*ePotGreenLeaves)/(*ePotGreenLeaves+ePotDeadLeaves);
         waterContent -= evap;
         *ePotGreenLeaves-=*greenEvap;
         WaterBudget.AddOutput(evap);
         theOutput->AddIndicator(environmentalIndicator,"70.12 Evapotranspiration","cubic metre",evap*area*10);
      }
   }
   if (theOutputData->DetailsData.getWaterDetails()==true)
   {
	   waterDetails.WriteWithTab(snowEvap+evap);

   }

   *surplus=0.0;
   if (waterContent>interceptCapacity)
   {
      *surplus = waterContent-interceptCapacity;
      waterContent = interceptCapacity;
   }
}



/****************************************************************************\
\****************************************************************************/
void ecosystem::TransferRootMatter(decomposable * DeadRoot,double * RootLengthList)
{

   DeadRoot->Setamount(0.01*DeadRoot->GetAmount()); 									// Conversion from g/m2 to t/ha
   theOutput->AddIndicator(environmentalIndicator,"32.20 N from roots incorporated into soil","kg N",(DeadRoot->GetorgN_content()+DeadRoot->GetNO3_content()+DeadRoot->GetNH4_content())*DeadRoot->GetAmount()*area*1000.0);
	theOutput->AddIndicator(environmentalIndicator,"40.80 C from roots incorporated into soil","kg C",DeadRoot->GetAmount()*DeadRoot->GetC_content()*area*1000.0);
   aSoil->AddDecomposable(DeadRoot,RootLengthList);
}

/****************************************************************************\
Error check for N from soil
\****************************************************************************/
void ecosystem::CheckNfromSoil(nitrogen &NFromSoil,double NitrogenDemand)
{
   if (NFromSoil.n>(NitrogenDemand+1e-6))
   {
      theMessage->WarningWithDisplay("ecosystem::CheckNfromSoil - nitrogen from soil should not exceed request");
      NFromSoil.n=NitrogenDemand;
   }
   if (NFromSoil.n<0)
   {
    	if (NFromSoil.n<-0.000000001)
      	theMessage->WarningWithDisplay("ecosystem::CheckNfromSoil - nitrogen from soil can not be less than zero");
	   NFromSoil.n=0.0;
   }
}

/****************************************************************************\
Error check for transpirationratio
\****************************************************************************/
void ecosystem::CheckTranspirationRatio(double &TranspirationRatio)
{
   if (TranspirationRatio<0)
   {
      if (TranspirationRatio<-1E-10)
         theMessage->WarningWithDisplay("ecosystem::CheckTranspirationRatio - transpirationratio can not be negative");
      TranspirationRatio=0.0;
   }
   if (TranspirationRatio>1.0)
   {
      if (TranspirationRatio>(1+1E-10))
         theMessage->WarningWithDisplay("ecosystem::CheckTranspirationRatio - transpirationratio can not be greater than one");
      TranspirationRatio=1;
   }
}

/****************************************************************************\
Calculates relative potential evaporation from green leaves
Returns 1 at ppmCO2=377
\****************************************************************************/
double ecosystem::CO2ConcentrationFactor(double CO2conc)
{
   return (1.0+0.00029*377)-0.00029*CO2conc;
}

/****************************************************************************\
Handles information exchange between soil and crop objects
Parameters:
   year, day, month
   temperature        mean air temperature, Celcius
   precip             precipitation, mm
   radiation          global radiation, MJ/m2
   Epot               potential evaporation, mm
\****************************************************************************/
void ecosystem::CropSoilExchange()
{
   double precip = theClimate->precip;
   double EPot = theClimate->epot;
   double temp = theClimate->tmean;
   double radiation = theClimate->rad;

   linkList<rootStructure>* roots = new linkList<rootStructure>;
   cloneList<crop>::PS aCropElement;
   rootStructure* cropRoot;
   crop* aCrop;
   nFixThisDay=0.0;

   // 1) Add irrigation nitrogen and atmospheric nitrogen deposition to soil
   double NitrogenInRain = theClimate->GetNitrogenInRain();
   double irrigation = irrigationWater.GetAmount()/10;        // converts m3 to mm
   double irrigationN = irrigationWater.GetN_content();
   nitrogen NinWater,N1;
   NinWater.SetBoth(precip*NitrogenInRain+irrigation*irrigationN,0);
   aSoil->AddNutrient(N1,N1,N1,NinWater);
   WaterBudget.AddInput(precip+irrigation);
   theOutput->AddIndicator(environmentalIndicator,"70.01 Rainwater","cubic metre",precip*area*10.0);
   theOutput->AddIndicator(environmentalIndicator,"31.02 N from deposition","kg N",NitrogenInRain*precip*area*10.0);
   theOutput->AddIndicator(environmentalIndicator,"31.05 N from irrigation water","kg N",irrigation*irrigationN*area*10.0);

   if (theOutputData->DetailsData.getWaterDetails()==true)
   {

	   waterDetails.WriteWithTabS(theTime.GetString2());
	   waterDetails.WriteWithTab(precip+irrigation);
	   waterDetails.WriteWithTab(EPot);

   }



   // 2) Sum leaf areas and interception capacity, calculate "effective" extinction coeff.
   totalgLAI=0.0;
   double totalyLAI=0.0;
   double InterceptionCapacity=0.0;
   double k=0.0;
   theCrops->PeekHead(aCropElement);
   while (aCropElement!=NULL)
   {
   	totalgLAI            += aCropElement->element->GiveLeafAreaIndex();
   	totalyLAI            += aCropElement->element->GiveYellowLeafAreaIndex();
   	InterceptionCapacity += aCropElement->element->InterceptionCapacity();
      k                    += aCropElement->element->GiveExtCoeff()*aCropElement->element->GiveLeafAreaIndex();
      theCrops->OneStep(aCropElement);
   }
   if (totalgLAI>0.0)
      k=k/totalgLAI; // k calculated according to Ross et al. 1972. J. Appl. Ecol.: 535-556
   else
      k=0.44;

   // Convey information about total LAI to all crops
   theCrops->PeekHead(aCropElement);
   while (aCropElement!=NULL)
   {
      aCropElement->element->SetTotalLAIStand(totalgLAI+totalyLAI);
      theCrops->OneStep(aCropElement);
   }
/*#ifndef __ECOSYSTEM__
   if (theTime.GetMonth()==6 && theTime.GetDay()==15 && (totalgLAI+totalyLAI)<1E-10 && !CropName=="NoCrop")
   {
      cout << endl << " Object path : " << GetLongName();
      theMessage->FatalError("ecosystem::CropSoilExchange - no leaf area present 15th June");
   }
#endif*/

   // 3) Compute corrected Epot for canopy
   double EpotLeaves = EPot*(1.0-FractionEPotToSoil(totalgLAI,totalyLAI));
   double correctedEPotLeaves=0.0;
   double weightedMeanEvapFactor=0.0;
   theCrops->PeekHead(aCropElement);
   while (aCropElement!=NULL)
   {
      aCrop = aCropElement->element;
      if ((totalgLAI+totalyLAI)>0)
         correctedEPotLeaves += EpotLeaves*aCrop->GiveEvapFactor()*(aCrop->GiveLeafAreaIndex()+aCrop->GiveYellowLeafAreaIndex())/(totalgLAI+totalyLAI);
      weightedMeanEvapFactor += aCrop->GiveEvapFactor()*aCrop->GiveLeafAreaIndex();
      theCrops->OneStep(aCropElement);
   }
   if (totalgLAI>0)
      weightedMeanEvapFactor=weightedMeanEvapFactor/totalgLAI;

   // 4) Update canopy water interception and eventual snow layer
   double surplus = precip + irrigation;
   double greenLeafEpot = correctedEPotLeaves;
   double CO2conc = theClimate->GetCO2Concentration();
   if (FractionEPotToSoil(totalgLAI,totalyLAI)<1.0)
      greenLeafEpot = CO2ConcentrationFactor(CO2conc)*correctedEPotLeaves*FractionEPotToGreenLeaves(totalgLAI)/(1.0-FractionEPotToSoil(totalgLAI,totalyLAI));
   double soilEpot = EPot*FractionEPotToSoil(totalgLAI,totalyLAI);
   double greenInterceptEvap=0.0;
   UpdateInterception(&surplus,
                		 temp,
			 	          radiation,
                      &greenLeafEpot,
			 			    correctedEPotLeaves*FractionEPotToYellowLeaves(totalgLAI,totalyLAI),
			 			    &soilEpot,
                      InterceptionCapacity,
                      &greenInterceptEvap);

   // 5) Collect root and water demand parameters
   theCrops->PeekHead(aCropElement);
   int q=0;
   while (aCropElement!=NULL)
   {
      aCrop = aCropElement->element;
      cropRoot = new rootStructure;
    	aCrop->GetStatus (&cropRoot->rootRadius,
								&cropRoot->rootpF,
								&cropRoot->NitrogenDemand,
								&cropRoot->NitrateUptakeRate,
								&cropRoot->AmmoniumUptakeRate,
	                     &cropRoot->MinimumSoilNitrate,
	                     &cropRoot->MinimumSoilAmmonium);
      cropRoot->transpirationDemand = 0.0;
      if (totalgLAI>0)
         cropRoot->transpirationDemand = greenLeafEpot*(aCrop->GiveEvapFactor()/weightedMeanEvapFactor)*(aCrop->GiveLeafAreaIndex()/totalgLAI);
      cropRoot->rootLengthList = aCrop->GiveRootLengthList();
      roots->InsertLast(cropRoot);
      theCrops->OneStep(aCropElement);

      q++;
   }

   // 6) Update soil
   	if(theTime.GetDay()==11&&theTime.GetMonth()==8&&theTime.GetYear()==2003)
		cout<<endl;
	aSoil->Update(surplus,temp,soilEpot,snowContent,roots);
  	aSoil->Incorporate(10,0.02,false,"none");                               // Daily transfer of org. matter to top soil layer (10 mm)

   // 7) Update crop

   theCrops->PeekHead(aCropElement);
   const int CanopyIntervals = 25;         // canopy divided into 25 height intervals
   double CropRadiation[MaxPlants];
//   double CropExtinction[MaxPlants];
   double MaxCanopyHeight = 0;

   for (int i=0;i<theCrops->NumOfNodes();i++)
   {
    	CropRadiation[i] = 0.0;
   	MaxCanopyHeight = max(MaxCanopyHeight,theCrops->ElementAtNumber(i)->GiveCropHeight());
   }

   double CanopyPar[CanopyIntervals+1];
   double CanopyIntervalHeight =  MaxCanopyHeight/CanopyIntervals;
   CanopyPar[CanopyIntervals] = radiation;

   for (int i=CanopyIntervals-1;i>=0; i--)
   {
      double a = 0.0;
      double IntervalStart = i*CanopyIntervalHeight;
      double IntervalEnd = MaxCanopyHeight-i*CanopyIntervalHeight;
      for (int j=0;j<theCrops->NumOfNodes();j++)
      {
//          const crop* aCrop = theCrops->ElementAtNumber(j);  //Eclipse complains about the 'const' NJH, feb 2009
        crop* aCrop = theCrops->ElementAtNumber(j);
      	a += aCrop->TotalLeafAreaIndex(IntervalStart,IntervalEnd)* (aCrop->GiveExtCoeff());
         /* Code from BMP+NJH version @@@
      	double totalLAI= aCrop->TotalLeafAreaIndex(IntervalStart,IntervalEnd);
      	double extinctionCoeff = aCrop->GiveExtCoeff();
      	a += totalLAI*extinctionCoeff;
         */
      }
      CanopyPar[i] = radiation*exp(-a);
   }
   for (int i=CanopyIntervals-1;i>=0; i--)
   {
      for (int j=0;j<theCrops->NumOfNodes();j++)
      {
         double k = theCrops->ElementAtNumber(j)->GiveExtCoeff();
         double LAIinInterval = theCrops->ElementAtNumber(j)->GreenLeafAreaIndex(i*CanopyIntervalHeight,CanopyIntervalHeight);
         CropRadiation[j] += CanopyPar[i]*k*LAIinInterval;
   	}
   }

   // 8) Transfer nitrogen and water information to crop
   nitrogen* NitrogenUptake  = aSoil->GetNitrogenUptake();
   double TotalDryMatterStorage = 0.0;
   double TotalDryMatterVegTop = 0.0;
   for (int i=0;i<roots->NumOfNodes();i++)
   {
      rootStructure* aRoot = roots->ElementAtNumber(i);
   	aCrop = theCrops->ElementAtNumber(i);
   	double TranspirationRatio = 1.0;
      if (aRoot->transpirationDemand>0.0)
      	TranspirationRatio = (greenInterceptEvap+aRoot->actualTranspiration)/(greenInterceptEvap+aRoot->transpirationDemand);
      CheckTranspirationRatio(TranspirationRatio);
    	nitrogen NFromSoil;
      NFromSoil.n=0.0;
      NFromSoil.n15=0.0;
   	if (NitrogenUptake)
	   {
      	NFromSoil = NitrogenUptake[i];
      	CheckNfromSoil(NFromSoil,aRoot->NitrogenDemand);
      	theOutput->AddIndicator(environmentalIndicator,"32.10 N uptake from soil","kg N",NFromSoil*area*10);
      }
      aCrop->NitrogenAndTranspiration(NFromSoil,TranspirationRatio);
      TotalDryMatterStorage += aCrop->GiveDryMatterStorage();
      TotalDryMatterVegTop += aCrop->GiveDryMatterVegTop();
	}

   // 9) Upate crop growth, calculate C and N from litterfall, N from fixation plus approximate Carbon content in plant
   double TotalCFixation = 0.0;
   for (int j=0;j<theCrops->NumOfNodes();j++)
   {
      aCrop = theCrops->ElementAtNumber(j);
 
      aCrop->SetDryMatterVegTopTotalStand(TotalDryMatterVegTop);
      TotalCFixation += 0.42*aCrop->Update(CropRadiation[j]);

      decomposable * rootMatter = aCrop->GetRootMatter();
      if (rootMatter)
      {
      	TransferRootMatter(rootMatter,aCrop->GiveRootLengthList());
         aCrop->GetRootMatter()->Setamount(0.0);
         // aCrop->DeleteRootMatter(); !!!???@@@
      }

      decomposable * topMatter = aCrop->GetTopMatter();
      if (topMatter)
      {
         aSoil->AddDecomposable(topMatter);
   	   theOutput->AddIndicator(environmentalIndicator,"32.22 N from leaf litterfall incorporated into soil","kg N",topMatter->GetAllN()*topMatter->GetAmount()*area*1000.0);
		   theOutput->AddIndicator(environmentalIndicator,"40.82 C from leaf litterfall incorporated into soil","kg C",topMatter->GetAmount()*topMatter->GetC_content()*area*1000.0);
      }
      theOutput->AddIndicator(environmentalIndicator,"31.04 N from fixation","kg N",aCrop->GetNFixationThisDay()*area*10.0);
      nFixThisDay+=aCrop->GetNFixationThisDay();
   }
   theOutput->AddIndicator(environmentalIndicator,"40.01 Net crop carbon fixation","kg C",TotalCFixation*area*10);

   // 10. Remove dead crops from list
   theCrops->PeekHead(aCropElement);
   while (aCropElement!=NULL)
   {
      aCrop = aCropElement->element;
      if (aCrop->ReadyForRemoval() && theCrops->NumOfNodes()>1)
      {
          cloneList<crop>::PS P=aCropElement;
          theCrops->OneStep(aCropElement);
          theCrops->Remove(P);
      }
      else
         theCrops->OneStep(aCropElement);
   }

	if (autoGraze) // Simple grazing developed by JBE+BMP
      SimpleGraze();
   /* @@@ ???!!!
	if (FOP->GetOperationId()==MakeGrazable)
   {
   	makeGrazable * graz = (makeGrazable *) FOP;
   	if (graz->GetmakeItPossible())
      	Setgrazable(true);
      else
      	Setgrazable(false);
   }
   */

   // N balance calculation
	nitrogen N_leached = aSoil->GetNitrateLeaching(aSoil->GetMaximumDepth())+aSoil->GetAmmoniumLeaching(aSoil->GetMaximumDepth());
   nitrogen N_den     = aSoil->GetDenitrification();
   nitrogen N_runoff  = aSoil->GetRunOff_N();
   WaterBudget.AddOutput(aSoil->GetDrainage(aSoil->GetMaximumDepth())+aSoil->GetEvapotranspiration()+aSoil->GetRunOff());

   delete roots;
   irrigationWater.Setamount(0.0);

   // Water calculation
   double soilEvaporation = aSoil->GetSoilSurface()->GetEvaporation();
   double totalEvaporation = aSoil->GetEvapotranspiration();

   if (theOutputData->DetailsData.getWaterDetails()==true)
   {
	   waterDetails.WriteWithTab(soilEvaporation);
	   waterDetails.WriteWithTab(totalEvaporation-soilEvaporation);
	   waterDetails.WriteWithTab(aSoil->GetDrainage(aSoil->GetMaximumDepth()));
	   waterDetails.WriteNewLine();
   }
   EndBudget();

}

/****************************************************************************\
\****************************************************************************/
void ecosystem::AddHarvestedAreaToIndicators(const char * cid, double a)
{
   if (strcmp(cid,"W1")==0 || strcmp(cid,"W2")==0)
      theOutput->AddIndicator(economicIndicator,"18.10 Exp. W.wheat yield (harv. bef. 1/8)","ha",a);
	if (strcmp(cid,"W5")==0 || strcmp(cid,"W6")==0)
      theOutput->AddIndicator(economicIndicator,"18.11 Exp. S.wheat yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"RW")==0)
      theOutput->AddIndicator(economicIndicator,"18.12 Exp. W.rape yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"RS")==0)
      theOutput->AddIndicator(economicIndicator,"18.13 Exp. S.rape yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"B5")==0 || strcmp(cid,"B6")==0)
      theOutput->AddIndicator(economicIndicator,"18.14 Exp. W.barley yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"B1")==0 || strcmp(cid,"B2")==0)
      theOutput->AddIndicator(economicIndicator,"18.15 Exp. S.barley yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"RY")==0)
      theOutput->AddIndicator(economicIndicator,"18.16 Exp. Rye yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"OA")==0)
      theOutput->AddIndicator(economicIndicator,"18.17 Exp. Oat yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"PE")==0)
      theOutput->AddIndicator(economicIndicator,"18.18 Exp. Pea yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"BP")==0)
		theOutput->AddIndicator(economicIndicator,"18.26 Barley/Pea area (harv. bef. 1/9)","ha",a);
   if (strcmp(cid,"PO")==0)
      theOutput->AddIndicator(economicIndicator,"18.19 Exp. Potato yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"SG")==0)
      theOutput->AddIndicator(economicIndicator,"18.20 Exp. Grass for seed yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"MA")==0)
      theOutput->AddIndicator(economicIndicator,"18.21 Exp. Maize yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"BP")==0)
      theOutput->AddIndicator(economicIndicator,"18.23 Exp. Barley/Pea yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"C1")==0 || strcmp(cid,"C2")==0)
      theOutput->AddIndicator(economicIndicator,"18.24 Exp. Grass/clover yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"BE")==0)
      theOutput->AddIndicator(economicIndicator,"18.22 Exp. Beet yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"SB")==0)
      theOutput->AddIndicator(economicIndicator,"18.21 Exp. Soy bean yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"SF")==0)
      theOutput->AddIndicator(economicIndicator,"18.23 Exp. Sunflower yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"HV")==0)
      theOutput->AddIndicator(economicIndicator,"18.26 Exp. Dyerswoad yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"TR")==0)
      theOutput->AddIndicator(economicIndicator,"18.27 Exp. Triticale yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"FB")==0)
      theOutput->AddIndicator(economicIndicator,"18.28 Exp. FabaBean yield (harv. bef. 1/8)","ha",a);
   if (strcmp(cid,"CR")==0)
      theOutput->AddIndicator(economicIndicator,"18.29 Exp. CrimsonClover yield (harv. bef. 1/8)","ha",a);
}

/****************************************************************************\
\****************************************************************************/
void ecosystem::GiveIndicators()
{
   if (balanceDepth<1E-10)
      theMessage->FatalError("ecosystem::GiveIndicators - soil depth not initialised");

   theOutput->AddStateIndicator(environmentalIndicator,"40.60 Total C in soil","kg C",aSoil->GetTotalCarbon()*area*10);  //GetPoolCarbon(const char * Name, double startDep, double thick)
#ifdef COOLCROP
	theOutput->AddStateIndicator(environmentalIndicator,"40.62 Total C in soil 0-25 cm","kg C",aSoil->GetTotalCarbon(0,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.63 Total C in soil 25-50 cm","kg C",aSoil->GetTotalCarbon(250,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.64 Total C in soil 50-75 cm","kg C",aSoil->GetTotalCarbon(500,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.65 Total C in soil 75-250 cm","kg C",aSoil->GetTotalCarbon(750,1750)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.66 C in AOM+SMB 0-25 cm","kg C",
   	(aSoil->GetPoolCarbon("AOM1",0,250)+aSoil->GetPoolCarbon("AOM2",0,250)+aSoil->GetPoolCarbon("SMB1",0,250)+
      aSoil->GetPoolCarbon("SMB2",0,250)+aSoil->GetPoolCarbon("SMR",0,250))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.67 C in AOM+SMB 25-50 cm","kg C",
   	(aSoil->GetPoolCarbon("AOM1",250,250)+aSoil->GetPoolCarbon("AOM2",250,250)+aSoil->GetPoolCarbon("SMB1",250,250)+
      aSoil->GetPoolCarbon("SMB2",250,250)+aSoil->GetPoolCarbon("SMR",250,250))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.68 C in AOM+SMB 50-75 cm","kg C",
   	(aSoil->GetPoolCarbon("AOM1",500,250)+aSoil->GetPoolCarbon("AOM2",500,250)+aSoil->GetPoolCarbon("SMB1",500,250)+
      aSoil->GetPoolCarbon("SMB2",500,250)+aSoil->GetPoolCarbon("SMR",500,250))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.69 C in AOM+SMB 75-250 cm","kg C",
   	(aSoil->GetPoolCarbon("AOM1",750,1750)+aSoil->GetPoolCarbon("AOM2",750,1750)+aSoil->GetPoolCarbon("SMB1",750,1750)+
      aSoil->GetPoolCarbon("SMB2",750,1750)+aSoil->GetPoolCarbon("SMR",750,1750))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.70 C in NOM 0-25 cm","kg C",aSoil->GetPoolCarbon("NOM",0,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.71 C in NOM 25-50 cm","kg C",aSoil->GetPoolCarbon("NOM",250,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.72 C in NOM 50-75 cm","kg C",aSoil->GetPoolCarbon("NOM",500,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.73 C in NOM 75-250 cm","kg C",aSoil->GetPoolCarbon("NOM",750,1750)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.74 C in IOM 0-25 cm","kg C",aSoil->GetPoolCarbon("IOM",0,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.75 C in IOM 25-50 cm","kg C",aSoil->GetPoolCarbon("IOM",250,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.76 C in IOM 50-75 cm","kg C",aSoil->GetPoolCarbon("IOM",500,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"40.77 C in IOM 75-250 cm","kg C",aSoil->GetPoolCarbon("IOM",750,1750)*area*10);
#endif
   theOutput->AddStateIndicator(environmentalIndicator,"40.90 AOM1 C in soil","kg C",aSoil->GetPoolCarbon("AOM1",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"40.91 AOM2 C in soil","kg C",aSoil->GetPoolCarbon("AOM2",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"40.92 SMB1 C in soil","kg C",aSoil->GetPoolCarbon("SMB1",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"40.93 SMB2 C in soil","kg C",aSoil->GetPoolCarbon("SMB2",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"40.94 SMR C in soil","kg C",aSoil->GetPoolCarbon("SMR",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"40.95 NOM C in soil","kg C",aSoil->GetPoolCarbon("NOM",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"40.96 IOM C in soil","kg C",aSoil->GetPoolCarbon("IOM",0,balanceDepth)*area*10);

   theOutput->AddIndicator(environmentalIndicator,"40.40 CO2 soil respiration","kg C",aSoil->GetCO2Evolution()*area*10);

   theOutput->AddStateIndicator(environmentalIndicator,"32.62 AOM1 N in soil","kg N",aSoil->GetPoolNitrogen("AOM1",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"32.63 AOM2 N in soil","kg N",aSoil->GetPoolNitrogen("AOM2",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"32.64 SMB1 N in soil","kg N",aSoil->GetPoolNitrogen("SMB1",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"32.65 SMB2 N in soil","kg N",aSoil->GetPoolNitrogen("SMB2",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"32.66 SMR N in soil","kg N",aSoil->GetPoolNitrogen("SMR",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"32.67 NOM N in soil","kg N",aSoil->GetPoolNitrogen("NOM",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"32.68 IOM N in soil","kg N",aSoil->GetPoolNitrogen("IOM",0,balanceDepth)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"32.60 Organic N in soil","kg N",(aSoil->GetTotalNitrogen(0,balanceDepth)-aSoil->GetMinNitrogen(0,balanceDepth))*area*10);
#ifdef COOLCROP
	theOutput->AddStateIndicator(environmentalIndicator,"32.69 Organic N in soil 0-25 cm","kg N",(aSoil->GetTotalNitrogen(0,250)-aSoil->GetMinNitrogen(0,250))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.70 Mineral N in soil 0-25 cm","kg N",aSoil->GetMinNitrogen(0,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.71 Organic N in soil 25-50 cm","kg N",(aSoil->GetTotalNitrogen(250,250)-aSoil->GetMinNitrogen(250,250))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.72 Mineral N in soil 25-50 cm","kg N",aSoil->GetMinNitrogen(250,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.73 Organic N in soil 50-75 cm","kg N",(aSoil->GetTotalNitrogen(500,250)-aSoil->GetMinNitrogen(500,250))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.74 Mineral N in soil 50-75 cm","kg N",aSoil->GetMinNitrogen(500,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.75 Organic N in soil 75-250 cm","kg N",(aSoil->GetTotalNitrogen(750,1750)-aSoil->GetMinNitrogen(750,1750))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.76 Mineral N in soil 75-250 cm","kg N",aSoil->GetMinNitrogen(750,1750)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.77 N in AOM+SMB 0-25 cm","kg N",
   	(aSoil->GetPoolNitrogen("AOM1",0,250)+aSoil->GetPoolNitrogen("AOM2",0,250)+aSoil->GetPoolNitrogen("SMB1",0,250)+
      aSoil->GetPoolNitrogen("SMB2",0,250)+aSoil->GetPoolNitrogen("SMR",0,250))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.78 N in AOM+SMB 25-50 cm","kg N",
   	(aSoil->GetPoolNitrogen("AOM1",250,250)+aSoil->GetPoolNitrogen("AOM2",250,250)+aSoil->GetPoolNitrogen("SMB1",250,250)+
      aSoil->GetPoolNitrogen("SMB2",250,250)+aSoil->GetPoolNitrogen("SMR",250,250))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.79 N in AOM+SMB 50-75 cm","kg N",
   	(aSoil->GetPoolNitrogen("AOM1",500,250)+aSoil->GetPoolNitrogen("AOM2",500,250)+aSoil->GetPoolNitrogen("SMB1",500,250)+
      aSoil->GetPoolNitrogen("SMB2",500,250)+aSoil->GetPoolNitrogen("SMR",500,250))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.80 N in AOM+SMB 75-250 cm","kg N",
   	(aSoil->GetPoolNitrogen("AOM1",750,1750)+aSoil->GetPoolNitrogen("AOM2",750,1750)+aSoil->GetPoolNitrogen("SMB1",750,1750)+
      aSoil->GetPoolNitrogen("SMB2",750,1750)+aSoil->GetPoolNitrogen("SMR",750,1750))*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.81 N in NOM 0-25 cm","kg N",aSoil->GetPoolNitrogen("NOM",0,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.82 N in NOM 25-50 cm","kg N",aSoil->GetPoolNitrogen("NOM",250,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.90 N in NOM 50-75 cm","kg N",aSoil->GetPoolNitrogen("NOM",500,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.91 N in NOM 75-250 cm","kg N",aSoil->GetPoolNitrogen("NOM",750,1750)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.92 N in IOM 0-25 cm","kg N",aSoil->GetPoolNitrogen("IOM",0,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.93 N in IOM 25-50 cm","kg N",aSoil->GetPoolNitrogen("IOM",250,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.94 N in IOM 50-75 cm","kg N",aSoil->GetPoolNitrogen("IOM",500,250)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.95 N in IOM 75-250 cm","kg N",aSoil->GetPoolNitrogen("IOM",750,1750)*area*10);
#endif
   theOutput->AddStateIndicator(environmentalIndicator,"32.83 Nmin 0-500","kg N",aSoil->GetMinNitrogen(0,500)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.84 Nmin 0-750","kg N",aSoil->GetMinNitrogen(0,750)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"32.85 Nmin 0-1000","kg N",aSoil->GetMinNitrogen(0,1000)*area*10);

   theOutput->AddStateIndicator(economicIndicator,"01.01 Total area","ha",area);

   theOutput->AddIndicator(environmentalIndicator,"70.40 Percolation from leaching depth","cubic meter",aSoil->GetDrainage(leachingDepth)*area*10);
   theOutput->AddIndicator(environmentalIndicator,"70.10 Percolation","cubic meter",aSoil->GetDrainage(balanceDepth)*area*10);
   theOutput->AddIndicator(environmentalIndicator,"70.11 Surface run-off","cubic meter",aSoil->GetRunOff()*area*10);
	theOutput->AddIndicator(environmentalIndicator,"70.12 Evapotranspiration","cubic meter",aSoil->GetEvapotranspiration()*area*10);
	theOutput->AddIndicator(environmentalIndicator,"70.42 Bare soil evaporation","cubic meter",aSoil->GetSoilEvaporation()*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"70.15 Water in field to balancedepth","cubic meter",(snowContent+waterContent+aSoil->GetWater(0,balanceDepth)+aSoil->GetSurfaceWater())*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"70.41 Water in field to leaching depth","cubic meter",(snowContent+waterContent+aSoil->GetWater(0,leachingDepth)+aSoil->GetSurfaceWater())*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"70.20 Water in soil","cubic meter",waterContent*area*10);

   theOutput->AddIndicator(environmentalIndicator,"32.40 Nitrate N leached from leaching depth","kg N",aSoil->GetNitrateLeaching(leachingDepth).n*area*10);
   theOutput->AddIndicator(environmentalIndicator,"32.41 Ammonium N leached from leaching depth","kg N",aSoil->GetAmmoniumLeaching(leachingDepth).n*area*10);
   theOutput->AddIndicator(environmentalIndicator,"31.40 Nitrate N leached","kg N",aSoil->GetNitrateLeaching(balanceDepth).n*area*10);
	theOutput->AddIndicator(environmentalIndicator,"31.41 Ammonium N leached","kg N",aSoil->GetAmmoniumLeaching(balanceDepth).n*area*10);
	theOutput->AddIndicator(environmentalIndicator,"31.47 N in surface run-off","kg N",aSoil->GetRunOff_N().n*area*10);
	theOutput->AddIndicator(environmentalIndicator,"31.42 N2 from denitrification","kg N",aSoil->GetDenitrification().n*area*10);
   theOutput->AddIndicator(environmentalIndicator,"31.43 N2O from denitrification","kg N",aSoil->GetN2OFromDenitrification().n*area*10);
   theOutput->AddIndicator(environmentalIndicator,"31.44 N2O from nitrification","kg N",aSoil->GetN2OFromNitrification().n*area*10);
	theOutput->AddIndicator(environmentalIndicator,"32.30 Gross N-mineralisation from soil org. matter","kg",aSoil->GetNitrogenNetMineralisation().n*area*10.0);
   theOutput->AddStateIndicator(environmentalIndicator,"32.61 Mineral N in soil","kg N",aSoil->GetMinNitrogen().n*area*10);

   theOutput->AddIndicator(environmentalIndicator,"38.12 N15 leached from leaching depth","kg N",aSoil->GetNitrateLeaching(leachingDepth).n15*area*10+aSoil->GetAmmoniumLeaching(leachingDepth).n15*area*10);
	theOutput->AddIndicator(environmentalIndicator,"38.21 N15-N2O from denitrification","kg N",aSoil->GetN2OFromDenitrification().n15*area*10);
   theOutput->AddIndicator(environmentalIndicator,"38.22 N15-N2O from nitrification","kg N",aSoil->GetN2OFromNitrification().n15*area*10);
   theOutput->AddIndicator(environmentalIndicator,"38.23 N15-Denitrification","kg N",aSoil->GetDenitrification().n15*area*10);
 	theOutput->AddStateIndicator(environmentalIndicator,"38.24 Total soil/soilsurface 15N","kg N",aSoil->GetTotalNitrogen(1000).n15*area*10);

   nitrogen NCrop;
   double CCrop = 0;
   if (theCrops)
   {
      cloneList<crop>::PS aCropElement;
   	theCrops->PeekHead(aCropElement);
   	while (aCropElement!=NULL)
      {
         NCrop = NCrop + aCropElement->element->GiveTotalNitrogen();
         CCrop += (aCropElement->element->GiveDryMatterStorage()+aCropElement->element->GiveDryMatterVegTop()+aCropElement->element->GiveDryMatterRoot())*0.45;
         theCrops->OneStep(aCropElement);
      }
   }
   theOutput->AddStateIndicator(environmentalIndicator,"31.60 N in standing crop/straw","kg N",NCrop*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"31.61 Total soil/soilsurface N","kg N",aSoil->GetTotalNitrogen(balanceDepth)*area*10);
	theOutput->AddStateIndicator(environmentalIndicator,"31.99 Total soil/soilsurface nitrogen to 1 m depth","kg N",aSoil->GetTotalNitrogen(1000)*area*10);
   theOutput->AddStateIndicator(environmentalIndicator,"40.61 C in standing crop/straw","kg C",CCrop*area*10);

      if (theTime.GetDay()==1 && (theTime.GetMonth()==9 || theTime.GetMonth()==11))
   {
      double CatchDM = 0;
      double CatchN = 0;
      cloneList<crop>::PS aCropElement;
   	theCrops->PeekHead(aCropElement);
   	while (aCropElement!=NULL)
      {
         if (aCropElement->element->GetCropName()=="Ryegrass" || aCropElement->element->GetCropName()=="Clover"
         	|| aCropElement->element->GetCropName()=="DyersWoad" || aCropElement->element->GetCropName()=="OilRadish"
            || aCropElement->element->GetCropName()=="HairyWetch" || aCropElement->element->GetCropName()=="Chickory")
         {
         	if (aCropElement->element->GetCropName()=="OilRadish")
            {
            	CatchDM += aCropElement->element->GiveDryMatterVegTop()*area*10;
            	CatchN += aCropElement->element->NitrogenInVegTop().n*area*10;
            }
            else
            {
            	CatchDM += (aCropElement->element->GiveDryMatterVegTop()+aCropElement->element->GiveDryMatterStorage())*area*10;
            	CatchN += (aCropElement->element->NitrogenInVegTop().n+aCropElement->element->NitrogenInStorage().n)*area*10;
            }
         }
			theCrops->OneStep(aCropElement);
      }
   	if (theTime.GetMonth()==9 && theTime.GetDay()==1)
      {
   		theOutput->AddIndicator(environmentalIndicator,"39.01 Catch crop DM (1/9)","kg DM",CatchDM);
			theOutput->AddIndicator(environmentalIndicator,"39.03 Catch crop N (1/9)","kg N",CatchN);
      }
      if (theTime.GetMonth()==11 && theTime.GetDay()==1)
      {
			theOutput->AddIndicator(environmentalIndicator,"39.02 Catch crop DM (1/11)","kg DM",CatchDM);
			theOutput->AddIndicator(environmentalIndicator,"39.04 Catch crop N (1/11)","kg N",CatchN);
      }
   }

   // cutPlantProducts in tonnes/ha
	if (cutPlantProducts)
   {
      if (cutPlantProducts->NumOfNodes()>0)
      {
         nitrogen NInStraw;
         for (int i=0;i<cutPlantProducts->NumOfNodes();i++)
         {
            plantItem* straw = cutPlantProducts->ElementAtNumber(i);
            NInStraw = NInStraw + straw->GetAllN()*straw->GetAmount()*1000.0; // kg N/ha
         }
         theOutput->AddStateIndicator(environmentalIndicator,"31.60 N in standing crop/straw","kg N",NInStraw*area);
      }
   }
}

void ecosystem::AddYieldToIndicators(const char * cid, double y,double n)
{
   if (strcmp(cid,"WNWHT")==0)
   {
		theOutput->AddIndicator(economicIndicator,"17.10 W.wheat yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.00 W.wheat yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"SPWHT")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.11 S.wheat yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.01 S.wheat yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"WNRPE")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.12 W.rape yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.02 W.rape yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"SPRAPE")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.13 S.rape yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.03 S.rape yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"WNBRL")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.14 W.barley yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.04 W.barley yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"BARLEY")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.15 S.barley yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.05 S.barley yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"RYE")==0)
	{
      theOutput->AddIndicator(economicIndicator,"17.16 Rye yield (harv. bef. 1/8)","t",y);
	   theOutput->AddIndicator(environmentalIndicator,"36.06 Rye yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"OAT")==0)
   {
   	theOutput->AddIndicator(economicIndicator,"17.17 Oat yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.07 Oat yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"PEA")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.18 Pea yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.08 Pea yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"POTATO")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.19 Potato yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.09 Potato yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"SEEDGRASS")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.20 Grass for seed yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.10 Grass for seed yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"MAIZE")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.21 Grain maize yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.11 Grain maize yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"BEET")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.22 Beet yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.12 Beet yield (harv. bef. 1/8)","kg N",n);
   }
   if ((strcmp(cid,"RYEGRASS")==0)||(strcmp(cid,"RYEGRASSWHOLE")==0))
	{
   	theOutput->AddIndicator(economicIndicator,"17.23 Ryegrass yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.13 Ryegrass yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"CLOVER")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.24 Clover yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.14 Clover yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"BP")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.25 Barley/Pea yield (harv. bef. 1/9)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.15 Barley/Pea yield (harv. bef. 1/9)","kg N",n);
   }
   if (strcmp(cid,"MAIZEWHOLE")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.26 Maize wholecrop yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.16 Maize wholecrop yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"ITALRYEGWHOLE")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.27 Italian ryegrass yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.17 Italian ryegrass yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"SOYBEAN")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.28 Soybean yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.18 Soybean yield (harv. bef. 1/8)","kg N",n);
   }
   if (strcmp(cid,"SUNFLOWER")==0)
	{
   	theOutput->AddIndicator(economicIndicator,"17.29 Sunflower yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.19 Sunflower yield (harv. bef. 1/8)","kg N",n);
   }
   if ((strcmp(cid,"WNWHTWHOLE")==0)||(strcmp(cid,"SPWHTWHOLE")==0))
   {
		theOutput->AddIndicator(economicIndicator,"17.30 Wheat silage yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.20 Wheat silage yield (harv. bef. 1/8)","kg N",n);
   }
   if ((strcmp(cid,"LUCERNE")==0)||(strcmp(cid,"LUCERNEWHOLE")==0))
	{
   	theOutput->AddIndicator(economicIndicator,"17.31 Lucerne yield (harv. bef. 1/8)","t",y);
   	theOutput->AddIndicator(environmentalIndicator,"36.21 Lucerne yield (harv. bef. 1/8)","kg N",n);
   }
}

/****************************************************************************\
\****************************************************************************/
void ecosystem::AddExpectedYieldToIndicators(const char * cid, double y)
{
   if (strcmp(cid,"W1")==0 || strcmp(cid,"W2")==0)	theOutput->AddIndicator(economicIndicator,"18.10 Exp. W.wheat yield (harv. bef. 1/8)","t",y);
	if (strcmp(cid,"W5")==0 || strcmp(cid,"W6")==0) theOutput->AddIndicator(economicIndicator,"18.11 Exp. S.wheat yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"RW")==0)                        theOutput->AddIndicator(economicIndicator,"18.12 Exp. W.rape yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"RS")==0)                        theOutput->AddIndicator(economicIndicator,"18.13 Exp. S.rape yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"B5")==0 || strcmp(cid,"B6")==0) theOutput->AddIndicator(economicIndicator,"18.14 Exp. W.barley yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"B1")==0 || strcmp(cid,"B2")==0) theOutput->AddIndicator(economicIndicator,"18.15 Exp. S.barley yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"RY")==0)                        theOutput->AddIndicator(economicIndicator,"18.16 Exp. Rye yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"OA")==0)                        theOutput->AddIndicator(economicIndicator,"18.17 Exp. Oat yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"PE")==0)                        theOutput->AddIndicator(economicIndicator,"18.18 Exp. Pea yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"PO")==0)                        theOutput->AddIndicator(economicIndicator,"18.19 Exp. Potato yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"SG")==0)                        theOutput->AddIndicator(economicIndicator,"18.20 Exp. Grass for seed yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"MA")==0)                        theOutput->AddIndicator(economicIndicator,"18.21 Exp. Maize yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"BE")==0)                        theOutput->AddIndicator(economicIndicator,"18.22 Exp. Beet yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"BP")==0)                        theOutput->AddIndicator(economicIndicator,"18.23 Exp. Barley/Pea yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"C1")==0 || strcmp(cid,"C2")==0) theOutput->AddIndicator(economicIndicator,"18.24 Exp. Grass/clover yield (harv. bef. 1/8)","t",y);
   if (strcmp(cid,"G1")==0 || strcmp(cid,"G2")==0) theOutput->AddIndicator(economicIndicator,"18.25 Exp. Grass ley yield (harv. bef. 1/8)","ha",y);
   if (strcmp(cid,"HV")==0)
      theOutput->AddIndicator(economicIndicator,"18.26 Exp. Dyerswoad yield (harv. bef. 1/8)","ha",y);
   if (strcmp(cid,"TR")==0)
      theOutput->AddIndicator(economicIndicator,"18.27 Exp. Triticale yield (harv. bef. 1/8)","ha",y);
   if (strcmp(cid,"FB")==0)
      theOutput->AddIndicator(economicIndicator,"18.28 Exp. FabaBean yield (harv. bef. 1/8)","ha",y);
   if (strcmp(cid,"CR")==0)
      theOutput->AddIndicator(economicIndicator,"18.29 Exp. CrimsonClover yield (harv. bef. 1/8)","ha",y);
}
/****************************************************************************\
Demands that the first crop is the main crop
\****************************************************************************/
bool ecosystem::GetReadyForHarvestOrLater()
{
   if (theCrops->NumOfNodes()>0)
      return (theCrops->ElementAtNumber(0)->ReadyForHarvest() || theCrops->ElementAtNumber(0)->GetTerminated());
   theMessage->WarningWithDisplay("ecosystem::ReadyForHarvestOrLater - function called but no crops are defined. May hamper the rest of the simulation.");
   return false;
}

/****************************************************************************\
Demands that the first crop is the main crop
\****************************************************************************/
bool ecosystem::GetIrrigationDemand()
{
   if (theCrops->NumOfNodes()>0)
      return theCrops->ElementAtNumber(0)->IrrigationDemand();
   return false;
}

/****************************************************************************\
Clover percentage
\****************************************************************************/
double ecosystem::CloverPercentage()
{
	double TotalDM = 0.0;
   double CloverDM = 0.0;
   for (int i=0;i<theCrops->NumOfNodes();i++)
   {
   	double DM = theCrops->ElementAtNumber(i)->GiveDryMatterVegTop();
      if (theCrops->ElementAtNumber(i)->GetCropName()=="Clover")
      	CloverDM += DM;
      TotalDM += DM;
   }
   double frac = 0.0;
   if (TotalDM>0.0) frac = CloverDM/TotalDM;
	return frac;
}

/****************************************************************************\
Improve terminate to return products!!!
Look upon indicator update!!!!
\****************************************************************************/
void ecosystem::KillAllCrops()
{
  double * RootLengthList=NULL;
   crop * aCrop;
   cloneList<crop>::PS aCropElement;

   theCrops->PeekHead(aCropElement);
   while (aCropElement!=NULL)
   {
      aCrop = aCropElement->element;
      if (aCrop)
      {
         if (!aCrop->IsBareCrop())
         {
            if (cutPlantProducts->NumOfNodes()>0)
               theMessage->WarningWithDisplay("ecosystem-KillAllCrops:: Cut plant products should not be present");
            decomposable* Root = new decomposable;
            plantItem* Straw = new plantItem;
            aCrop->Terminate(Straw,Root,RootLengthList);
            if (Straw)
            {
               Straw->Setamount(0.01*Straw->GetAmount()); // Convert from g/m2 to t/ha
               aSoil->AddDecomposable(Straw);
               theOutput->AddIndicator(environmentalIndicator,"32.21 N from straw incorporated into soil","kg N",Straw->GetAllN()*Straw->GetAmount()*area*1000.0);
               double C_Amount = Straw->GetAmount()*Straw->GetC_content()*area*1000.0;
               theOutput->AddIndicator(environmentalIndicator,"40.81 C from straw incorporated into soil","kg C",C_Amount);
//               theOutput->AddIndicator(environmentalIndicator,"40.99 Plant top res. C incorp. in org. matt.","kg C",C_Amount);
               delete Straw;
               Straw = NULL;
            }
            if (Root->GetAmount()>0)
            	TransferRootMatter(Root,RootLengthList);
            delete Root;
         }
      }
      theCrops->Remove(aCropElement);
   }
   aCrop=new cropbare("cropbare",0,this);
   theCrops->InsertLast(aCrop);
   if (theCrops->NumOfNodes()!=1)
      theMessage->FatalError("ecosystem::KillAllCrops - there should only be one (bare) crop here");
}

/****************************************************************************\
Uses the crop ID string to initiate the correct crop model
modified NJH May 2009, cropName allows variety to be passed via seed name for specific operations
\****************************************************************************/
crop * ecosystem::AllocateCrop(char * crop_id)
{
//	Setgrazable(false);
   crop * newCrop=NULL;
   if (strcmp(crop_id,"W1")==0 || strcmp(crop_id,"W2")==0)
      newCrop= new cropWwheat("cropWwheat",0,this);
   if (strcmp(crop_id,"RW")==0)
      newCrop= new cropWrape("cropWrape",0,this);
   if (strcmp(crop_id,"RS")==0)
      newCrop= new cropSrape("cropSrape",0,this);
   if (strcmp(crop_id,"B1")==0 || strcmp(crop_id,"B2")==0)
      newCrop= new cropSbarley("cropSbarley",0,this);
   if (strcmp(crop_id,"F1")==0 || strcmp(crop_id,"G1")==0 || strcmp(crop_id,"GR")==0 || strcmp(crop_id,"G4")==0)// || strcmp(crop_id,"C1")==0)   NJH removed this Nov 2007
   {
      newCrop= new cropRyegrass("cropRyegrass",0,this);
      if (theCrops->NumOfNodes()>0)
     		newCrop->SetUndersown(true); 
      else
         newCrop->SetUndersown(false);
   }
   if (strcmp(crop_id,"B5")==0 || strcmp(crop_id,"B6")==0)
      newCrop= new cropWbarley("cropWbarley",0,this);
   if (strcmp(crop_id,"PE")==0)
      newCrop= new cropPea("cropPea",0,this);
   if (strcmp(crop_id,"W5")==0)
      newCrop= new cropSpringWheat("cropSpringWheat",0,this);
   if (strcmp(crop_id,"BE")==0)
      newCrop= new cropBeet("cropBeet",0,this);
   if (strcmp(crop_id,"PO")==0)
      newCrop= new cropPotato("cropPotato",0,this);
   if (strcmp(crop_id,"MA")==0)
      newCrop= new cropMaize("cropMaize",0,this);
   if (strcmp(crop_id,"NO")==0)
      newCrop= new cropbare("cropbare",0,this);
   if ((strcmp(crop_id,"K1")==0)||(strcmp(crop_id,"C1")==0))
   {
      newCrop= new cropClover("cropClover",0,this);
      newCrop->SetUndersown(true); // ???!!!
   }
   if (strcmp(crop_id,"RY")==0)
      newCrop= new cropRye("cropRye",0,this);
   if (strcmp(crop_id,"O1")==0 || strcmp(crop_id,"O2")==0)
      newCrop= new cropOat("cropOat",0,this);
   if (strcmp(crop_id,"OR")==0)
      newCrop= new cropOilRadish("cropOilRadish",0,this);
   if (strcmp(crop_id,"CH")==0)
      newCrop= new cropChickory("cropChickory",0,this);
   if (strcmp(crop_id,"DW")==0)
      newCrop= new cropDyersWoad("cropDyersWoad",0,this);
   if (strcmp(crop_id,"HV")==0)
      newCrop= new cropHairyVetch("cropHairyVetch",0,this);
   if (strcmp(crop_id,"S1")==0)
   {
      newCrop= new cropSeedGrass("cropSeedGrass",0,this);
      newCrop->SetUndersown(true);
   }
   //new crops added NJH May 2009
   if (strcmp(crop_id,"IR")==0)
      newCrop= new cropItalRyegrass("cropItalRyegrass",0,this);
   if (strcmp(crop_id,"SB")==0)
      newCrop= new cropSoybean("cropSoybean",0,this);
   if (strcmp(crop_id,"SF")==0)
      newCrop= new cropSunflower("cropSunflower",0,this);
   if (strcmp(crop_id,"L1")==0)
   {
      newCrop= new cropLucerne("cropLucerne",0,this);
   }
   //new crops added NJH for compatibility with MEL, July 2009
   if (strcmp(crop_id,"CH")==0)
   {
      newCrop= new cropChickory("cropChickory",0,this);
      newCrop->SetUndersown(true); // ???!!!
   }
   if (strcmp(crop_id,"OR")==0)
   {
      newCrop= new cropOilRadish("cropOilRadish",0,this);
      newCrop->SetUndersown(true); // ???!!!
   }
   if (strcmp(crop_id,"DW")==0)
   {
      newCrop= new cropDyersWoad("cropDyersWoad",0,this);
      newCrop->SetUndersown(true);
   }
   if (strcmp(crop_id,"HV")==0)
	{
      newCrop= new cropHairyVetch("cropHairyVetch",0,this);
      newCrop->SetUndersown(true); 
   }
  if (strcmp(crop_id,"TR")==0)
	{
      newCrop= new cropTriticale("Triticale",0,this);
   
   }
     //
   if (strcmp(crop_id,"FB")==0)
	{
      newCrop= new cropFabaBean("FabaBean",0,this);
   }
    //
   if (strcmp(crop_id,"CR")==0)
	{
      newCrop= new cropCrimsonClover("CrimsonClover",0,this);
	 
   }
    //
    //
   if (strcmp(crop_id,"F2")==0 || strcmp(crop_id,"BP")==0 || strcmp(crop_id,"C2")==0
       || strcmp(crop_id,"B8")==0 || strcmp(crop_id,"G2")==0 || strcmp(crop_id,"G3")==0
       || strcmp(crop_id,"K2")==0 || strcmp(crop_id,"W8")==0
       || strcmp(crop_id,"B9")==0  || strcmp(crop_id,"S2")==0)
      theMessage->FatalError("ecosystem::AllocateCrop - crop ",crop_id," can not be sown directly");
   if (newCrop==NULL)
      theMessage->FatalError("ecosystem::AllocateCrop - crop ",crop_id," not found");
   newCrop->ReadParameters("CropParameters.dat");
   newCrop->AssignRootParameters(aSoil);
   return newCrop;
}


/****************************************************************************\
Initialise budget variables
\****************************************************************************/
void ecosystem::StartBudget()
{
	aSoil->StartBudget();
   if (theCrops)
   	for (int i=0;i<theCrops->NumOfNodes();i++)
	   	theCrops->ElementAtNumber(i)->StartBudget();
   WaterBudget.SetInput(snowContent+waterContent+aSoil->GetWater()+aSoil->GetSurfaceWater());
}

/****************************************************************************\
Check to see if budget is still valid
\****************************************************************************/
bool ecosystem::EndBudget()
{
	bool ret_val = true;
   double WaterRemain;
   double N, DM;
   if (theCrops)
   	for (int i=0;i<theCrops->NumOfNodes();i++)
	   	if (!theCrops->ElementAtNumber(i)->EndBudget(&N,&DM))
            ret_val=false;
   aSoil->EndBudget(&N,&WaterRemain);
   if (!WaterBudget.Balance(WaterRemain+waterContent+snowContent))
   	ret_val=false;
   if (!ret_val)
      theMessage->FatalError("ecosystem::EndBudget error");
   return ret_val;
}

/****************************************************************************\
Returns the nitrogen amount in the soil and the crop (kg/ha)
\****************************************************************************/
double ecosystem::NitrogenInSystem()
{
   return NitrogenInSoil()+NitrogenInCrops();
}


/****************************************************************************\
Terminate a crop
\****************************************************************************/
void ecosystem::TerminateCrop(crop * aCrop)
{
   double * RootLengthList = NULL;
   decomposable* Root = new decomposable;
   plantItem* Straw = new plantItem;
   aCrop->Terminate(Straw,Root,RootLengthList);
   if (Straw)
   {
      Straw->Setamount(0.01*Straw->GetAmount()); // Convert from g/m2 to t/ha
      aSoil->AddDecomposable(Straw);
      theOutput->AddIndicator(environmentalIndicator,"32.21 N from straw incorporated into soil","kg N",Straw->GetAllN()*Straw->GetAmount()*area*1000.0);
      theOutput->AddIndicator(environmentalIndicator,"40.81 C from straw incorporated into soil","kg C",Straw->GetAmount()*Straw->GetC_content()*area*1000.0);
      delete Straw;
      Straw = NULL;
   }
   if (Root->GetAmount()>0)
      TransferRootMatter(Root,RootLengthList);
}

/****************************************************************************\
Get all above ground N, including N in manure, crops etc in kg
\****************************************************************************/
double ecosystem::GetAbovegroundCropN()
{
	double amountN = 0.0;
   if (theCrops)
   {
   	for (int i=0;i<theCrops->NumOfNodes();i++)
      {
         crop * aCrop = theCrops->ElementAtNumber(i);
         amountN += aCrop->NitrogenInVegTop().n;
      }
   }
   return amountN*10.0*area;
}

/****************************************************************************\
Get all above ground N, including N in manure, crops etc in kg
\****************************************************************************/
double ecosystem::GetTotalAbovegroundN()
{
	double amountN = 0.0;
   if (cutPlantProducts)
   {
   	for (int i=0;i<cutPlantProducts->NumOfNodes();i++)
      {
         plantItem * aPlantProduct = cutPlantProducts->ElementAtNumber(i);
         amountN += aPlantProduct->GetAllN().n;
      }
   }
   return amountN*10.0;
}

/****************************************************************************\
Get max crop height (m)
\****************************************************************************/
double ecosystem::GetMaxCropHeight()
{
	double maxHeight = 0.0;
   if (theCrops)
   {
   	for (int i=0;i<theCrops->NumOfNodes();i++)
      {
         crop * aCrop = theCrops->ElementAtNumber(i);
         double ht = aCrop->GiveCropHeight();
         if (ht>maxHeight)
            maxHeight=ht;
      }
   }
   /*
   ------- Code BMP May 2006 - based on density and therefore romoved
   double totalDM=0.0;
   double density=0.0;
   cloneList<crop>::PS aCropElement;
   theCrops->PeekHead(aCropElement);
   crop* c;
   while (aCropElement!=NULL)
   {
      c = (cropRyegrass*)aCropElement->element;
      totalDM+=c->GiveDryMatterVegTop();
      density+=c->GetBulkDensity()*aCropElement->element->GiveDryMatterVegTop();
      theCrops->OneStep(aCropElement);
   }
   if (totalDM>0.0 && density>0)
   {
      density=density/totalDM; // Obtain the weighted average
      maxHeight=totalDM/density; // Height in m
   }
   */
   return maxHeight;
}

/****************************************************************************\
Called by the animal object to find out what is available
Returns quality and tonnes fresh weight per ha
\****************************************************************************/
void ecosystem::GetFeedResource(feedItem * my_resource, double grazing_height, int animalType)
{
//   double grazing_height= bite_depth_factor * GetMaxCropHeight();
   /* ???!!!@@@
   if (grazing_height<min_stubble_ht)
   	grazing_height = min_stubble_ht;
   */
   //calculate the total DM and N etc available within the ecosystem
   bool gotCrop = false;
   for(int i=0;i<theCrops->NumOfNodes();i++)
   {
      feedItem* aCropResource = theCrops->ElementAtNumber(i)->GetAvailability(grazing_height, animalType);
      // load the information into the grazing resource
      if (aCropResource!=NULL)
      {
         *my_resource + *aCropResource;
         delete aCropResource;
         gotCrop = true;
      }
   }
   if (!gotCrop)
         theMessage->FatalError("ecosystem:GetFeedResource - no grazable crop found");
   double FW = my_resource->GetAmount()/100.0;  // fresh weight, converted to tonnes per ha
   my_resource->Setamount(FW); // Is this correctly calculated ???!!!
}


/****************************************************************************\
Get mass of dry matter of ecosystem (kg)
\****************************************************************************/
double ecosystem::GetStandingDM()
{
   return GetStandingDMdensity() * area;
}

/****************************************************************************\
Get mass of dry matter of ecosystem (g/m**2)
\****************************************************************************/
double ecosystem::GetStandingDMdensity()
{
	double mass = 0.0;
   if (theCrops)
   {
   	for (int i=0;i<theCrops->NumOfNodes();i++)
      {
         crop * aCrop = theCrops->ElementAtNumber(i);
         mass += aCrop->GiveStandingDryMatter();
      }
   }
   return mass;
}
/****************************************************************************\
Get mass of dry matter (gDM/m**2)of ecosystem that is above a certain cutting
or grazing height (m)
\****************************************************************************/
double ecosystem::GetAvailableStandingDM(double height)
{
	double mass = 0.0;
   if (theCrops)
   {
   	for (int i=0;i<theCrops->NumOfNodes();i++)
      {
         crop * aCrop = theCrops->ElementAtNumber(i);
         mass += aCrop->GetAvailableStandingDM(height);
      }
   }
   return mass * area;
}

/****************************************************************************\
Sets the crop variables that record the material grazed
Called by each animal that grazes this ecosystem
\****************************************************************************/
void ecosystem::SetGrazed(double grazing_height, double DMGrazed)
{
   double grazableDMThisCrop, grazableNThisCrop;
   double grazableDM=0.0;
   double grazableN=0.0;
   double grazedN=  0.0;
   double grazedC = DMGrazed * 0.46;    //Here lies a well buried little fudge!!!
   for(int i=0;i<theCrops->NumOfNodes();i++)
   {
		grazableDM += (theCrops->ElementAtNumber(i))->GetgrazableDM();
		grazableN += (theCrops->ElementAtNumber(i))->GetgrazableN();
   }
	if (grazableDM>0.0)
   {
      for(int i=0;i<theCrops->NumOfNodes();i++)
      {
         grazableDMThisCrop = (theCrops->ElementAtNumber(i))->GetgrazableDM();
         double grazedDMThisCrop = DMGrazed * grazableDMThisCrop/grazableDM;
         grazableNThisCrop = (theCrops->ElementAtNumber(i))->GetgrazableN();
         (theCrops->ElementAtNumber(i))->SetGrazed(grazing_height, grazedDMThisCrop);
         if (grazableDMThisCrop>0.0) // Added BMP
            grazedN+=grazedDMThisCrop * grazableNThisCrop/grazableDMThisCrop;
      }
   }
   totalGrazed_DM+=DMGrazed;
   totalGrazed_N+= grazedN;
   theOutput->AddIndicator(environmentalIndicator,"31.23 N removed by cattle","kg N",grazedN*area*10.0);
   theOutput->AddIndicator(environmentalIndicator,"40.23 C removed by cattle","kg C",grazedC*area*10.0);
}

/****************************************************************************\
Add's two patches together
\****************************************************************************/
/*void ecosystem::Add(ecosystem* addPatch, double fraction)
{
   if (soilManure)
      *soilManure=*soilManure*(1.0-fraction);
   if (addPatch->soilManure)
      *(addPatch->soilManure)=*(addPatch->soilManure)*fraction;
   if (soilManure && addPatch->soilManure)
      *soilManure=*soilManure+*(addPatch->soilManure);
   if (!soilManure && addPatch->soilManure)
      *soilManure=*(addPatch->soilManure);
   if (cropManure)
      *cropManure=*cropManure*(1.0-fraction);
   if (addPatch->cropManure)
      *(addPatch->cropManure)=*(addPatch->cropManure)*(1.0-fraction);
   if (cropManure && addPatch->cropManure)
      *cropManure=*cropManure+*(addPatch->cropManure);
   if (!cropManure && addPatch->cropManure)
      *cropManure=*(addPatch->cropManure);
   irrigationWater=irrigationWater*(1.0-fraction);
   addPatch->irrigationWater=addPatch->irrigationWater*fraction;
   irrigationWater=irrigationWater+addPatch->irrigationWater;
   if (theCrops)
   	for (int i=0;i<theCrops->NumOfNodes();i++)
	   	theCrops->ElementAtNumber(i)->Add(addPatch->GetCrops()->ElementAtNumber(i),fraction);
   if (cutPlantProducts || addPatch->cutPlantProducts)
   	theMessage->Warning("Patch::Add - there should not be straw on ecosystem");
   if (cutPlantProducts)
      for (int i=0;i<cutPlantProducts->NumOfNodes();i++)
      	*(cutPlantProducts->ElementAtNumber(i))=*(cutPlantProducts->ElementAtNumber(i))*(1.0-fraction);
   if (addPatch->cutPlantProducts)
   	for (int i=0;i<addPatch->cutPlantProducts->NumOfNodes();i++)
      {
         *(addPatch->cutPlantProducts->ElementAtNumber(i))=*(addPatch->cutPlantProducts->ElementAtNumber(i))*fraction;
      	cutPlantProducts->InsertLast(addPatch->cutPlantProducts->ElementAtNumber(i));
      }
   aSoil->Add(addPatch->GetSoil(),fraction);
   waterContent = (1.0-fraction)*waterContent+fraction*addPatch->waterContent;
   snowContent = (1-fraction)*snowContent+fraction*addPatch->snowContent;
   tempSnowFall1 = (1.0-fraction)*tempSnowFall1+fraction*addPatch->tempSnowFall1;
   tempSnowFall2 = (1.0-fraction)*tempSnowFall2+fraction*addPatch->tempSnowFall2;
   snowMeltRate1 = (1.0-fraction)*snowMeltRate1+fraction*addPatch->snowMeltRate1;
   snowMeltRate2 = (1.0-fraction)*snowMeltRate2+fraction*addPatch->snowMeltRate2;
   totalgLAI = (1.0-fraction)*totalgLAI+fraction*addPatch->totalgLAI;
   leachingDepth = (1.0-fraction)*leachingDepth+fraction*addPatch->leachingDepth;
   balanceDepth = (1.0-fraction)*balanceDepth+fraction*addPatch->balanceDepth;
   k1 = (1.0-fraction)*k1+fraction*addPatch->k1;
   if (!overlap==addPatch->overlap)
   	overlap = false;
   if (!dung==addPatch->dung)
   	dung = false;
   if (!grazable==addPatch->grazable)
   	grazable = false;
   if (!fingerFlow==addPatch->fingerFlow)
   	fingerFlow = 0;
   WaterBudget.Add(addPatch->WaterBudget,fraction);
   area += addPatch->GetArea();
   birthday->SetTime(theTime.GetDay(),theTime.GetMonth(),theTime.GetYear());
}
*/

/****************************************************************************\
Get amount grazed from this ecosystem during the current period (kg)
\****************************************************************************/
double ecosystem::GetGrazedDM()
{
   double grazedDM=  0.0;
   for(int i=0;i<theCrops->NumOfNodes();i++)
		grazedDM += (theCrops->ElementAtNumber(i))->GetGrazedDM();
	return grazedDM * area * 10.0;
}

/****************************************************************************\
Returns growth in kg DM
\****************************************************************************/
double ecosystem::GetDailyDMProduction()
{
	double ret_val=0.0;
   for(int i=0;i<theCrops->NumOfNodes();i++)
   {
   	crop * aCrop = theCrops->ElementAtNumber(i);
      ret_val+=aCrop->GetDailyDMProduction();
   }
   ret_val*=area * 10.0;
   return ret_val;
}

/****************************************************************************\
Developed by JBE + BMP
\****************************************************************************/
void ecosystem::SimpleGraze()
{
   crop* aCrop=NULL;
   if (theCrops->NumOfNodes()==0)
      theMessage->FatalError("ecosystem::Graze - attempt to graze on field without plants");
   double totalDM=0.0;
   double density=0.0;
   cloneList<crop>::PS aCropElement;
   theCrops->PeekHead(aCropElement);
   while (aCropElement!=NULL)
   {
      aCrop = aCropElement->element;
      totalDM+=aCrop->GiveDryMatterVegTop();
      density+=aCrop->GetBulkDensity()*aCrop->GiveDryMatterVegTop();
      theCrops->OneStep(aCropElement);
   }
   double height=0.0;
   double fraction=1.0;
   double cuttingHeight = 0.09;
   if (totalDM>0.0)
   {
      density=density/totalDM; 		// Obtain the weighted average
      height=totalDM/density; 		// Height in m
      fraction=cuttingHeight/height;
   }
   else
      density=0.0;
   if (fraction>1.0) fraction=1.0;
//   double totHay=0.0;
   double Nremove = 0.0;            // used to simulate heifers

   theCrops->PeekHead(aCropElement);
   while (aCropElement!=NULL)
   {
      plantItem* Hay = new plantItem;
      aCrop = aCropElement->element;
      aCrop->Graze(Hay,fraction);
      double WasteFraction = 0.2;                   // This is a "harvest" waste, covering a number of different effects. Improve in later versions!
      if (Hay->GetAmount()>0.0)
      {
         plantItem* Waste = new plantItem(*Hay);
         Nremove += (Hay->GetAllN()*Hay->GetAmount()).n*(1.0-WasteFraction);      // unit g-N m-2
      	Hay->Setamount(Hay->GetAmount()*0.01*(1.0-WasteFraction)*area);     // Convert t
         theOutput->AddIndicator(environmentalIndicator,"31.23 N removed by cattle","kg N",Hay->GetAllN()*Hay->GetAmount()*1000.0);
         theOutput->AddIndicator(environmentalIndicator,"40.23 C removed by cattle","kg C",Hay->GetC_content()*Hay->GetAmount()*1000.0);

         Waste->Setamount(Waste->GetAmount()*WasteFraction*0.01);        // Convert t/ha
         aSoil->AddDecomposable(Waste);
   	   theOutput->AddIndicator(environmentalIndicator,"20.53 Plant top res. N incorp. in org. matt.","kg N",Waste->GetAllN()*Waste->GetAmount()*area*1000.0);
//		   theOutput->AddIndicator(environmentalIndicator,"40.99 Plant top res. C incorp. in org. matt.","kg C",Waste->GetAmount()*Waste->GetC_content()*area*1000.0);
      }
   	delete Hay;
      theCrops->OneStep(aCropElement);
   }

   double NitrogenLoad = nLoad;
   if (NitrogenLoad==0)
   	NitrogenLoad = grazingEffiency*Nremove;             // assumes that standard 84% of heifers is excreted

   if (NitrogenLoad>0.0)
   {
   	double NH4Loss = 0.15;
		manure cowManure;

		cowManure.Setname("CATTLE-SLURRY-FRESH");
      theProducts->GiveProductInformation(&cowManure);
      cowManure.Setamount(area*NitrogenLoad/cowManure.GetAllN().n/100.0);                         // Convert g/m2 to t
      theProducts->SubtractProduct(&cowManure);

      nitrogen LostNH4 = cowManure.GetNH4_content()*cowManure.GetAmount()*NH4Loss;                                       // assumes a loss of 15%
      cowManure.SetNH4_content(cowManure.GetNH4_content()*(1.0-NH4Loss));

      theOutput->AddIndicator(environmentalIndicator,"31.46 Ammonia-N loss during grazing","kg N",LostNH4*1000.0);
      theOutput->AddIndicator(environmentalIndicator,"31.09 Nitrogen from grazing animals","kg N",
                              cowManure.GetAllN()*cowManure.GetAmount()*1000.0+LostNH4*1000.0);
      theOutput->AddIndicator(environmentalIndicator,"41.10 C from grazing animals","kg C",
                              cowManure.GetC_content()*cowManure.GetAmount()*1000.0);
      theOutput->AddIndicator(economicIndicator,"10.21 Manure spread on field","t",cowManure.GetAmount());

      decomposable d=(decomposable) cowManure;
      d.Setamount(d.GetAmount()/area);                                                              // convert back to t/ha
		aSoil->AddDecomposable(&d);
   }
}

/****************************************************************************\
Returns mean potential growth in kg DM/ha/day
Is fairly crude - assumes total interception + even distribution between crops
\****************************************************************************/
double ecosystem::GetPotentialGrowthRate(double radiation, double temperature)
{
	double ret_val=0.0;
   int numCrops=theCrops->NumOfNodes();
   for(int i=0;i<numCrops;i++)
   {
   	crop * aCrop = theCrops->ElementAtNumber(i);
      ret_val+=aCrop->PotentialDryMatter(radiation, temperature) * 10.0;
   }
   ret_val/=numCrops;
   return ret_val;
}

/****************************************************************************\
\****************************************************************************/
void ecosystem::clearGrazingRecords()
{
	totalGrazed_DM=0.0;
   totalGrazed_N=0.0;
   for(int i=0;i<theCrops->NumOfNodes();i++)
		(theCrops->ElementAtNumber(i))->ClearTemporaryVariables();
}


void ecosystem::Initialise(char * soilFileName, int theIndex, double anArea)
{
	Index = theIndex;
  	aSoil->Initialize(soilFileName);
   balanceDepth=aSoil->GetMaximumDepth();
   area = anArea;
   StartBudget();
}



/****************************************************************************\
\****************************************************************************/
void ecosystem::SetirrigationWater(water *aWater)
{
	irrigationWater=*aWater;
   if (fingerFlow==1)
         irrigationWater.Setamount(3.0/2.0*irrigationWater.GetAmount());     // This part of the field receives an excess of water
   if (fingerFlow==2)
         irrigationWater.Setamount(0.0);                                     // This part of the field receives no water
}

//NJH May 2009, cropName allows variety to be passed via seed name for specific operations
void ecosystem::Sow(char * cropID, double SeedDensDryMatt,double RelativeDensity,double NitrogenInSeed)
{
   crop* aCrop=NULL;
   if ((theCrops->NumOfNodes()+1)>MaxPlants)
   {
      //cout << "Field " << Owner->GetIndex() << endl;
      theMessage->FatalError("ecosystem::Sow - Too many crops planted");
   }
   cloneList<crop>::PS aCropElement;
   theCrops->PeekHead(aCropElement);
   while (aCropElement!=NULL)
   {  // Remove all bare "crops"
      aCrop = aCropElement->element;
      if (aCrop->IsBareCrop() || aCrop->GetTerminated())
         theCrops->Remove(aCropElement);
      else
         theCrops->OneStep(aCropElement);
   }
	aCrop = AllocateCrop(cropID);    //changed NJH May 2009 to allow seed name to determine crop variety
	aCrop->Sow(SeedDensDryMatt,RelativeDensity,NitrogenInSeed);
	theCrops->InsertLast(aCrop);
};

/****************************************************************************\
\****************************************************************************/
void ecosystem::HarvestOperations(harvestFields * hrv)
{
	switch (hrv->GetOperationId())
   {
   	case  CombineHarvesting:
      case  CombineHarvestingShred:
      case  BeetHarvesting:
      case	BeetTopHarvesting:
      case  BeetTopChopping:
	            HarvestOp(hrv);
			      break;
      case  ForageHarvesting:
      case  Mowing:
      case  CutSetAside:
      			HarvestGrasslandOp(hrv);
      			break;
      case	GatheringBales:
      case  GatheringHay:
      case  GatheringSilage:
      case  GatheringCereal:
      			Gathering(hrv);
      			break;
      default:
      theMessage->FatalError("ecosystem::HarvestOperations - Operation not found");
   }
};


/****************************************************************************\
\****************************************************************************/
void ecosystem::HarvestOp(harvestFields * hrv)
{
   crop* aCrop=NULL;
/*   if (theCrops->NumOfNodes()>0)
      aCrop=theCrops->ElementAtNumber(0); // By default look at first crop;*/
   bool operationPerformed=false;
   double * RootLengthList=NULL;
   if (cutPlantProducts->NumOfNodes()>0)              // Check if straw has not been gathered or ploughed
      theMessage->WarningWithDisplay("patch::HandleOperation - There should not be straw on field at this point.");
/*
   // Expected yields
   for (int i=0;i<theCrops->NumOfNodes();i++)
      if (hrv->GetMainProduct(i))
         AddExpectedYieldToIndicators(theCrops->ElementAtNumber(i)->PlantItemName.c_str(),hrv->GetMainProduct(i)->GetAmount()*area);
*/
   int j=0;
   for (int i=0;i<theCrops->NumOfNodes();i++)
   {
      aCrop = theCrops->ElementAtNumber(i);
      const char* presCrop = aCrop->PlantItemName.c_str();
      if (!aCrop->IsUndersown() && !aCrop->GetTerminated()||(hrv->GetOperationId()==BeetHarvesting))
      {
         double fractionStrawLeft   = theControlParameters->GetStrawLeft(); // Fraction of straw biomass left by combine harvesting
         double fractionStorageLeft = theControlParameters->GetHarvestWaste(); // Covers waste by harvest and higher N-content in residues of reproductional plant parts
         if (fractionStorageLeft>0.25)
            theMessage->WarningWithDisplay("patch::patch - abnormal high waste by harvest");
         if (theControlParameters->GetOrganicFarm())
            fractionStorageLeft = 0.25;
         if (hrv->GetFractionStrawHarvested()>0)
            fractionStrawLeft = 1.0-hrv->GetFractionStrawHarvested();
         if (hrv->GetFractionGrainHarvested()>0)
            fractionStorageLeft = 1.0-hrv->GetFractionGrainHarvested();

         plantItem* Storage = new plantItem;
         plantItem* Straw = new plantItem;
         decomposable* Root = new decomposable;

         // Harvest crop
         double MaxRootDepth = aCrop->GiveRootDepth();
         theOutput->AddIndicator(economicIndicator,"17.04 Maximum root depth","m",MaxRootDepth);

         aCrop->Harvest(Storage,Straw);
         operationPerformed=true;

         scenarielog.WriteWithTabS(aCrop->CropName);
         scenarielog.WriteWithTabS(theTime.GetString2());
         if (Storage->GetAmount()>0.0)
         	scenarielog.WriteWithTab(Storage->GetAmount()*(1.0-fractionStorageLeft)*Storage->GetdryMatter());   //MEL 2009: These amounts are in DM
         else
        	 	scenarielog.WriteWithTab(0);
         if (Straw->GetAmount() > 0.0)
        	 	scenarielog.WriteWithTab(Straw->GetAmount()*(1.0-fractionStrawLeft)*Straw->GetdryMatter());         //MEL 2009: These amounts are in DM
         else
        	 	scenarielog.WriteWithTab(0);
         if (Storage->GetTotalN() > 0.0)
        	 	scenarielog.WriteWithTab((Storage->GetTotalN()*Storage->GetAmount()*(1-fractionStorageLeft)).n);
         else
        	 	scenarielog.WriteWithTab(0);
         if (Straw->GetAllN() > 0.0)
        	 	scenarielog.WriteWithTab((Straw->GetAllN()*Straw->GetAmount()*(1.0-fractionStrawLeft)).n);
         else
        	 	scenarielog.WriteWithTab(0);
         scenarielog.WriteNewLine();
         nitrogen StorageN = Storage->GetTotalN()*Storage->GetAmount();

         //amounts in patch.cpp are in g/sq metre. Multiply by 10 to get kg, divide by 100 to get tonnes

         double N_removed=StorageN.n*(1.0-fractionStorageLeft)*area*10.0;
         theOutput->AddIndicator(economicIndicator,"17.02 Grain yield at economic indicator date","t",Storage->GetAmount()*(1.0-fractionStorageLeft)*area/100.0);
         theOutput->AddIndicator(economicIndicator,"17.03 Straw yield at economic indicator date","t",Straw->GetAmount()*(1.0-fractionStrawLeft)*area/100.0);
         theOutput->AddIndicator(environmentalIndicator,"31.20 N removed in grain","kg N",N_removed);;
         theOutput->AddIndicator(environmentalIndicator,"40.20 C removed in grain","kg C",Storage->GetAmount()*Storage->GetC_content()*(1.0-fractionStorageLeft)*area*10.0);
         theOutput->AddIndicator(environmentalIndicator,"38.10 N15 harvest in grain","kg N",StorageN.n15*(1.0-fractionStorageLeft)*area*10.0);
         theOutput->AddIndicator(environmentalIndicator,"38.11 Total N15 harvest","kg N",StorageN.n15*(1.0-fractionStorageLeft)*area*10.0);
			theOutput->AddIndicator(environmentalIndicator,"39.10 Grain yield at environmental indicator date","t",Storage->GetAmount()*(1.0-fractionStorageLeft)*area/100.0);
			theOutput->AddIndicator(environmentalIndicator,"39.11 Straw yield at environmental indicator date","t",Straw->GetAmount()*(1.0-fractionStrawLeft)*area/100.0);

   		if (i==0)
      	{
      		bsTime aDate;
   			aDate = aCrop->GiveDateOfEmergence();
      		int DOY = aDate.GetDayInYear();
      		theOutput->AddIndicator(environmentalIndicator,	"39.20 Date of emergence of main crop","day of year",double(DOY));
   			aDate = aCrop->GiveDateOfFlowering();
      		DOY = aDate.GetDayInYear();
				theOutput->AddIndicator(environmentalIndicator,	"39.21 Date of flowering of main crop","day of year",double(DOY));
   			aDate = aCrop->GiveDateOfEndGrainFill();
      		DOY = aDate.GetDayInYear();
				theOutput->AddIndicator(environmentalIndicator,	"39.22 Date of end of grain filling of main crop","day of year",double(DOY));
				aDate = aCrop->GiveDateOfRipeness();
      		DOY = aDate.GetDayInYear();
         	theOutput->AddIndicator(environmentalIndicator,	"39.23 Date of ripeness of main crop","day of year",double(DOY));
      	}


#ifdef TUPLE
         if (((field*)Owner)->GetWriteTuples());
            ((field*)Owner)->AddTuple(theTime.GetString2(),-N_removed,Storage->GetAmount()*area*0.1,"HARVEST","MAIN_PRODUCT",0);
#endif
         AddYieldToIndicators(presCrop,(1.0-fractionStorageLeft)*area*Storage->GetAmount()/100.0,StorageN.n*(1.0-fractionStorageLeft)*area*10.0);
         AddHarvestedAreaToIndicators(presCrop,area);

         // Harvest waste
         plantItem * StorageWaste = new plantItem(*Storage);
         Storage->Setamount(Storage->GetAmount()*(1.0-fractionStorageLeft));
         StorageWaste->Setamount(StorageWaste->GetAmount()*fractionStorageLeft);
         theOutput->AddIndicator(environmentalIndicator,"32.23 N from storage incorporated into soil","kg N",StorageWaste->GetAllN()*StorageWaste->GetAmount()*area*10.0);
         theOutput->AddIndicator(environmentalIndicator,"40.83 C from storage incorporated into soil","kg C",StorageWaste->GetAmount()*StorageWaste->GetC_content()*area*10.0);
         StorageWaste->Setamount(0.01*StorageWaste->GetAmount()); //convert to t/ha
         aSoil->AddDecomposable(StorageWaste);
         delete StorageWaste;

         plantItem * StrawWaste = new plantItem(*Straw);
         Straw->Setamount(Straw->GetAmount()*(1.0-fractionStrawLeft));
         StrawWaste->Setamount(StrawWaste->GetAmount()*fractionStrawLeft);
         theOutput->AddIndicator(environmentalIndicator,"32.21 N from straw incorporated into soil","kg N",StrawWaste->GetAllN()*StrawWaste->GetAmount()*area*10.0);
         theOutput->AddIndicator(environmentalIndicator,"40.81 C from straw incorporated into soil","kg C",StrawWaste->GetAmount()*StrawWaste->GetC_content()*area*10.0);
         StrawWaste->Setamount(0.01*StrawWaste->GetAmount()); //convert to t/ha
         aSoil->AddDecomposable(StrawWaste);
         delete StrawWaste;

         if (Storage)
         {
            Storage->Setamount(Storage->GetAmount()*area*0.01);          // Convert to tonnes
            if (j==0)//&&(!hrv->GetMainProduct()))  //NICK CHECK DETTE (MEL)!!
               hrv->SetMainProduct(Storage);
            else
               hrv->AddMainProduct(Storage);
            j++;
         }
         else
            theMessage->Warning("patch::HandleOperation - Harvested amount should not be zero at this point.");

         if ((hrv->GetOperationId()==CombineHarvestingShred  || hrv->GetOperationId()==BeetHarvesting))
         {
            theOutput->AddIndicator(environmentalIndicator,"32.21 N from straw incorporated into soil","kg N",Straw->GetAllN()*Straw->GetAmount()*area*10.0);
            double C_Amount = Straw->GetAmount()*Straw->GetC_content()*area*10.0;
            theOutput->AddIndicator(environmentalIndicator,"40.81 C from straw incorporated into soil","kg C",C_Amount);
            Straw->Setamount(0.01*Straw->GetAmount());  //convert to t/ha
            aSoil->AddDecomposable(Straw);
            delete Straw;
         }
         else  // Straw is expected to be gathered
         {
            Straw->Setamount(Straw->GetAmount()*0.01);  //convert to tonnes/ha
            cutPlantProducts->InsertLast(Straw);
         }
         if(!aCrop->ContinueGrowth())
         {
         	plantItem* ExtraStraw = new plantItem;
         	aCrop->Terminate(ExtraStraw,Root,RootLengthList);
         	if (ExtraStraw->GetAmount()>0.0)
         		theMessage->WarningWithDisplay("ecosystem::Harvest crop not harvested correctly");
         	delete ExtraStraw;
         	TransferRootMatter(Root,RootLengthList);
         	delete Root;
         	delete Storage;   //NJH March 2009
         }

      }
      else
      {  // Fraction of undersown crop that is cut by ordinary harvest
         plantItem * Waste = new plantItem();
         aCrop->Cut(Waste,0.25);
         Waste->Setamount(Waste->GetAmount()*0.01);     // Convert to t/ha
         if (Waste->GetAmount()>0.0)
         {
            theOutput->AddIndicator(environmentalIndicator,"32.21 N from straw incorporated into soil","kg N",Waste->GetAllN()*Waste->GetAmount()*area*1000.0);
            theOutput->AddIndicator(environmentalIndicator,"40.81 C from straw incorporated into soil","kg C",Waste->GetAmount()*Waste->GetC_content()*area*1000.0);
            aSoil->AddDecomposable(Waste);
            delete Waste;
         }
         aCrop->SetUndersown(false);
      }
   }
   if (!operationPerformed)
      theMessage->FatalError("ecosystem::HarvestOp - no crops present for harvest in harvest operation");
}

/****************************************************************************\
\****************************************************************************/
void ecosystem::HarvestGrasslandOp(harvestFields * hrv)
{
   double cuttingHeight= hrv->GetCuttingHeight();
   if (cuttingHeight>0.5)
   {
      theMessage->WarningWithDisplay("patch::HandleOp - cutting height in forage harvesting is above 0.5 m, cm input assumed!");
      cuttingHeight=0.01*cuttingHeight;
   }
   if (GetMaxCropHeight()>0.0)               // This is returned in m
   {
      crop* c;
      int j=0;
      for (int i=0;i<theCrops->NumOfNodes();i++)
      {
         c = theCrops->ElementAtNumber(i);
		   const char* presCrop = c->PlantItemName.c_str();

         if (!c->GetTerminated())
         {
            /*
            TEST lines - REMOVE later !!!
            cropRyegrass * cr = (cropRyegrass *)c;
            feedItem * f=cr->GetAvailability(cuttingHeight,1);
            cout << "DM amount from 'GetAvailableStandingDM' without weighing: " << cr->GetAvailableStandingDM(cuttingHeight,false) << endl;
            cout << "DM amount from 'GetAvailableStandingDM' with weighing   : " << cr->GetAvailableStandingDM(cuttingHeight,true) << endl;
            cout << GetAvailablePatchStandingDMkg(cuttingHeight) << endl;
            */
            plantItem * forage = new plantItem();  //used to generate indicator output concerning whole harvested material. Should be deleted before exiting this function
            plantItem* Storage = new plantItem();  //used to store information on material harvested in grain and seed. Will be passed to products.cpp
            plantItem* Straw = NULL;  // used to store information on material harvested in straw or leaves and stem. Will be passed to products.cpp
				if (!hrv->GetOperationId()==CutSetAside)
            {
               if (!hrv->GetForageProduct(1)) //there is only one product from this operation. This is here to enable legacy code to operate
               {
                  plantItem * f=hrv->GetForageProduct(0);
                  if (f==NULL)
                        theMessage->FatalError("patch::HandleOp - no forage product defined for cut");
                  else
                     Straw=(plantItem*)f->clone(); // Set the right type of product
               }
            }
            else
	            Straw = new plantItem();

            forage->Setamount(0.0);
            double WasteFraction = 0.0;                // This is a "harvest" waste, covering a number of different effects. Improve in later versions!
            if (c->ContinueGrowth())
            {
               c->Cut(forage,cuttingHeight);     // Cutting height must be given in m
               *Straw + *forage;
			       Storage->Setamount(0.0);
                Straw->Setamount(Straw->GetAmount()*0.01);     // Convert to t/ha
                forage->Setamount(forage->GetAmount()*0.01);     // Convert to t/ha
            }
            else // This is not a crop that can be cut more than once
            {
               double plantHeight=c->GiveCropHeight();
               if (plantHeight>cuttingHeight)
               {
                  double fraction=1.0-cuttingHeight/plantHeight;
                  WasteFraction=1.0-fraction;
                  decomposable* Root = new decomposable;
		            if (!hrv->GetForageProduct(1)) //there is only one product. Still included to cope with legacy code
                  {
		               c->Cut(forage,cuttingHeight);     // Cutting height must be given in m
                     *Straw + *forage;
                      Storage->Setamount(0.0);
                      Straw->Setamount(Straw->GetAmount()*0.01);     // Convert to t/ha
		                forage->Setamount(forage->GetAmount()*0.01);     // Convert to t/ha
                  }
                  else
                  {
                     c->Harvest(Storage,Straw);
			            Storage->Setamount(Storage->GetAmount()*0.01);     // Convert to t/ha
			            Straw->Setamount(Straw->GetAmount()*0.01);     // Convert to t/ha
                     *forage + *Storage;
                     *forage + *Straw;
                  }
                  plantItem* ExtraStraw = new plantItem;
                  double * RootLengthList;
                  c->Terminate(ExtraStraw,Root,RootLengthList);
                  if (ExtraStraw->GetAmount()>0.0)
                     theMessage->WarningWithDisplay("patch::Harvest crop not harvested correctly");
                  delete ExtraStraw;
                  TransferRootMatter(Root,RootLengthList);
                  delete Root;
               }
            }
         	scenarielog.WriteWithTabS(c->CropName);
         	scenarielog.WriteWithTabS(theTime.GetString2());
         	scenarielog.WriteWithTab(0);  //no storage drymatter
            if (forage->GetAmount()>0.0)
        	 		scenarielog.WriteWithTab(forage->GetAmount()*forage->GetdryMatter());
         	else
        	 		scenarielog.WriteWithTab(0);
            scenarielog.WriteWithTab(0);   //no storage N
         	if (forage->GetTotalN() > 0.0)
        	 		scenarielog.WriteWithTab((forage->GetTotalN()*forage->GetAmount()).n);
         	else
        	 		scenarielog.WriteWithTab(0);
         	scenarielog.WriteNewLine();


/*            scenarielog.WriteWithTabS(c->CropName);
            scenarielog.WriteWithTabS(theTime.GetString2());
            scenarielog.WriteWithTab(forage->GetAmount()*forage->GetdryMatter());
            scenarielog.WriteWithTab((forage->GetTotalN()*forage->GetAmount()).n);  */
            if (forage->GetAmount()>0.0)
            {
               plantItem* Waste = new plantItem(*forage);
               forage->Setamount(forage->GetAmount()*(1.0-WasteFraction));
               Storage->Setamount(Storage->GetAmount()*(1.0-WasteFraction));
               Straw->Setamount(Straw->GetAmount()*(1.0-WasteFraction));
               Waste->Setamount(Waste->GetAmount()*WasteFraction);
               aSoil->AddDecomposable(Waste);
               theOutput->AddIndicator(environmentalIndicator,"32.21 N from straw incorporated into soil","kg N",Waste->GetAllN()*Waste->GetAmount()*area*1000.0);
               theOutput->AddIndicator(environmentalIndicator,"40.81 C from straw incorporated into soil","kg C",Waste->GetAmount()*Waste->GetC_content()*area*1000.0);
               delete Waste;
            }

            if (area<=0)
               theMessage->WarningWithDisplay("Patch::HandleOp - area shold be above zero");

            if (hrv->GetOperationId()==CutSetAside)
            {
               if (forage->GetAmount()>0.0)
               {
                  aSoil->AddDecomposable(forage);
                  theOutput->AddIndicator(environmentalIndicator,"32.21 N from straw incorporated into soil","kg N",forage->GetAllN()*forage->GetAmount()*area*1000.0);
                  theOutput->AddIndicator(environmentalIndicator,"40.81 C from straw incorporated into soil","kg C",forage->GetAmount()*forage->GetC_content()*area*1000.0);
                  delete forage;
                  delete Storage;
                  delete Straw;
               }
            }
            else
            {  // harvest products will be removed
               nitrogen forageN = forage->GetTotalN()*forage->GetAmount();
               string modpresCrop=presCrop;
               modpresCrop.append("WHOLE");
               AddYieldToIndicators(modpresCrop.c_str(),area*forage->GetAmount(),forageN.n*area*1000.0);
               AddHarvestedAreaToIndicators(presCrop,area);
               if (hrv->GetOperationId()==ForageHarvesting)
               {
                  forage->Setamount(forage->GetAmount()*area);
                  double n=forage->GetAllN().n*forage->GetAmount()*1000.0;
                  theOutput->AddIndicator(environmentalIndicator,"31.22 N removed in cut","kg N",n);
                  theOutput->AddIndicator(environmentalIndicator,"40.22 C removed in cut","kg C",forage->GetAmount()*forage->GetC_content()*1000.0);
				  		theOutput->AddIndicator(environmentalIndicator,"39.12 Cut at environmental indicator date","t",forage->GetAmount());
#ifdef TUPLE
                  if (((field*)Owner)->GetWriteTuples());
                     ((field*)Owner)->AddTuple(theTime.GetString2(),-n,forage->GetAmount()*10.0,"HARVEST","FORAGE/SILAGE",0);
#endif

                  if (j>10)
                     theMessage->FatalError("ecosystem::HarvestGrasslandOp - too many harvested products");
                  Storage->Setamount(Storage->GetAmount()*area);
                  hrv->SetForageProduct(Storage,j);
                  j++;
                  if (j>10)
                     theMessage->FatalError("ecosystem::HarvestGrasslandOp - too many harvested products");
                  Straw->Setamount(Straw->GetAmount()*area);
                  hrv->SetForageProduct(Straw,j);
                  j++;
                  delete forage;
               }
               else
               {
                  cutPlantProducts->InsertLast(Storage);
                  cutPlantProducts->InsertLast(Straw);
                  delete forage;
               }
            }
         }
      }
   }
   else
      theMessage->WarningWithDisplay("patch::HandleOp - crop has zero height at a cutting date");
};

void ecosystem::Gathering(harvestFields * hrv)
{
   if (cutPlantProducts->NumOfNodes()>0)
   {
      for (int i=0;i<cutPlantProducts->NumOfNodes();i++)  //cutPlantProducts in tonnes/ha
      {
//         plantItem* aCutProduct = new plantItem(*cutPlantProducts->ElementAtNumber(i));
         if (i>3)
            theMessage->FatalError("ecosystem::Gathering - too many harvested products");
         plantItem* aCutProduct = cutPlantProducts->ElementAtNumber(i);
         double Noutput = aCutProduct->GetAllN().n*aCutProduct->GetAmount()*area*1000.0;
         double Coutput = aCutProduct->GetAmount()*aCutProduct->GetC_content()*area*1000.0;
         aCutProduct->Setamount(aCutProduct->GetAmount()*area);
         if (hrv->GetOperationId()==GatheringBales)
         {
            theOutput->AddIndicator(environmentalIndicator,"31.21 N removed in straw","kg N",Noutput);
            theOutput->AddIndicator(environmentalIndicator,"40.21 C removed in straw","kg C",Coutput);
#ifdef TUPLE
            if (((field*)Owner)->GetWriteTuples());
               ((field*)Owner)->AddTuple(theTime.GetString2(),-Noutput,aCutProduct->GetAmount()*10.0,"HARVEST","SECONDARY_PRODUCT",0);
#endif
            if (i==0)//&&(!hrv->GetStrawProduct()))     //NICK CHECK DETTE (MEL)!!
               hrv->SetStrawProduct(aCutProduct);
            else
               hrv->AddStrawProduct(aCutProduct);
         }
         if (hrv->GetOperationId()==GatheringCereal)
         {
            if (i==0)   //is grain
            {

         		theOutput->AddIndicator(environmentalIndicator,"31.20 N removed in grain","kg N",Noutput);
               theOutput->AddIndicator(environmentalIndicator,"40.20 C removed in grain","kg C",Coutput);
#ifdef TUPLE
               if (((field*)Owner)->GetWriteTuples());
                  ((field*)Owner)->AddTuple(theTime.GetString2(),-Noutput,aCutProduct->GetAmount()*10.0,"HARVEST","MAIN_PRODUCT",0);
#endif
               hrv->SetMainProduct(aCutProduct);
            }
            if (i==1)   //is straw
            {
               theOutput->AddIndicator(environmentalIndicator,"31.21 N removed in straw","kg N",Noutput);
               theOutput->AddIndicator(environmentalIndicator,"40.21 C removed in straw","kg C",Coutput);
#ifdef TUPLE
               if (((field*)Owner)->GetWriteTuples());
                  ((field*)Owner)->AddTuple(theTime.GetString2(),-Noutput,aCutProduct->GetAmount()*10.0,"HARVEST","SECONDARY_PRODUCT",0);
#endif
               hrv->SetStrawProduct(aCutProduct);
            }
            if (i>1)   //is error
               theMessage->FatalError("Patch:: HandleOp - too many products in GatheringCereal");
         }
         if (hrv->GetOperationId()==GatheringSilage || hrv->GetOperationId()==GatheringHay)
         {
            theOutput->AddIndicator(environmentalIndicator,"31.22 N removed in cut","kg N",Noutput);
            theOutput->AddIndicator(environmentalIndicator,"40.22 C removed in cut","kg C",Coutput);
#ifdef TUPLE
            if (((field*)Owner)->GetWriteTuples());
               ((field*)Owner)->AddTuple(theTime.GetString2(),-Noutput,aCutProduct->GetAmount()*10.0,"HARVEST","FORAGE/SILAGE",0);
#endif
            if (hrv->GetOperationId()==GatheringSilage)
               aCutProduct->SetCode(1);
            if (hrv->GetOperationId()==GatheringHay)
               aCutProduct->SetCode(2);
            hrv->SetForageProduct(aCutProduct,i);
         }
      }
   }
   else
      theMessage->Warning("patch::HandleOperation - Straw already gathered.");
   cutPlantProducts->Reset();
};


void  ecosystem::Tillage(soilTreatFields *FOP)
{
	if(theTime.GetDay()==18&&theTime.GetMonth()==9&&theTime.GetYear()==2002)
	cout<<theTime<<endl;
	if (FOP->GetOperationId()==Ploughing)
   {
   	KillAllCrops();
		soilTreatFields * Tillage=(soilTreatFields *)FOP;
      double Depth = 0;
      Depth=10.0*Tillage->GetDepth();

   	double Incorporation = 0.95;       // Standard incorporated 95%
      Tillage->GetFractionIncorporated();
      if (Incorporation<=0.001)
         Incorporation = 0.95;       // Standard incorporated 95%
      Incorporation=1.0; // Otherwise error - correct later !!!
	  
	  aSoil->Incorporate(Depth,Incorporation,true,"Ploughing",Tillage->MinDepth()*10,Tillage->getPlantName());
   }

   if (FOP->GetOperationId()==StubHarrowing)
   {
   	// KillAllCrops(); // BMP added 17.11.2006 - and removed 20.11.2006, should be superflouos now!!!
   	soilTreatFields * Tillage=(soilTreatFields *)FOP;
      double Depth = 80;                                            // harrowing to 80 mm
      Depth = 10.0*Tillage->GetDepth();
      if (Depth<=10 || Depth>200)
         Depth = 80;                                   // harrowing to 80 mm
   	double Incorporation = 0.95;                                  // Standard incorporated 95%
      Incorporation = Tillage->GetFractionIncorporated();
      if (Incorporation<=0.001)
         Incorporation = 0.95;       // Standard incorporated 95%
      Incorporation=1.0; // Otherwise error - correct later !!!
	
      aSoil->Incorporate(Depth,Incorporation,true,"Harrowing",Tillage->MinDepth()*10,Tillage->getPlantName());
   }
};


/****************************************************************************\
Returns the nitrogen amount in the soil (kg/ha)
\****************************************************************************/
double ecosystem::NitrogenInSoil()
{
   double ret = aSoil->GetTotalNitrogen().n;
   ret=ret*area*10; // kg/ha
   return ret;
}

/****************************************************************************\
Returns the nitrogen amount in the crops (kg/ha)
\****************************************************************************/
double ecosystem::NitrogenInCrops()
{
   double ret=0.0;
   if (theCrops)
   {
      cloneList<crop>::PS aCropElement;
   	theCrops->PeekHead(aCropElement);
	   int cropNumber = theCrops->NumOfNodes();
   	for (int i=0;i<cropNumber;i++)
	   {
   		crop * aCrop = theCrops->ElementAtNumber(i);
      	if (aCrop->GetCropName()!="NoCrop")
            ret += aCrop->GiveTotalNitrogen().n;
      }
   }
   ret=ret*area*10; // kg/ha
   return ret;
}

/****************************************************************************\
Returns the nitrogen amount in the ecosystem (kg/ha)
\****************************************************************************/
double ecosystem::GetTotalNitrogen()
{
	return NitrogenInCrops()+NitrogenInSoil();
}
/****************************************************************************\
\****************************************************************************/
void ecosystem::WriteNitrogen(double f)
{
   double cropN=0;
   if (theCrops)
   {
      cloneList<crop>::PS aCropElement;
   	theCrops->PeekHead(aCropElement);
	   int cropNumber = theCrops->NumOfNodes();
   	for (int i=0;i<cropNumber;i++)
	   {
   		crop * aCrop = theCrops->ElementAtNumber(i);
      	if (aCrop->GetCropName()!="NoCrop")
            cropN += aCrop->GiveTotalNitrogen().n;
      }
   }
   cout << "N in crops " << cropN*f << endl;
   cout << "N in soil (total) " << aSoil->GetTotalNitrogen().n*f << endl;
   cout << "   Mineral N in soil surface " << aSoil->GetMineralNitrogenSurface().n*f << endl;
   cout << "   Organic N in soil surface " << aSoil->GetOrganicNitrogenSurface().n*f << endl;
   cout << "   Mineral N in soil profile " << aSoil->GetMineralNitrogenProfile().n*f << endl;
   cout << "   Organic N in soil profile " << aSoil->GetOrganicNitrogenProfile().n*f << endl;
   cout << "      N in AOM1 pool " << aSoil->GetPoolNitrogen("AOM1").n*f << endl;
   cout << "      N in AOM2 pool " << aSoil->GetPoolNitrogen("AOM2").n*f << endl;
   cout << "      N in SMB1 pool " << aSoil->GetPoolNitrogen("SMB1").n*f << endl;
   cout << "      N in SMB2 pool " << aSoil->GetPoolNitrogen("SMB2").n*f << endl;
   cout << "      N in SMR pool  " << aSoil->GetPoolNitrogen("SMR").n*f << endl;
   cout << "      N in NOM pool  " << aSoil->GetPoolNitrogen("NOM").n*f << endl;
   cout << "      N in IOM pool  " << aSoil->GetPoolNitrogen("IOM").n*f << endl;
}


void ecosystem::SetRUEfactor(double aVal)
{
	theCrops->ElementAtNumber(0)->SetRUEfactor(aVal);
}

void ecosystem::outputCropDetails(fstream *afile)
{


         int cropNumber = theCrops->NumOfNodes();
         *afile << "\t" << cropNumber;
         *afile << "\t" << CloverPercentage();

   if (theCrops)
   {
      cloneList<crop>::PS aCropElement;
      theCrops->PeekHead(aCropElement);
      for (int i=0;i<theCrops->NumOfNodes();i++)
      {
         crop * aCrop = theCrops->ElementAtNumber(i);
         aCrop->StreamKeyData(*afile);
      }
   }
}


/****************************************************************************\
Add's two patches together
\****************************************************************************/
/*void ecosystem::Add(patch* addPatch, double fraction)
{
   if (soilManure)
      *soilManure=*soilManure*(1.0-fraction);
   if (addPatch->soilManure)
      *(addPatch->soilManure)=*(addPatch->soilManure)*fraction;
   if (soilManure && addPatch->soilManure)
      *soilManure=*soilManure+*(addPatch->soilManure);
   if (!soilManure && addPatch->soilManure)
      *soilManure=*(addPatch->soilManure);
   if (cropManure)
      *cropManure=*cropManure*(1.0-fraction);
   if (addPatch->cropManure)
      *(addPatch->cropManure)=*(addPatch->cropManure)*(1.0-fraction);
   if (cropManure && addPatch->cropManure)
      *cropManure=*cropManure+*(addPatch->cropManure);
   if (!cropManure && addPatch->cropManure)
      *cropManure=*(addPatch->cropManure);
   irrigationWater=irrigationWater*(1.0-fraction);
   addPatch->irrigationWater=addPatch->irrigationWater*fraction;
   irrigationWater=irrigationWater+addPatch->irrigationWater;
   if (theCrops)
   	for (int i=0;i<theCrops->NumOfNodes();i++)
	   	theCrops->ElementAtNumber(i)->Add(addPatch->GetCrops()->ElementAtNumber(i),fraction);
   if (cutPlantProducts || addPatch->cutPlantProducts)
   	theMessage->Warning("Patch::Add - there should not be straw on patch");
   if (cutPlantProducts)
      for (int i=0;i<cutPlantProducts->NumOfNodes();i++)
      	*(cutPlantProducts->ElementAtNumber(i))=*(cutPlantProducts->ElementAtNumber(i))*(1.0-fraction);
   if (addPatch->cutPlantProducts)
   	for (int i=0;i<addPatch->cutPlantProducts->NumOfNodes();i++)
      {
         *(addPatch->cutPlantProducts->ElementAtNumber(i))=*(addPatch->cutPlantProducts->ElementAtNumber(i))*fraction;
      	cutPlantProducts->InsertLast(addPatch->cutPlantProducts->ElementAtNumber(i));
      }
   aSoil->Add(addPatch->GetSoil(),fraction);
   waterContent = (1.0-fraction)*waterContent+fraction*addPatch->waterContent;
   snowContent = (1-fraction)*snowContent+fraction*addPatch->snowContent;
   tempSnowFall1 = (1.0-fraction)*tempSnowFall1+fraction*addPatch->tempSnowFall1;
   tempSnowFall2 = (1.0-fraction)*tempSnowFall2+fraction*addPatch->tempSnowFall2;
   snowMeltRate1 = (1.0-fraction)*snowMeltRate1+fraction*addPatch->snowMeltRate1;
   snowMeltRate2 = (1.0-fraction)*snowMeltRate2+fraction*addPatch->snowMeltRate2;
   totalgLAI = (1.0-fraction)*totalgLAI+fraction*addPatch->totalgLAI;
   leachingDepth = (1.0-fraction)*leachingDepth+fraction*addPatch->leachingDepth;
   balanceDepth = (1.0-fraction)*balanceDepth+fraction*addPatch->balanceDepth;
   k1 = (1.0-fraction)*k1+fraction*addPatch->k1;
   if (!overlap==addPatch->overlap)
   	overlap = false;
   if (!dung==addPatch->dung)
   	dung = false;
   if (!grazable==addPatch->grazable)
   	grazable = false;
   if (!fingerFlow==addPatch->fingerFlow)
   	fingerFlow = 0;
   WaterBudget.Add(addPatch->WaterBudget,fraction);
   area += addPatch->GetArea();
   birthday->SetTime(theTime.GetDay(),theTime.GetMonth(),theTime.GetYear());
}
  */


/****************************************************************************
Volatilise ammonia (only called if manure is present)
NJH Aug 2007 - Relic code but contains some features that we might wish to use later
void ecosystem::Volatilise(double duration, bool kill)
{
   return; // Code presently not used in order to get movin' !!!!!!!!!!!!!!!!!!!!!!!!!

	double actualNH4_loss=0.0;
	double temperature, min_temp, max_temp, evap, infiltration, ppt, radiation,
                     windspeed, relHumidity, resistance, ra, rb, rCrop;
   if (soilManure)
   {
   	double Nin = soilManure->GetAllN().n*soilManure->GetAmount()*1000.0; // kg N/ha as soilManure is in t/ha
      manure *infiltManure = (manure *) soilManure->clone();
      // Below climate module is called, but not "ready" !!!
      theClimate->GetClimate(temperature, min_temp, max_temp, ppt, radiation, evap, windspeed, relHumidity);
      infiltration = 0.0;
      ppt /=24.0;  //convert to hourly rate
      evap/=24.0;
      // calculate infitration rate - this is a fudge!!!
      double currentWater = aSoil->GetAvailCapacity(aSoil->GetLayerStartDepth(0),aSoil->GetLayerStartDepth(1));
      double maxWater = aSoil->GetFieldCapacity(aSoil->GetLayerStartDepth(0),aSoil->GetLayerStartDepth(1));
      double diff = maxWater-currentWater;
      infiltration = diff * soilManure->GiveInfiltrationReduction()/24.0;

      double resistance = 0.0;
      if(windspeed<0.0001) windspeed=0.0001;
      rb=6.2*pow(windspeed,-0.67);				//Olesen & Summer 94  boundary layer res.
      double l=0.071;
      ra=(log(l/0.01))/(0.4*windspeed);		//------||--------    Aerodynamic res.
      rCrop=0.0;
      resistance=rb+ra+rCrop;                 		//rC is the crusts resistance to volatization

      // actualNH4loss is in tonnes per sq metre, as soilManure is in tonnes per ha
      double waterEvap = 0.0;
      actualNH4_loss=soilManure->VolatAmmonia(temperature, evap, infiltration, ppt, 10000.0, resistance, duration, &waterEvap, infiltManure);

      double remainingTAN = soilManure->GetAmount()*soilManure->GetNH4_content().n*1000.0; //amount remaining in kg per ha
      double remainTotN=(soilManure->GetAmount()*soilManure->GetAllN().n + infiltManure->GetAmount()*infiltManure->GetAllN().n)*1000.0;  //in kg per ha
      double test = Nin-(remainTotN+actualNH4_loss*1000.0);
      if (fabs(test)>0.1)
            theMessage->FatalError("ecosystem::Volatilise - balance error");

      // If manure contains little TAN then absorb remainder of soilManure. Also absorb if soilManure killed (eg by ploughing)
      if ((remainingTAN<1.0) || (kill))
      {
      	*infiltManure + *soilManure;
         delete soilManure;
         soilManure = NULL;
      }

      if (infiltManure->GetAmount()>0.0)
      {
         decomposable d=(decomposable) *infiltManure;
         aSoil->AddDecomposable(&d);
         aSoil->Incorporate(0.01,0.02); // From NJH @@@ Seems a very small depth !!!???
         delete infiltManure;
      }
   }
   theOutput->AddIndicator(environmentalIndicator,"31.45 Ammonia-N loss by spreading","kg N",actualNH4_loss * area *1000.0);
}

****************************************************************************/

