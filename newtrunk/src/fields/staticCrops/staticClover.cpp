
#include "../../base/common.h"
#include "staticClover.h"

staticClover::staticClover() {
    cropName = "CLOVER";
}

void staticClover::SpecifiedPlan(fieldOrder * PP,
                                 int          JB_NO,
                                 string       crop_id) {
    staticCrop::SpecifiedPlan(PP, JB_NO, crop_id);
}

cloneList<fieldOperationFields> * staticClover::MakeOpList(fieldOrder * PP,
        int                                                             JB_NO) {
    aPlantItem1 = new plantItem;

    aPlantItem1 -> Setname("CLOVER");

    string crop_id = "K1";    // TEMPORARY

    ResetOpList();

    if (!PP -> GetAutomatic()) {
        SpecifiedPlan(PP, JB_NO, crop_id);
    } else {    // Automatic plan
        {       // Ploughing
            soilTreatFields * FO = new soilTreatFields;

            FO -> DefineOp(Ploughing, crop_id, cropName, PP -> GetStartYear(), 9, 1, PP -> GetArea(), 0,
                           PP -> GetDistance());
            AddOpList(FO);
        }
        {    // Seedbed harrowing
            soilTreatFields * FO = new soilTreatFields;

            FO -> DefineOp(SeedbedHarrowing, crop_id, cropName, PP -> GetStartYear(), 9, 2, PP -> GetArea(), 0,
                           PP -> GetDistance());
            AddOpList(FO);
        }
        {                                                         // Sowing
            sowFields * FO    = new sowFields;
            seed *      aSeed = new seed;

            aSeed -> Setname("SEEDCLOVER");
            theProducts -> GiveProductInformation(aSeed);
            aSeed -> Setamount(aSeed -> GetStandardDensity());    // 25
            FO -> DefineSowFieldsOp(Sowing, crop_id, cropName, PP -> GetStartYear(), 9, 3, PP -> GetArea(), 0,
                                    PP -> GetDistance(), PP -> GetPVStrat(), 0, aSeed);
            AddOpList(FO);

            delete aSeed;
        }
    }

    delete aPlantItem1;

    return LP;
}

double staticClover::NitrogenAmount(int JB_NO,
        double                          free_N,
        int                             N_level) {
    return 0;
}

double staticClover::ExpectedYield_(int JB_NO,
        int                             N_level,
        int                             PV_strat) {
    double factor = 1;    // PVstratFactor(PV_strat);

    if (JB_NO <= 4)    // Sandy soil
    {
        if (irrigation) {
            return factor * 3.5;
        } else {
            return factor * 2.0;
        }
    }

    return factor * 4.5;    // FORKERTE V�RDIER RET!!!
}