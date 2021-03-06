/****************************************************************************\
 $URL$
 $LastChangedDate$
 $LastChangedRevision$
 $LastChangedBy$
\****************************************************************************/
#include "../base/common.h"
#include "cropRotation.h"
#include "../base/bstime.h"
#include "../base/climate.h"
#include "../base/IndicatorOutput.h"
#include "staticCrops/staticWwheat.h"
#include "staticCrops/staticSwheat.h"
#include "staticCrops/staticSbarley.h"
#include "staticCrops/staticWbarley.h"
#include "staticCrops/staticWrape.h"
#include "staticCrops/staticSrape.h"
#include "staticCrops/staticFallow.h"
#include "staticCrops/staticFallowPermanent.h"
#include "staticCrops/staticPea.h"
#include "staticCrops/staticBarleyPea.h"
#include "staticCrops/staticCloverGrass.h"
#include "staticCrops/staticSeedGrass.h"
#include "staticCrops/staticBeet.h"
#include "staticCrops/staticMaize.h"
#include "staticCrops/staticGrass.h"
#include "staticCrops/staticGrassEstablished.h"
#include "staticCrops/staticGrassPermanent.h"
#include "staticCrops/staticOat.h"
#include "staticCrops/staticRye.h"
#include "staticCrops/staticPotato.h"
#include "../legislation/legislation.h"
#include "../base/message.h"
#include "staticCrops/staticClover.h"
#include "staticCrops/staticGrazingGrass.h"
#include "staticCrops/staticItalianRyeGrass.h"
#include "../base/controlParameters.h"
#include "staticCrops/staticLucerneEstablished.h"
#include "../data/systemData.h"
// -------------- Initialize static members ----------------------------------
	linkList<field>::PS cropRotation::GlobalP=NULL;
	int cropRotation::field_no_=0;
	int cropRotation::soil_type_=0;
	cropRotation * cropRotation::ths=NULL;
   double cropRotation::manureAdjustment = 1.0;
// ---------------------------------------------------------------------------

/****************************************************************************\
Constructor.
\****************************************************************************/
cropRotation::cropRotation()
{
	CropPointer=&staticBase;
	InitLegalTable();
	InitReducTable();
	PvName="PEST";
	FertLevName="F";
	FieldName="FIELD";
// Reduc=new ReducType;
#ifdef __BCplusplus__
   char filename[40]= "\\fasset\\fieldlist.txt";
#else
	char filename[40]= "fieldlist.txt";
#endif

   f.open(filename,ios::out);
   strcpy(Name,"cropRotation");
   Index=0;
}

/****************************************************************************\
Destructor.
\****************************************************************************/
cropRotation::~cropRotation()
{
	fieldList.Reset();
   for (int i=0;i<NumOfCrops();i++)
   {
		if (CropArray[i].CP != &staticBase)
			delete CropArray[i].CP;
		delete []  CropArray[i].CrpNme;
   }
   f.close();
}

/****************************************************************************\
\****************************************************************************/
double cropRotation::GetTotalPremium()
{
   double premium=0.0;
  	char * id;
   int NF=NumOfFields();
	for (int i=0;i<NF;i++)
   {
		linkList<field>::PS P;
	  	fieldList.SearchByNumber(P,i);
      if (P->element->GetActive())
      {
      	P->element->PresentCrop_(id);
    		SetCropPointer(id);
      	premium+=theLegislation->GetContribution(CropPointer->GetCropName())*P->element->FieldArea_();
		}
   }
   return premium;
}
/****************************************************************************\
\****************************************************************************/
double cropRotation::GetDiversePlant()
{
   double diverse = 0.0;
   double costsPrHa = 0.0;
  	char * id;
   int NF=NumOfFields();
	for (int i=0;i<NF;i++)
   {
		linkList<field>::PS P;
	  	fieldList.SearchByNumber(P,i);
      if (P->element->GetActive())
      {
	      P->element->PresentCrop_(id);
	    	SetCropPointer(id);
	      if (strcmp(CropPointer->GetCropName(),"FLLW")==0)
	      	costsPrHa = 0.0;
	      else
	      {
	         if (P->element->JBno()<=3)
	         	costsPrHa = 300.0;
	         else
	         	costsPrHa = 400.0;
	      }
	      diverse+=costsPrHa*P->element->FieldArea_();
      }
	}
   return diverse;
}
/****************************************************************************\
\****************************************************************************/
double cropRotation::GetTotalArea()
{
   double area=0.0;
  	int NF=NumOfFields();
	for (int i=0;i<NF;i++)
   {
		linkList<field>::PS P;
	  	fieldList.SearchByNumber(P,i);
      area+=P->element->FieldArea_();
	}
   return area;
}

/****************************************************************************\
\****************************************************************************/
double cropRotation::GetValueofLand()
{
   double value=0.0;
  	int NF=NumOfFields();
	for (int i=0;i<NF;i++)
   {
		linkList<field>::PS P;
	  	fieldList.SearchByNumber(P,i);
      int JB=P->element->JBno();
      double haPrice=30000;
      if (JB<4)
         haPrice=20000;
      value+=haPrice*P->element->FieldArea_();
	}
   return value;
}

/****************************************************************************\
Outputs a dot to the screen.
\****************************************************************************/
void cropRotation::Dot()
{
 cout << ".";
}

/****************************************************************************\
Updates the field object pointed to with currrent date and climate data.
Parameters:
   PP   Pointer to field object
\****************************************************************************/
void cropRotation::UpdateFld(field * PP)
{
	field * fP=NULL;
	PP->SplitMe(fP);
	PP->UpdateField();
   while (fP!=NULL)
   {
      ReplaceField(fP);
		PP->SplitMe(fP);
	}
}

