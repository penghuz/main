/****************************************************************************\
 $URL$
 $LastChangedDate$
 $LastChangedRevision$
 $LastChangedBy$
\****************************************************************************/
/****************************************************************************\
 Created by NJH - 2.6.99
  ----------------------------------------------------------------------------
      Log
 Describes a separate free-standing store)
\****************************************************************************/
#include "../base/common.h"
#include "dungheap.h"
#include "../base/IndicatorOutput.h"
#include "../base/climate.h"
#include "../base/controlParameters.h"

#include "../data/outputData.h"
/****************************************************************************\
  Destructor
\****************************************************************************/
dungheap::~dungheap()
{
   delete surfaceLayer;
}

/****************************************************************************\
  Constructor with arguments
\****************************************************************************/
dungheap::dungheap(const char *aName,const int aIndex,const base* aOwner)
 : manurestore(aName,aIndex,aOwner), building(aName,aIndex,aOwner)
{
	ventilated = false;
   surfaceLayer = new manurestore("surface", 0, this);
   surfaceLayer->SetnextStore(this);
   surfaceLayer->Setdepth(0.02);  //depth 20 cm
//   surfaceLayer->Setdepth(0.0);  //depth 20 cm
}

/****************************************************************************\
  Copy Constructor
\****************************************************************************/
dungheap::dungheap(dungheap& s): manurestore(s)
{
	theMessage->FatalError("dungheap:: copy constructor not implemented");
}


/**
 * 	Init reads the relevant values from its owners file
 */
void dungheap::ReadParameters(fstream * file)
{
	building::ReadParameters(file);
	manurestore::ReadParameters(file);
   aBase->Setfile(file);
   aBase->Setcur_pos(0);
   aBase->FindSection(aBase->GetName(),aBase->GetIndex());
	string isVentilated;
//   aBase->SetCritical();
   aBase->UnsetCritical();
	aBase->GetParameter("Ventilated",&isVentilated);

   if (isVentilated=="TRUE") ventilated = true;
   else ventilated = false;
   aBase->UnsetCritical();
   aBase->Setfile(NULL);
   surfaceLayer->SetsurfaceArea(surfaceArea);
   surfaceLayer->SetCapacity(surfaceArea * surfaceLayer->Getdepth());
   surfaceLayer->SetOMdegRateConst(OMdegRateConst);
  	manure * aManure=new manure(*GetManure());
   aManure->Setamount(0);
   surfaceLayer->SetManure(aManure);
   aBase->Setfile(NULL);
   if (theOutputData->DetailsData.getManureManageDetails())
		OpenDetailsFile();
   else
   	detailsFile=NULL;
}


/**
 * Calculated the Ammonia Emission, N2 Emission, N2 OEmission, CO2 Emission and CH4 Emission. Those are added to the Indicat file
 */
void dungheap::DailyUpdate()
{
	building::DailyUpdate();
   double volatAmmonia=0.0, N2Emission=0.0, N2OEmission=0.0, CH4Emission=0.0, CO2Emission=0.0;

	switch (theControlParameters->GetvolatModel()){
   case 0:{
   			double Nlost,Clost;
   			Volatize(&Nlost,&Clost,&volatAmmonia, &N2Emission, &N2OEmission, &CH4Emission, &CO2Emission);
            Nbudget.AddOutput(Nlost);
            Cbudget.AddOutput(Clost);
   		 }
	   break;
   case 1: VolatizeDung(&volatAmmonia, &N2Emission, &N2OEmission, &CO2Emission, &CH4Emission);
   	break;
   }
	manurestore::DailyUpdate(volatAmmonia, N2Emission, N2OEmission, CO2Emission, CH4Emission);
   theOutput->AddIndicator(environmentalIndicator,"35.43 NH3-N volatilisation from manure storage","kg N",volatAmmonia*1000.0);
   theOutput->AddIndicator(environmentalIndicator,"35.44 N2-N volatilisation from manure storage","kg N",N2Emission*1000.0);
   theOutput->AddIndicator(environmentalIndicator,"35.45 N2O-N volatilisation from manure storage","kg N",N2OEmission*1000.0);
   theOutput->AddIndicator(environmentalIndicator,"45.43 CO2-C volatilisation from manure storage","kg C",CO2Emission*1000.0);
   theOutput->AddIndicator(environmentalIndicator,"45.44 CH4-C volatilisation from manure storage","kg C",CH4Emission*1000.0);
   GiveIndicator(environmentalIndicator);
}
/**
 * Calculate the total amount of slurry, n and c. Used for the indicat
 */
