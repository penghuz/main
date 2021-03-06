
#ifndef WNWHT_H
#define WNWHT_H

#include "staticCrop.h"
#include "../../base/message.h"
#include "../../base/typer.h"

class staticWwheat:
    public staticCrop
{
    protected:
        enum { Num_of_pr_str = 3, Nitrogen_levels = 12 };

        double Nopt(int JB_NO);

        double Ymin(int JB_NO);

        double Ymax(int JB_NO);

    public:
        staticWwheat();

        virtual int SpeciesCanGiveUsableStraw() {
            return 1;
        }

        virtual double PVstratFactor(int PV_strat);

        virtual void SpecifiedPlan(fieldOrder * PP,
                                   int          JB_NO,
                                   string       crop_id);

        virtual cloneList<fieldOperationFields> * MakeOpList(fieldOrder * PP,
                int                                                       JB_NO);

        virtual double NitrogenAmount(int    soil_type,
                                      double free_N,
                                      int    N_level);

        virtual double ExpectedYield_(int soil_type,
                                      int N_level,
                                      int PV_strat);

        virtual double ExpectedStrawYield_(int JB_NO,
                                           int N_level,
                                           int PV_strat);

        virtual double NUseSlurrySpring() {
            return 0.55;
        }
};
#endif