/****************************************************************************\
Inserts a field, whith no area. This hack (!!!) is made only to "reserve" the
field number for later, which ensures that the order in which crops occupy fields
does not depend on crop development (climate).
\****************************************************************************/
void cropRotation::InsertGhostField()
{
   field * f=new field("field",ths->fieldList.NumOfNodes(),this);
   f->SetActive(false);
   ths->fieldList.InsertLast(f);
}

/****************************************************************************\
Replaces a "ghost" field with a real field. See "InsertGhostField()" for
rationale.
\****************************************************************************/
void cropRotation::ReplaceField(field * fP)
{
   int num=fP->ReturnNumber();
	linkList<field>::PS P;
 	fieldList.SearchByNumber(P,num);
   if (P->element->GetActive())
      theMessage->FatalError("cropRotation::ReplaceField - attempt to replace active field");
   delete(P->element);
   P->element=fP;
}

/****************************************************************************\
Creates cropping plan.
Parameters
   PP   object with specifications
\****************************************************************************/
void cropRotation::MakeSequenceInstance(fieldOrder * PP)
{
   cout << "cropRotation::MakeSequenceInstance - processing field no: " << PP->GetFieldNo() << endl;
   ths->CorrectCropId(PP); // Hack to get movin' in FarmN !!!!!!!!!!!!!
	ths->SetCropPointer(PP->GetCropId());
	int num;

   num = GlobalP->element->ReturnNumber();
   int futureNum=num;
   bool allreadyTaken;
   allreadyTaken=GlobalP->element->FuturePlanExists();
   if (allreadyTaken)
   {
      futureNum=ths->NumOfFields();
      ths->InsertGhostField();
      theMessage->WarningWithDisplay("cropRotation::MakeSequenceInstance - 'Ghost' field inserted");
   }
	cloneList<fieldOperationFields> * OP=
		ths->CropPointer->MakeOpList(PP,soil_type_,ths->YieldReduction(num,PP->GetCropId()));
   if (PP->GetAdjustArea()>0)
      ths->CropPointer->SetAdjustArea(OP,PP);
	if (!ths->IsCropLegal(num,PP->GetCropId()))
   {
   	string outstring1="cropRotation::MakeSequenceInstance - crop ";
      string outstring2=" is illegal crop on field ";
      outstring1=outstring1+PP->GetCropId()+outstring2;
		theMessage->Warning(outstring1,PP->GetFieldNo());
   }
	GlobalP->element->PlanField_(OP,PP->GetCropId(),futureNum);
   ths->AddAreaToIndicators(PP->GetCropId(),PP->GetArea(),soil_type_);
}

/****************************************************************************\
\****************************************************************************/
void cropRotation::AddAreaToIndicators(char * cid, double a, int JBNO)
{
   if (JBNO<=4)
   	theOutput->AddIndicator(economicIndicator,"01.05 JB 1-4","ha",a);
   else
      theOutput->AddIndicator(economicIndicator,"01.06 JB 5-7","ha",a);


	if (strcmp(cid,"W1")==0 || strcmp(cid,"W2")==0)
      theOutput->AddIndicator(economicIndicator,"15.10 W.wheat area","ha",a);
	if (strcmp(cid,"W5")==0 || strcmp(cid,"W6")==0)
      theOutput->AddIndicator(economicIndicator,"15.11 S.wheat area","ha",a);
   if (strcmp(cid,"RW")==0)
      theOutput->AddIndicator(economicIndicator,"15.12 W.rape area","ha",a);
   if (strcmp(cid,"RS")==0)
      theOutput->AddIndicator(economicIndicator,"15.13 S.rape area","ha",a);
   if (strcmp(cid,"B5")==0 || strcmp(cid,"B6")==0)
      theOutput->AddIndicator(economicIndicator,"15.14 W.barley area","ha",a);
   if (strcmp(cid,"B1")==0 || strcmp(cid,"B2")==0)
      theOutput->AddIndicator(economicIndicator,"15.15 S.barley area","ha",a);
   if (strcmp(cid,"RY")==0)
      theOutput->AddIndicator(economicIndicator,"15.16 Rye area","ha",a);
   if (strcmp(cid,"OA")==0)
      theOutput->AddIndicator(economicIndicator,"15.17 Oat area","ha",a);
   if (strcmp(cid,"PE")==0)
      theOutput->AddIndicator(economicIndicator,"15.18 Pea area","ha",a);
   if (strcmp(cid,"PO")==0)
      theOutput->AddIndicator(economicIndicator,"15.19 Potato area","ha",a);
   if (strcmp(cid,"SG")==0)
      theOutput->AddIndicator(economicIndicator,"15.20 Grass for seed area","ha",a);
   if (strcmp(cid,"MA")==0)
      theOutput->AddIndicator(economicIndicator,"15.21 Maize area","ha",a);
   if (strcmp(cid,"BP")==0)
      theOutput->AddIndicator(economicIndicator,"15.23 Barley/Pea area","ha",a);
   if (strcmp(cid,"G1")==0 || strcmp(cid,"G2")==0 || strcmp(cid,"G3")==0)
      theOutput->AddIndicator(economicIndicator,"15.24 Grass ley area","ha",a);
   if (strcmp(cid,"C1")==0 || strcmp(cid,"C2")==0)
      theOutput->AddIndicator(economicIndicator,"15.25 Grass/clover ley area","ha",a);
   if (strcmp(cid,"F1")==0 || strcmp(cid,"F2")==0)
      theOutput->AddIndicator(economicIndicator,"15.26 Fallow area","ha",a);
   if (strcmp(cid,"BE")==0)
      theOutput->AddIndicator(economicIndicator,"15.22 Beet area","ha",a);
/*   if (strcmp(cid,"L2")==0)
      theOutput->AddIndicator(economicIndicator,"15.23 Lucerne area","ha",a);  */
}