void dungheap::GiveIndicator(int indicatorType)
{
 	double Vt=0,N=0, C=0;
   double tempV=0.0;
   manure * aManure=(manure *) surfaceLayer->GetManure();

   if (aManure)
   {
	   Vt=aManure->GetAmount();           //The total amount of slurry in kg
   	N=Vt*aManure->GetTotalN().n  ;            //The total N-fraction
      C=Vt*aManure->GetC_content()  ;
   }

   aManure=(manure *) theStore->GetProductPointer();

   if (aManure)
   {
	   tempV=aManure->GetAmount();           //The total amount of slurry in kg
   	N+=tempV*aManure->GetTotalN().n  ;            //The total N-fraction
      C+=tempV*aManure->GetC_content()  ;
      Vt+=tempV;
   }
 //  cout << " V " << Vt << " N " << N << " C " << C << endl;
	if (external)
   {
    if (indicatorType==environmentalIndicator)
    {
       theOutput->AddStateIndicator(environmentalIndicator,"35.60 N bound in manure storage","kg N",N*1000.0);
       theOutput->AddStateIndicator(environmentalIndicator,"45.60 C bound in manure storage","kg C",C*1000.0);
    }
    theOutput->AddStateIndicator(economicIndicator,"11.71 Dung in dungheap","t",Vt/1000);
   }
}


void dungheap::CalcLP()
{
 manurestore::CalcLP();
}

/*
 * Calculated Ammonia Emission, N2 Emission, N2O Emission, CO2Emission and CH4Emission. Used in dynamicly modeling
 */
void dungheap::VolatizeDung(double *volatAmmonia, double *N2Emission, double *N2OEmission, double *CO2Emission, double *CH4Emission)
{
	if (surfaceLayer->GetAmountInManureStore()>0.0)
   {

      double waterEvap=0.0;
      double rb,ra,resistance;

      double Ameantemp,Amin_temp,Amax_temp,Aprecip,Aradiation,AEPot,Awindspeed, Ahumidity;

      theClimate->GetClimate(Ameantemp, Amin_temp,Amax_temp, Aprecip, Aradiation,AEPot, Awindspeed, Ahumidity);

      if(Awindspeed<0.0001)
    	  Awindspeed=0.0001;

      rb=6.2*pow(Awindspeed,-0.67);				//Olesen & Summer 94  boundary layer res.   86400 (s->day) inserted by JB

      double l=0.071;
      ra=(log(l/0.001))/(0.4*Awindspeed);		//------||--------    Aerodynamic res.
      double coverResistance=0.0;
      double specificResistance=0.0;                             //note fudge below
      resistance=2000.0 *(rb+ra+coverResistance+specificResistance);                 		//rC is the crusts resistance to volatization

      surfaceLayer->Volatize(Ameantemp, Ahumidity, Aprecip, surfaceArea, resistance,
                                 volatAmmonia, &waterEvap, N2Emission, N2OEmission, CO2Emission, CH4Emission);

   /*   *theMessage->GiveDebugHandle() << theTime.GetDay() << "," << theTime.GetMonth() << "," << theTime.GetYear() << ","
       << Ameantemp << "," << volat*1000.0 << endl;
      theMessage->CloseDebugHandle();*/
   }
}

/**
 * receive manure. Accepted manure is:"FLUID-MANURE","SOLID-MANURE","WASHWATER","STRAW","WATER","PIG-FYM" and "CATTLE-FYM"
 * It cannot mix "CATTLE-FYM" and "PIG-FYM".
 * If there is not enough room for it on the surface some manure will be transfered to the store
 */
