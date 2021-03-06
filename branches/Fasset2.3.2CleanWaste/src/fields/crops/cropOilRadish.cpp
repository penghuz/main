#include "../../base/common.h"
#include "cropOilRadish.h"
#include "../../products/products.h"

/****************************************************************************\
\****************************************************************************/
cropOilRadish::cropOilRadish(const char * aName, const int aIndex, const base * aOwner,string cropName)
              :cropBeet(aName,aIndex,aOwner,cropName)
{
   // For complete nomenclature see base class 'crop'

   PlantItemName  			= "OILRADISH";                 // no primarly harvest item
   StrawItemName  			= "OILRADISH";           // These names maps to products.dat !
	WinterSeed     			= true;                     // Is the present crop a wintercrop.
   GrowthContinue 			= false;


}
/****************************************************************************\
\****************************************************************************/
double cropOilRadish::Update(double ActivePar)
{
  	double DailyDMProd = cropBeet::Update(ActivePar);
   return DailyDMProd;
}
/****************************************************************************\
Returns cut material in g/m2. Oilradish has belowground storage organs.
Therefore no cut of storage organs
\****************************************************************************/
void cropOilRadish::Cut(plantItem * cutPlantMaterial, double CutHeight)
{
   nitrogen NVegTop = NitrogenInVegTop();

   CutOrHarvested = true;
   if (DryMatterVegTop>0.0)
   {
      double fraction=0.0;
      if (PlantHeight>0.0)
         fraction=1.0-CutHeight/PlantHeight;
   	if (fraction<=0.0)
      {
         fraction=0.0;
         theMessage->WarningWithDisplay("cropOilRadish::Cut - crop to low to cut");
      }
   	else
   	{
      double CutDM =  fraction*DryMatterVegTop;

      cutPlantMaterial->Setname(PlantItemName);
      theProducts->GiveProductInformation(cutPlantMaterial);
      cutPlantMaterial->Setamount(CutDM/cutPlantMaterial->GetdryMatter());

      // Nitrogen
      nitrogen CutN = NVegTop*fraction;

		if (cutPlantMaterial->GetAmount()>0)
         cutPlantMaterial->SetorgN_content(CutN/cutPlantMaterial->GetAmount());

   Nitrogen = Nitrogen - CutN;
     DryMatterVegTop    -= max(0.0,(CutDM));

      if (DryMatterVegTop<0 || CutN.n<0 || Nitrogen.n<0 || CutDM<0)
      	theMessage->FatalError("CropOilRadish:: Values at harvest should not be negative");

      Nbudget.AddOutput(CutN.n);                    // Budgets
      N15budget.AddOutput(CutN.n15);                    // Budgets
      DMbudget.AddOutput(CutDM);

      GreenCropAreaIndex = (1.0-fraction)*GreenCropAreaIndex;
      YellowCropAreaIndex= (1.0-fraction)*YellowCropAreaIndex;
      PlantHeight        = CutHeight;
   	}
  	}
   double NRemain, DMRemain;
   EndBudget(&NRemain,&DMRemain);
 }