/****************************************************************************\
Sets the "CropPointer" to point at the crop given by "crop_id".
Parameters:
   crop_id   crop identification
\****************************************************************************/
void cropRotation::SetCropPointer(char * crop_id)
{
	int No=GiveCropNo(crop_id);
	if ((No>=0) && (No<=cattle_cropnum))
		CropPointer=CropArray[No].CP;
	else
   {
		CropPointer=&staticBase;
		theMessage->WarningWithDisplay("cropRotation::SetCropPointer - ",crop_id," is illegal cropname ");
	}
}

/****************************************************************************\
Reductions caused by soil born sickness, damage to soil structure etc.
In a later stage this might be modelled in a more accurate manner,
but basic knowledge about this issue is still missing.
Parameters:
   field_no   number of field in question
   crop_id    crop identification
\****************************************************************************/
double cropRotation::YieldReduction(int field_no,char * crop_id)
{
	char id[3];
	strcpy(id,"  ");
	int arr[BackScope+Maxperiods];
	id[0]=*crop_id;
	crop_id++;
	id[1]=*crop_id;
	arr[BackScope]=GiveCropNo(id);
	if ((arr[BackScope]<0)||(arr[BackScope]>=cattle_cropnum))
   {
		theMessage->WarningWithDisplay("cropRotation::YieldReduction - Illegal crop ",crop_id);
		return 0.0;
	}
	unsigned int k=0;
	while ((k*2<strlen(crop_id))&&(arr[BackScope+k]>=0))
   {
		k++;
		crop_id++;
		id[0]=*crop_id;
		crop_id++;
		id[1]=*crop_id;
		arr[BackScope+k]=GiveCropNo(id);
	}
	if (arr[BackScope+k]<0)
		k--;
	linkList<field>::PS P;
	fieldList.SearchByNumber(P,field_no);
	if (P!=NULL)
   {
		char * s;
		int i;
		for (i=0;i<BackScope;i++)
      {
			P->element->CropHistory(i,s);
			arr[i]=GiveCropNo(s);
		}

      return Reduc[arr[BackScope+k]][arr[BackScope+k-1]];
	}
	else
   {
		theMessage->WarningWithDisplay("cropRotation::YieldReduction - illegal fieldnumber");
		return 0.0;
	}
}

/****************************************************************************\
The update function which must be called on a daily basis.
\****************************************************************************/
void cropRotation::DailyUpdate()
{
	theOutput->AddIndicator(environmentalIndicator,"70.50 Precipitation","mm",theClimate->precip);
   ths=this;
   int n=fieldList.NumOfNodes();
   for (int i=0;i<n;i++)
   	UpdateFld(fieldList.ElementAtNumber(i));
}

/****************************************************************************\
Initiates the table containing information about the legal crop sequences.
Colums: Old crop(s). Rows: New crop.
Extend with Spring wheat W5 !!!
Not updatet properly !!!
\****************************************************************************/
void cropRotation::InitLegalTable()
{
	int Lgl[cattle_cropnum][cattle_cropnum] =
		//W1 W2 RW F1 B1 B2 F2 B5 B6 PE BP C1 C2 BE B8 G1 G2 G4 G5 W3 B7 B8 B9
		{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2},   // W1
		 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2},   // W2
		 {1, 1, 4, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 8, 0, 0, 0, 0, 0, 2, 1, 2, 2},   // RW
		 {0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 0, 2, 2},   // F1
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2},   // B1
		 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2},   // B2
		 {1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2},   // F2
		 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 1, 2, 2},   // B5
		 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 1, 2, 2},   // B6
		 {0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   // PE
		 {0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   // BP
		 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 0, 2, 2},   // C1
		 {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 2, 1, 2, 2},   // C2
		 {0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 1, 2, 2},   // BE
		 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2},   // B8
		 {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},   // G1
		 {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},   // G2
		 {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},   // G4
		 {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},   // G5
		 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 4, 4, 4, 4, 4, 5, 1, 5, 5},   // W3
		 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},   // B7
		 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 5, 1, 5, 5},   // B8
		 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 5, 1, 5, 5}};  // B9
	for (int i=0;i<cattle_cropnum;i++)
		for (int j=0;j<cattle_cropnum;j++)
			Legal[i][j]=Lgl[i][j];
}