void dungheap::RcvManure(const manure* someManure)
{
	theOutput->AddIndicator(economicIndicator,"19.73 Solid manure produced","t",someManure->GetAmount());
	manure *tmpManure=(manure*) someManure->clone();
   if (strcmp(theStore->GetProductPointer()->GetName(),"SLURRY")==0)
		theMessage->FatalError(manurestore::GetName()," ","dungheap: attempt to add wrong manure type");

   char theManureName[200];
   strcpy(theManureName,surfaceLayer->GetManure()->GetName());
   if ((strcmp(tmpManure->GetName(),"CATTLE-FYM")==0) && (strcmp(theManureName,"PIG-FYM")==0))
   {
      theMessage->FatalError("dungheap::RcvManure - trying to add cattle FYM to pig FYM");
   }
   if ((strcmp(tmpManure->GetName(),"PIG-FYM")==0) && (strcmp(theManureName,"CATTLE-FYM")==0))
   {
      theMessage->FatalError("dungheap::RcvManure - trying to add pig FYM to cattle FYM");
   }

  	if ((strcmp(tmpManure->GetName(),"FLUID-MANURE")==0)||(strcmp(tmpManure->GetName(),"SOLID-MANURE")==0)
      		||(strcmp(tmpManure->GetName(),"WASHWATER")==0)||(strcmp(tmpManure->GetName(),"STRAW")==0)
            ||(strcmp(tmpManure->GetName(),"WATER")==0)||(strcmp(tmpManure->GetName(),"PIG-FYM")==0)||
            (strcmp(tmpManure->GetName(),"CATTLE-FYM")==0))
      {
         Nbudget.AddInput(tmpManure->GetAmount()*tmpManure->GetAllN().n);
         Cbudget.AddInput(tmpManure->GetAmount()*tmpManure->GetC_content());
         tmpManure->Setname(theManureName);
         manure * aManure = new manure();
         aManure->Setname(theManureName);
         if (tmpManure->GetAmount()>=surfaceLayer->GetCapacity())
         {
         	double toEmpty= surfaceLayer->GetAmountManure();
            surfaceLayer->GetFromManureStore(toEmpty,aManure);  //empty surface layer
            theStore->AddToStore(aManure);  //send to main store
            aManure->Setamount(0.0);  //zero
            *aManure = *tmpManure;
            aManure->Setamount(surfaceLayer->GetCapacity());  //Set equal to surface capacity
            tmpManure->Setamount(tmpManure->GetAmount()-surfaceLayer->GetCapacity()); //remove this amount from manure sent
            surfaceLayer->RcvManure(aManure); //fill surface layer
            theStore->AddToStore(tmpManure);  //send rest to main store
         }
         else  //need to move only some of surface layer to main store
         {
            double surplus = surfaceLayer->GetAmountManure()+tmpManure->GetAmount()-surfaceLayer->GetCapacity();
            if (surplus > 0.0) // surface layer either full or will be filled
            {
	            if (surplus<tmpManure->GetAmount())  //layer will be filled with this addition
               {
                  aManure=(manure *) surfaceLayer->GetFromManureStore(surplus,tmpManure);  //make space for the new manure
                  theStore->AddToStore(aManure);  //send to main store
               }
               else
               {
                  aManure=(manure *) surfaceLayer->GetFromManureStore(tmpManure->GetAmount(),tmpManure);  //make space for the new manure
                  theStore->AddToStore(aManure);  //send to main store
               }
            }
            surfaceLayer->RcvManure(tmpManure);
         }
         delete aManure;
//                  theStore->AddToStore(tmpManure);  //send to main store
      }
   else
		theMessage->FatalError(manurestore::GetName()," ","manurestore: attempt to add wrong manure type");

   delete tmpManure;  //
}


/****************************************************************************\
	Operator << for output
\****************************************************************************/

ostream& operator<< (ostream& os,dungheap s)
{
/*  os << setiosflags(ios::left) << setw(40) << s.rT << endl;
  os << setiosflags(ios::left) << setw(40) << "Cover resistance " << s.rCover << endl;*/
  return os;
}
