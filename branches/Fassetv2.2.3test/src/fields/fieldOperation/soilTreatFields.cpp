/****************************************************************************\
 $URL$
 $LastChangedDate$
 $LastChangedRevision$
 $LastChangedBy$
\****************************************************************************/
#include "../../base/common.h"
#include "soilTreatFields.h"
#include "../../products/products.h"

/****************************************************************************\
\****************************************************************************/
soilTreatFields::soilTreatFields()
{
   Depth = 0;
   FractionIncorporated = 0;
   minDepth=0;
   plantName="all";
}

/****************************************************************************\
\****************************************************************************/
soilTreatFields::~soilTreatFields()
{
}

/****************************************************************************\
\****************************************************************************/
soilTreatFields* soilTreatFields::clone() const
{
	soilTreatFields* s=new soilTreatFields(*this);
   s->Depth = Depth;
	s->FractionIncorporated = FractionIncorporated;
	s->plantName = plantName;
	return s;
}

/**
 * Calculate the cost of this operation.
 * If update is true it will write the cost to the indicate file.
 * If update is false it will return the cost in the cost variable
 */
bool soilTreatFields::CalcCost(linkList<operation>    * aOperationList,
                                  linkList<techEquip>     * aTechFarmList,
                                  calcLPType               mode,
                                  double                 * cost,
                                  bool                  update)
{
   if (!update && ((mode==hoursCombine) || (mode==hoursAnimalFert)
      || (mode==hoursMineralFert) || (mode==hoursPlantCare)
      || (mode==varCostAnimalFert) || (mode==varCostMineralFert)))
      return false;
   // Calculate cost of operation
   linkList<operation>::PS aOperationNode;

   aOperationList->PeekHead(aOperationNode);
   while(aOperationNode!=NULL)
   {
      if(aOperationNode->element->GetoperationNumber()==op_id)
         break;
      aOperationNode=aOperationNode->next;
   }

   if(aOperationNode==NULL)
   {
      theMessage->WarningWithDisplay("soilTreatFields::CalcCost - Operation ",operationNameList[op_id].c_str()," not found in list");
      return false;
   }
   else
      if(area<=0.0)
      {
         theMessage->WarningWithDisplay("soilTreatFields::CalcCost - area is less than zero");
         return false;
      }
      else
      {
         soilTreatTech * soilTreatTechInstance =
            (soilTreatTech*)aOperationNode->element;

         if (!update && soilTreatTechInstance->SameConditionsSoilTreat(area,distance))
         {
            *cost=soilTreatTechInstance->OldCost();
            return true;
         }
         soilTreatTechInstance->Setarea(area);
         soilTreatTechInstance->Setdistance(distance);
         soilTreatTechInstance->SetoperationName(operationNameList[op_id]);
         soilTreatTechInstance->SetcropName(cropName);

         if (soilTreatTechInstance->CreateGang(aTechFarmList))
         {
            // All neccesary machines present at farm
	         if(update)
	            soilTreatTechInstance->Update(aTechFarmList);
	         else
	         {
	            soilTreatTechInstance->CalcCost(mode);
	            *cost=soilTreatTechInstance->Getcost();
	         }
         }
         else {  // Buy service
	         if(update)
            {
               product * hiredService = new product;
					hiredService->Setname("MachinePoolCost");
					theProducts->GiveProductInformation(hiredService);
					hiredService->Setamount(soilTreatTechInstance->GetMachinePoolPrice()*area);
               // theProducts->AddProduct(hiredService);Removed BMP 12.10.2000
               soilTreatTechInstance->UpdateProducts(true);
               theOutput->AddIndicator(economicIndicator,"07.08 Cost of machine contractors","Dkr",-soilTreatTechInstance->GetMachinePoolPrice()*area); 
				   delete hiredService;
            }
	         else
               if (mode==varCostTotalNotFert)
	               *cost=-soilTreatTechInstance->GetMachinePoolPrice();
               else
                  *cost=0.0;
         }
         soilTreatTechInstance->DeleteGang();
         return true;
      }
}

