/****************************************************************************\
 $URL$
 $LastChangedDate$
 $LastChangedRevision$
 $LastChangedBy$
\****************************************************************************/
#include "../../base/common.h"
#include "fieldOperationFields.h"

/****************************************************************************\
\****************************************************************************/
fieldOperationFields::fieldOperationFields()
{
	fieldno  = 0;
	area     = 0.0;
	distance = 0.0;
   op_id    = operationNames(0); //Merged March 2009 was -1
   strcpy(crop_id,"XX");
	strcpy(cropName,"XX");
	opTime = new bsTime();

}

/****************************************************************************\
\****************************************************************************/
fieldOperationFields::fieldOperationFields(const fieldOperationFields& f)
{
	fieldno=f.fieldno;
	area=f.area;
	distance=f.distance;
  	op_id=f.op_id;
	strcpy(crop_id,f.crop_id);
	strcpy(cropName,f.cropName);
	opTime = new bsTime(*f.opTime);

}

/****************************************************************************\
\****************************************************************************/
fieldOperationFields::~fieldOperationFields()
{
	delete opTime;

}

/**
 * Define the operation. Should this be a constructor?
 */
void fieldOperationFields::DefineOp(operationNames op,
												string      crop,
												string      cropNm,
												int            yr,
												int            mon,
												int            d,
												double         ar,
												int            fieldn,
												double         dist)
{
	op_id=op;
	strcpy(crop_id,crop.c_str());
	strcpy(cropName,cropNm.c_str());
   opTime->SetTime(d,mon,yr);
	area=ar;
	fieldno=fieldn;
	distance=dist;
}

/****************************************************************************\
\****************************************************************************/
ostream &operator<<(ostream &os,const fieldOperationFields &aFieldop)
{
	os  << operationNameList[aFieldop.op_id];
	os << " Crop: " << aFieldop.crop_id << "  Size: " << aFieldop.area << " ha ";
   if (strcmp(aFieldop.crop_id,"XX")==0)
	   theMessage->FatalError("Unspecified crop in operation - serious fault!!!");


      os << aFieldop.opTime->GetDay() << "-" << aFieldop.opTime->GetMonth() << "-" << aFieldop.opTime->GetYear() << endl;

	return os;
}



/**
 * Write down the Operation, the area of the field and the time
 */
void fieldOperationFields::Output(ofstream * fs)
{
	*fs << operationNameList[op_id]
	    << " Crop: " << crop_id << "  Size: " << area << " ha "
       << theTime.GetDay() << "-" << theTime.GetMonth() << "-" << theTime.GetYear() << endl;
}