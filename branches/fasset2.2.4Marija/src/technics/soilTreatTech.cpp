/****************************************************************************\
 $URL$
 $LastChangedDate$
 $LastChangedRevision$
 $LastChangedBy$
\****************************************************************************/
#include "../base/common.h"
#include "soilTreatTech.h"
#include "../products/energy.h"
#include "../products/products.h"

/****************************************************************************\
  Constructor with arguments
\****************************************************************************/
soilTreatTech::soilTreatTech(const char * aName, const int aIndex, const base * aOwner)
   : fieldOperationTech(aName, aIndex, aOwner)
{
   k = 0;
   firstFurrow = 0.0;
   markHeadland = 0.0;
}

/****************************************************************************\
  Destructor
\****************************************************************************/
soilTreatTech::~soilTreatTech()
{
}

/****************************************************************************\
\****************************************************************************/
void soilTreatTech::ReadParameters(fstream * file)
{
   fieldOperationTech::ReadParameters(file);
   Setfile(file);
   GetParameter("firstFurrow",&firstFurrow);
   GetParameter("markHeadland",&markHeadland);
   GetParameter("k",&k);
   Setfile(NULL);
}

/****************************************************************************\
 Function:   CalcCost
 Purpose:    Calculates cost for different modes corresponding to file types
             for the LP model
\****************************************************************************/
void soilTreatTech::CalcCost(calcLPType mode)
{
   switch(mode)
   {
      case hoursTotalNotCombineNotFert:
         CalcGangHours();
         cost=aGang->GetgangHours();
         oldCost=cost;
         oldArea=area;
         oldDistance=distance;
         break;
      case varCostTotalNotFert:
         CalcGangHours();
         CalcGangFuel();
         CalcGangMaintenance();
         cost = -1.0*(aGang->GetgangFuel()*theProducts->GetExpectedBuyPriceYear1("DIESEL") +
                      aGang->GetgangMaintenance());
         oldCost=cost;
         oldArea=area;
         oldDistance=distance;
         break;
      default: cost=0.0;
   }
}

/****************************************************************************\
\****************************************************************************/
void soilTreatTech::CalcGangHours()
{
   double    size = 0.0;
   techEquip aMainTechEquip("maintechequip",0,this);

   aMainTechEquip=*(aGang->GetMainTechEquip());
   size = aMainTechEquip.Getsize();

   if(workingSpeed==0)
      theMessage->FatalError("workingSpeed = 0 in operation ",
                            (char*)operationName.c_str());
   if(size==0)
      theMessage->FatalError("size = 0 in operation ",
                            (char*)operationName.c_str());

   double hours= (
      (area*600)/(workingSpeed*size) +
      (turningTime*sqrt(area*10000/2))/size +
      k*(14.6162*(pow(size,-0.35555)))+
      turningHeadland+
      firstFurrow*((sqrt(area*10000/2))*2*ceil(sqrt(area*10000/2)/100)*4.31/100)+
      markHeadland*(sqrt(area*10000/2)*2*3.45/100)+
      (disturbanceAllowance*area)
      )*(1+relaxationAllowance)*1.1 / 60;

   aGang->SetgangHours(hours);
}

/****************************************************************************\
\****************************************************************************/
void soilTreatTech::UpdateProducts(bool contractor)
{
   fieldOperationTech::UpdateProducts(contractor);
}