/****************************************************************************\
A simple approach to the effects of the previous crop not related
to nutrient balance and release.
Colums: previous crop. Rows: New crop.
Extend with Spring wheat W5 !!!
Not updatet properly !!!
\****************************************************************************/
void cropRotation::InitReducTable()
{
	float Red[cattle_cropnum][cattle_cropnum] =
	//  W1   W2   RW   F1   B1   B2   F2   B5   B6   PE   BP   C1   C2   X2   G1   G2   G3   G4   G5   W3   B7   B8   B9
	{{0.90,0.90,1.00,1.00,0.90,0.90,1.00,0.90,0.90,1.00,1.00,0.90,0.90,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // W1
	 {0.90,0.90,1.00,1.00,0.90,0.90,1.00,0.90,0.90,1.00,1.00,0.90,0.90,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // W2
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // RW
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // F1
	 {0.95,0.95,1.00,1.00,0.95,0.95,1.00,0.95,0.95,1.00,1.00,0.95,0.95,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // B1
	 {0.95,0.95,1.00,1.00,0.95,0.95,1.00,0.95,0.95,1.00,1.00,0.95,0.95,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // B2
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // F2
	 {0.95,0.95,1.00,1.00,0.95,0.95,1.00,0.95,0.95,1.00,1.00,0.95,0.95,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // B5
	 {0.95,0.95,1.00,1.00,0.95,0.95,1.00,0.95,0.95,1.00,1.00,0.95,0.95,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // B6
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // PE
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // BP
    {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // C1
    {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // C2
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // X2
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // G1
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // G2
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // G3
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // G4
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // G5
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // W3
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // B7
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}, // B8
	 {1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00}};// B9
	for (int i=0;i<cattle_cropnum;i++)
		for (int j=0;j<cattle_cropnum;j++)
			Reduc[i][j]=Red[i][j];
}

/****************************************************************************\
Initiates all single fields with data which each instance of field reads
from the disk.
"min_area" contains the minimum area for one field.
Parameters:
   filename   input file for single fields initialization
   min_area   minimum area of a single field (ha)
\****************************************************************************/
void cropRotation::InitSingleFields(string filename,double min_area,int daysBetweenIrrigation, int irrigationDelay,int FingerFlow)
{
	OpenInputFileString(filename);
   UnsetCritical();
   int NumberOfFields=999;
   // Next parameter get is to allow debugging without fields present - is not critical
   GetParameter("NumberOfFields",&NumberOfFields);
   if (NumberOfFields>0)
   {
      int index,first;
      int num = 0;
      SetCritical();
      GetSectionNumbers("field",&first,&num);
      int singleField=theControlParameters->GetSingleField();
      if (singleField>=num)
			theMessage->FatalError("croprot::InitSingleFields - singleField requested for non-existant field");
      for(index=first;index<(first+num);index++)
      {
         if (singleField==index || singleField<0)
         {
            field * fld=new field("field",index,this);
            fld->SetFingerFlow(FingerFlow);
            fld->ReadParameters(file);
            fld->SetMinimumArea(min_area);
            fld->SetDaysBetweenIrrigation(daysBetweenIrrigation);
            fld->SetIrrigationDelay(irrigationDelay);
            fld->StartBudget();
            if (singleField>0)
               fld->SetIndex(0);
            fieldList.InsertLast(fld);
         }
      }
      theOutput->SetMaxErrorN(0.01*(1.0+num-first)); // Change later - add 0.002 per ha !!!
      theOutput->SetMaxErrorC(0.01*(1.0+num-first)); // Change later - add 0.002 per ha !!!
   }
   CloseInputFile();
}

/****************************************************************************\
The initialization function to be called prior to simulation.
Parameters:
convent           0->ecological farm; 1->conventional farm.
irrigat           0->irrigation not possible;  1->irrigation possible.
cattl             0->No cattle on farm; 1->cattle on farm.
MinArea           minimum area for one field (ha).
\****************************************************************************/
void cropRotation::Initialize(int convent,int irrigat,int daysBetweenIrrigation,int irrigationDelay,
		int FingerFlow,int cattl,double MinArea)
{
	conventional=convent;
	irrigate=irrigat;
	cattle=cattl;
   InitStaticCrops();
   InitSingleFields("fields.dat",MinArea,daysBetweenIrrigation,irrigationDelay,FingerFlow);
   thePeriods.Initialize("per_crpr.dat");
}

/****************************************************************************\
Returns the number corresponding to the crop name given by "crop_id".
Returns -1 by illegal "crop_id".
Parameters:
   crop_id   crop identification
\****************************************************************************/
int cropRotation::GiveCropNo(char * crop_id)
{
   char * s;
   int j = -1;
   for (int i=0;i<cattle_cropnum;i++)
   {
	   CropId(i,s);

	   if (strcmp(crop_id,s) == 0)
		   j = i;
   }
   return j;
}

/****************************************************************************\
Gives the name corresponding to the crop number given by "crop_no".
References an empty string by illegal "crop_no".
Parameters:
   crop_no   crop number
   crop_id   reference to crop identification
\****************************************************************************/
void cropRotation::CropId(int crop_no,char * &crop_id)
{
	if ((crop_no >= 0) && (crop_no < cattle_cropnum))
		crop_id=CropArray[crop_no].CrpNme;
	else
   {
		theMessage->WarningWithDisplay("cropRotation::CropId - Illegal cropnumber ",crop_id);

		strcpy(crop_id,"");
	}
}

/****************************************************************************\
Initializes all static crop objects
\****************************************************************************/
void cropRotation::InitStaticCrops()
{
   for (int i=0;i<cattle_cropnum;i++)
   {
	   CropArray[i].CrpNme=new char[10];
		CropArray[i].CP=&staticBase;
   }

   strcpy(CropArray[0].CrpNme,"W1");
	CropArray[0].CP=new staticWwheat;
	CropArray[0].CP->SetGatheringStraw(false);
	 strcpy(CropArray[1].CrpNme,"W2");
	CropArray[1].CP=new staticWwheat;
	CropArray[1].CP->SetGatheringStraw(true);
	 strcpy(CropArray[2].CrpNme,"RW");
	CropArray[2].CP=new staticWrape;
	strcpy(CropArray[3].CrpNme,"F1");
	CropArray[3].CP=new staticFallow;

	strcpy(CropArray[4].CrpNme,"B1");
	CropArray[4].CP=new staticSbarley();
   CropArray[4].CP->SetGatheringStraw(false);

	strcpy(CropArray[5].CrpNme,"B2");
	CropArray[5].CP=new staticSbarley();
   CropArray[5].CP->SetGatheringStraw(true);

	strcpy(CropArray[6].CrpNme,"F2");
	CropArray[6].CP=new staticFallowPermanent;

	strcpy(CropArray[7].CrpNme,"B5");
	CropArray[7].CP=new staticWbarley;
   CropArray[7].CP->SetGatheringStraw(false);

	strcpy(CropArray[8].CrpNme,"B6");
	CropArray[8].CP=new staticWbarley;
   CropArray[8].CP->SetGatheringStraw(true);

	strcpy(CropArray[9].CrpNme,"PE");
	CropArray[9].CP=new staticPea;

	strcpy(CropArray[10].CrpNme,"BP");
	CropArray[10].CP=new staticBarleyPea;
	strcpy(CropArray[11].CrpNme,"C1");
	CropArray[11].CP=new staticCloverGrass(false);
	strcpy(CropArray[12].CrpNme,"C2");
	CropArray[12].CP=new staticCloverGrass(true);

	strcpy(CropArray[13].CrpNme,"BE");
   CropArray[13].CP=new staticBeet;

	strcpy(CropArray[14].CrpNme,"B8");
   CropArray[14].CP=new staticSbarley();              // spring barley, undersown with grass and clover
   CropArray[14].CP->SetGatheringStraw(true);
   staticSbarley * sp = (staticSbarley*)CropArray[14].CP;
   sp->SetUnderSown(true);

   strcpy(CropArray[15].CrpNme,"G1");
	CropArray[15].CP=new staticGrass;

	strcpy(CropArray[16].CrpNme,"G2");
	CropArray[16].CP=new staticGrassEstablished;

	strcpy(CropArray[17].CrpNme,"G3");
	CropArray[17].CP=new staticGrassPermanent;

	strcpy(CropArray[18].CrpNme,"K1");
	CropArray[18].CP=new staticClover;
	strcpy(CropArray[19].CrpNme,"W5");
	CropArray[19].CP=new staticSwheat;
	strcpy(CropArray[20].CrpNme,"K2"); // established clover
   CropArray[20].CP=new staticGrassPermanent; 					//CropArray[17].CP; WHY??? (JB 1_5_02 converted to new .....)

	strcpy(CropArray[21].CrpNme,"W8");  // winter wheat with grass
	CropArray[21].CP=new staticWwheat;

   strcpy(CropArray[22].CrpNme,"MA");
	CropArray[22].CP=new staticMaize;


	strcpy(CropArray[23].CrpNme,"NO");
	CropArray[23].CP=new staticCrop;


	strcpy(CropArray[24].CrpNme,"B9");
   CropArray[24].CP=new staticSbarley();                 // spring barley, undersown with grass
   CropArray[24].CP->SetGatheringStraw(true);            // NOT IMPLEMENTED PROPERLY !!!!!!!!!!!!!!!
   staticSbarley * sp1 = (staticSbarley*)CropArray[24].CP;
   sp1->SetUnderSown(true);
   // Implement "O1" !!!

	strcpy(CropArray[25].CrpNme,"O2");
   CropArray[25].CP=new staticOat();                    // Oat
   CropArray[25].CP->SetGatheringStraw(true);            // NOT IMPLEMENTED PROPERLY ???!!!


	strcpy(CropArray[26].CrpNme,"RY");
   CropArray[26].CP=new staticRye();                    // Rye
   CropArray[26].CP->SetGatheringStraw(true);            // NOT IMPLEMENTED PROPERLY ???!!!


   strcpy(CropArray[27].CrpNme,"S1");
   CropArray[27].CP=new staticGrass();                  // grass for seed
   CropArray[27].CP->SetGatheringStraw(true);            // NOT IMPLEMENTED PROPERLY ???!!!


   strcpy(CropArray[28].CrpNme,"S2");
   CropArray[28].CP=new staticSeedGrass();              // grass for seed
   CropArray[28].CP->SetGatheringStraw(true);            // NOT IMPLEMENTED PROPERLY ???!!!


   strcpy(CropArray[29].CrpNme,"RS");
   CropArray[29].CP=new staticSrape();              // spring rape


   strcpy(CropArray[30].CrpNme,"PO");
   CropArray[30].CP=new staticPotato();              // potato


   strcpy(CropArray[31].CrpNme,"G4");
   CropArray[31].CP=new staticGrazingGrass();              // grass for seed
   CropArray[31].CP->SetGatheringStraw(true);            // NOT IMPLEMENTED PROPERLY ???!!!


	strcpy(CropArray[32].CrpNme,"IR");//Italian ryegrass - NJH May 2009
	CropArray[32].CP=new staticItalianRyeGrass;


	strcpy(CropArray[33].CrpNme,"SB");//Soybean NJH May 2009
	CropArray[33].CP=new staticPea;


	strcpy(CropArray[34].CrpNme,"SF");   //Sunflower NJH May 2009
	CropArray[34].CP=new staticMaize;

	strcpy(CropArray[35].CrpNme,"L1");
   CropArray[35].CP=new staticLucerneEstablished();              // lucerne/alfalfa */

   strcpy(CropArray[36].CrpNme,"L2");
   CropArray[36].CP=new staticLucerneEstablished();              // lucerne/alfalfa */
      strcpy(CropArray[37].CrpNme,"OR");
   CropArray[37].CP=new staticBeet();              // lucerne/alfalfa */

   /* !!!
	for (int i=27;i<cattle_cropnum;i++)   // Neccesary as some crops are undefinded at this stage
		CropArray[i].CP=CropArray[4].CP;
   */

	staticBase.InitCrop(conventional,irrigate); //???!!!@@@
	for (int i=0;i<NumOfCrops();i++)
		CropArray[i].CP->InitCrop(conventional,irrigate);
}

/****************************************************************************\
Returns the expected yield (tonnes/ha)
Parameters
   field_no   the number of the field
   crop_id    name of the crop
   N_level    nitrogen level
   PV_strat   the crop protection strategy

IMPORTANT: This function requires that "SetCropPointer" is called in advance
\****************************************************************************/
double cropRotation::ExpectedYield(int field_no,char * crop_id,int N_level,int PV_strat)
{
	linkList<field>::PS P;
	fieldList.SearchByNumber(P,field_no);
	if (P!=NULL)
   {
		int soil_type=P->element->JBno();
		double factor=YieldReduction(field_no,crop_id); // Possible reduction caused by crop rotation.
		return factor*CropPointer->ExpectedYield_(soil_type,N_level,PV_strat);
	}
	else
   {
		theMessage->WarningWithDisplay("cropRotation::ExpectedYield - Illegal fieldnumber");
		return 0.0;
	}
}

/****************************************************************************\
Returns the expected straw yield (tonnes/ha)
Parameters
   field_no   the number of the field
   crop_id    name of the crop
   N_level    nitrogen level
   PV_strat   the crop protection strategy

IMPORTANT: This function requires that "SetCropPointer" is called in advance
\****************************************************************************/
double cropRotation::ExpectedStrawYield(int field_no,char * crop_id,int N_level,int PV_strat)
{
	linkList<field>::PS P;
	fieldList.SearchByNumber(P,field_no);
	if (P!=NULL)
   {
		int soil_type=P->element->JBno();
		double factor=YieldReduction(field_no,crop_id); // Possible reduction caused by crop rotation.
		return factor*CropPointer->ExpectedStrawYield_(soil_type,N_level,PV_strat);
	}
	else
   {
		theMessage->WarningWithDisplay("cropRotation::ExpectedStrawYield - Illegal fieldnumber");
		return 0;
	}
}

/****************************************************************************\
Returns the number of field objects.
\****************************************************************************/
int cropRotation::NumOfFields()
{
	return fieldList.NumOfNodes();
}

/****************************************************************************\
Returns the number of protection strategies for the crop given by "crop_id".
Parameters:
   crop_id   crop identification
\****************************************************************************/
int cropRotation::NumOfProtectStrat(char * crop_id)
{
	SetCropPointer(crop_id);
	return CropPointer->NumOfProtectStrat_();
}

/****************************************************************************\
Returns the number of crops.
\****************************************************************************/
int cropRotation::NumOfCrops()
{

/* !!!
	if (cattle)
		return cattle_cropnum;
	else
		return nocattle_cropnum;
*/
	return cattle_cropnum;
}

/****************************************************************************\
Returns the number of nitrogen application levels for the crop in question
Parameters:
   crop_id   crop identification
\****************************************************************************/
int cropRotation::NitrogenLevels(char * crop_id)
{
	SetCropPointer(crop_id);
	return CropPointer->NitrogenLevels_();
}

/****************************************************************************\
Returns the distance (meter) to the field in question.
Parameters:
   field_no   number of field
\****************************************************************************/
double cropRotation::DistanceToField(int field_no)
{
	linkList<field>::PS P;
	fieldList.SearchByNumber(P,field_no);
	if (P!=NULL)
		return P->element->DistanceToField_();
	else
   {
		theMessage->WarningWithDisplay("cropRotation::DistanceToField - Illegal fieldnumber");
		return 0;
	}
}

/****************************************************************************\
Returns 1 if the field referenced by "field_no" is fenced,
otherwise returns 0.
Parameters:
   field_no   number of field
\****************************************************************************/
int cropRotation::IsFieldFenced(int field_no)
{
	linkList<field>::PS P;
	fieldList.SearchByNumber(P,field_no);
	if (P!=NULL)
		return P->element->IsFieldFenced_();
	else
   {
		theMessage->WarningWithDisplay("cropRotation::IsFieldFenced - Illegal fieldnumber");
		return 0;
	}
}

/****************************************************************************\
Returns the area (ha) of the field in question
Parameters:
   field_no   number of field
\****************************************************************************/
double cropRotation::FieldArea(int field_no)
{
	linkList<field>::PS P;
	fieldList.SearchByNumber(P,field_no);
	if (P!=NULL)
		return P->element->FieldArea_();
	else
   {
		theMessage->WarningWithDisplay("cropRotation::FieldArea - Illegal fieldnumber.");
		return 0;
	}
}

/****************************************************************************\
Returns the soil-type (JBNO 1-8) of the field in question
Parameters:
   field_no   number of field
\****************************************************************************/
int cropRotation::FieldJBNo(int field_no)
{
	linkList<field>::PS P;
	fieldList.SearchByNumber(P,field_no);
	if (P!=NULL)
		return P->element->JBno();
	else
   {
		theMessage->WarningWithDisplay("cropRotation::FieldSoilType - Illegal fieldnumber.");
		return 0;
	}
}

/****************************************************************************\
Returns 1 if the crop is a fallow crop, otherwise returns 0.
Parameters:
   crop_id   crop identification
\****************************************************************************/
int cropRotation::IsCropFallow(char * crop_id)
{
	SetCropPointer(crop_id);
	return CropPointer->FallowCrop();
}

/****************************************************************************\
Returns 1 if the crop is a grass crop, otherwise returns 0.
Parameters:
   crop_id   crop identification
Presently clumsy hardcoding
\****************************************************************************/
int cropRotation::IsCropGrass(int crop_num)
{
	return ((crop_num>=15) && (crop_num<=17));
}

/****************************************************************************\
Returns 1 if the crop or crop sequence is legal on the field, otherwise
returns 0.
The legality depends on the fields cropping histroy.
Parameters:
   field_no   number of field in question
   crop_id    crop identification
\****************************************************************************/
int cropRotation::IsCropLegal(int field_no,char * crop_id)
{
	int OK = 1;
	char id[3];
	strcpy(id,"  ");
	int arr[BackScope+Maxperiods];
	id[0]=*crop_id;
	crop_id++;
	id[1]=*crop_id;
	crop_id++;
	arr[BackScope]=GiveCropNo(id);
   SetCropPointer(id);
   /* BMP: useless information outside rigid GAMS setup - removed !!!
   if (CropPointer->SpeciesCanGiveUsableStraw())
      if (CropPointer->ImplementationDoesGiveStraw()!=sellAllStraw)
	 	   OK=0;
   */
	if ((arr[BackScope]<0)||(arr[BackScope]>=cattle_cropnum))
   {
		theMessage->WarningWithDisplay("cropRotation::IsCropLegal - Illegal crop number ",id);
		return 0;
	}
	int k=0;
	while ((crop_id[0]!='_')&&(strlen(crop_id)>0)&&(arr[BackScope+k]>=0))
   {
		k++;
		id[0]=*crop_id;
		crop_id++;
		id[1]=*crop_id;
		crop_id++;
		arr[BackScope+k]=GiveCropNo(id);
	   SetCropPointer(id);
      /* BMP: useless information outside rigid GAMS setup - removed !!!
      if (CropPointer->SpeciesCanGiveUsableStraw())
		   if (CropPointer->ImplementationDoesGiveStraw()!=sellAllStraw)
		  	   OK=0;
      */
	}
	if (arr[BackScope+k]<0)
		k--;
	linkList<field>::PS P;
	fieldList.SearchByNumber(P,field_no);
	if (P!=NULL)
   {
		char * s;
		int i;
		for (i=0;i<BackScope;i++)
      {
			P->element->CropHistory(i,s);
			arr[i]=GiveCropNo(s);
		}
		for (int l=BackScope;l<=(k+BackScope);l++)
			for (i=l-BackScope;i<l;i++)
				if (((l-i)<=Legal[arr[l]][arr[i]]) || (IsCropGrass(arr[l])&&IsCropGrass(arr[l-2])))
					OK = 0;
		return OK;
	}
	else
   {
		theMessage->WarningWithDisplay("cropRotation::IsCropLegal - Illegal fieldnumber.");
		return 0;
	}
}

/****************************************************************************\
Returns the amount of applied nitrogen (kg/ha)
Parameters:
   field_no   the fields number
   crop_id    crop identification
   N_level    nitrogen application level
\****************************************************************************/
double cropRotation::NitrogenAmount(int field_no,char * crop_id,int N_level,int yearOffset)
{
	double ND;
	linkList<field>::PS P;
	fieldList.SearchByNumber(P,field_no);
	if (P!=NULL)
   {
		char * pres_crop;
		int soil_type=P->element->JBno();
		P->element->PresentCrop_(pres_crop);
		double free_N=ExtraNRelease(pres_crop)+P->element->ExtraN();
      if (yearOffset>1) free_N = 0.0;                                     // should be improved by looking at present and past crop !!!
		SetCropPointer(crop_id);
		ND=CropPointer->NitrogenAmount(soil_type,free_N,N_level);
		return ND;
	}
	else
   {
		theMessage->WarningWithDisplay("cropRotation::NitrogenAmount - Illegal fieldnumber.");
		return 0;
	}
}

/****************************************************************************\
Returns the amount of applied phosphorus (kg/ha)
In a later version, the amount should depend on the state of the field.
Parameters:
 field_no   the fields number
 crop_id    crop identification
 N_level    nitrogen application level
\****************************************************************************/
double cropRotation::PhosphorAmount(char * crop_id)
{
	SetCropPointer(crop_id);
	return CropPointer->PhosphorAmount_();
}

/****************************************************************************\
Returns the amount of applied potassium (kg/ha)
In a later version, the amount should depend on the state of the field.
Parameters:
 field_no   the fields number
 crop_id    crop identification
 N_level    nitrogen application level
\****************************************************************************/
double cropRotation::PotassiumAmount(char * crop_id)
{
	SetCropPointer(crop_id);
	return CropPointer->PotassiumAmount_();
}

/****************************************************************************\
Expected EXTRA release of nitrogen (kg/ha) i the growth season following the
crop given by "crop_id".
Cereals by definition return zero.
Parameters:
   crop_id   crop identification
\****************************************************************************/
double cropRotation::ExtraNRelease(char * crop_id)
{
	SetCropPointer(crop_id);
	return CropPointer->ExtraNRelease_();
}

/****************************************************************************\
Create cropping plans for the comming season.
Parameters:
   field_no         field number
   FieldOrderList   chain of field operations.
\****************************************************************************/
void cropRotation::PlanField(int field_no,FieldOrderList &FOL)
{
	fieldList.SearchByNumber(GlobalP,field_no);
	if (GlobalP!=NULL)
   {
		soil_type_=GlobalP->element->JBno();
		field_no_=field_no;
		ths=this;
		FOL.ForEach(MakeSequenceInstance);
	}
	else
		theMessage->WarningWithDisplay("cropRotation::PlanField - Illegal fieldnumber");
}

/****************************************************************************\
Read plans for the comming season.
Parameters:
   filename   name of input file
\****************************************************************************/
void cropRotation::ReceivePlan(char * filename)
{
	if (fieldList.NumOfNodes()>0)
   {
      WriteFieldList();
      int old_field_no=-1;
      theMessage->LogEvent("------------ Reading field allocation plan ------------");
      int index;
      linkList<fieldOrder> * FT=new linkList<fieldOrder>;
      SetCritical();
      OpenInputFile(filename);
      cout << "----------------- Reading " << filename << endl; // REMOVE LATER !!!!!!!!!!!!!!!!!!
      int first,num;
      int singleField=theControlParameters->GetSingleField();
      GetSectionNumbers("fieldOrder",&first,&num);
      for(index=first;index<(first+num);index++)
      {
         if (singleField==index || singleField<0)
         {
            cout << "Reading field plan no: " << index << endl; // REMOVE LATER !!!!!!!!!!!!!!!
            //int fieldno=index;
            fieldOrder * FO = new fieldOrder("fieldOrder",index);
#ifdef MEASCOPE
            bool start=false;
				if (!strcmp(filename,"FIELDS.F00"))
            	start=true;
            FO->ReadParameters(file,start);
#else
            FO->ReadParameters(file);
#endif
            if (singleField>=0)
               FO->SetFieldNo(0);
            FO->SetStartYear(theTime.GetYear());
            FO->SetDistance(DistanceToField(FO->GetFieldNo()));
            if (old_field_no<0)
               old_field_no=FO->GetFieldNo();
            if (FO->GetFieldNo()!=old_field_no)
            {
               PlanField(old_field_no,*FT);
               FT->Reset();
               old_field_no=FO->GetFieldNo();
            }
            FT->InsertLast(FO);
         }
      }
      CloseInputFile();
      PlanField(old_field_no,*FT);
      delete FT;
   }
}

/****************************************************************************\
\****************************************************************************/
void cropRotation::GiveIndicators()
{
	for (int i=0;i<fieldList.NumOfNodes();i++)
   	fieldList.ElementAtNumber(i)->GiveIndicators();
}

/****************************************************************************\
\****************************************************************************/
void cropRotation::EndBudget()
{
	for (int i=0;i<fieldList.NumOfNodes();i++)
   	fieldList.ElementAtNumber(i)->EndBudget();
}

// --------------- Temporary functions below -------------------------------

/****************************************************************************\
\****************************************************************************/
void cropRotation::ShowAllCurrentPlans(char * filename)
{
	fstream fs;
	fs.open(filename,ios::out);
	linkList<field>::PS P;
	int n = fieldList.NumOfNodes();
	fieldList.PeekTail(P);
	for (int i=0;i<n;i++)
   {
		fs << "Field no " << i << endl;
		fieldList.SearchByNumber(P,i);
		P->element->ShowCurrentPlan(&fs);
	}
   fs.close();
}

/****************************************************************************\
\****************************************************************************/
void cropRotation::ShowAllFuturePlans(char * filename)
{
	fstream fs;
	fs.open(filename,ios::out);
	linkList<field>::PS P;
	int n = fieldList.NumOfNodes();
	fieldList.PeekTail(P);
	for (int i=0;i<n;i++)
   {
		fs << "Field no " << i << endl;
		fieldList.SearchByNumber(P,i);
		P->element->ShowFuturePlans(&fs);
	}
   fs.close();
}

/****************************************************************************\
\****************************************************************************/
void cropRotation::ShowH(field * PP)
{
	PP->ShowHistory();
}

/****************************************************************************\
\****************************************************************************/
void cropRotation::ShowHist()
{
	fieldList.ForEach(ShowH);
}

/****************************************************************************\
\****************************************************************************/
void cropRotation::WriteFieldList()
{
   linkList<field>::PS P;
 	int n = fieldList.NumOfNodes();
	for (int i=0;i<n;i++)
   {
		fieldList.SearchByNumber(P,i);
      char * c = new char[10];
      strcpy(c,"   ");
      P->element->PresentCrop_(c);
		f << P->element->ReturnNumber() << " " << c << endl;
		
	}
}

/****************************************************************************\
Replaces unknown crops with know ones.
Replacement specifications are read from a file.
A hack to get movin' in FarmN.
Parameters
   PP   object with specifications
\****************************************************************************/
void cropRotation::CorrectCropId(fieldOrder * PP)
{
   char * crop_id = PP->GetCropId();

	int crop_no=GiveCropNo(crop_id);

   if (crop_no < 0)
   {
      string s1,s2,crp;
      crp=crop_id;
      cout << "Crop not found " << crop_id<<" - searching in crop replacement file" << endl;
      fstream f;
      string inputDir=theSystemData->getInputDirectory();
#ifdef UNIX
      if(inputDir.substr(inputDir.size()-1,1).compare("/")!=0)
    		  {
				  inputDir=inputDir+ "/";
    		  }
#else
       if(!inputDir.substr(inputDir.size()-1,1).compare("\\"))
          		  {
      				  inputDir=inputDir+ "\\";
          		  }
#endif
      inputDir=inputDir+"replacements.txt";
      f.open(inputDir.c_str(),ios::in);
      bool found=false;
      if (!f)
         theMessage->FatalError("cropRotation::CorrectCropId - could not open crop replacement file");
      while (f)
      {
         f >> s1 >> s2;
         if (crp==s1)
         {
            crp=s2;
            found=true;
            break;
         }
      }
      if (found)
      {
         crop_id[0]=s2[0];
         crop_id[1]=s2[1];
         PP->SetCropId(crop_id); // Overfl�dig ???!!!
      }
      else
         theMessage->WarningWithDisplay("No replacement found for unknown crop ",crop_id);
      f.close();
   }
}

/****************************************************************************\
Returns the name of the product corresponding to a crop
Parameters:
   crop_num   number of crop
Presently clumsy hardcoding
\****************************************************************************/
const char * cropRotation::ProductName(int crop_num)
{
	if ((crop_num==0) || (crop_num==1))
		return "WHEAT";
	if (crop_num==2)
		return "RAPE";
	if ((crop_num==3) || (crop_num==6))
		return "FALLOW";
	if ((crop_num==4) || (crop_num==5) || (crop_num==10) || (crop_num==11)
       || (crop_num==20) || (crop_num==21))
		return "BARLEY";
   if ((crop_num==7) || (crop_num==8))
   	return "WNBRL";
   if (crop_num==9)
      return "PEA";
	if (crop_num==10)
		return "BARLEY/PEA";
   if ((crop_num==11) || (crop_num==12))
      return "CLOVERGRASS";
	return "";
}


