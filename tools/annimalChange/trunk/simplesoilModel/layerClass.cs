﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace simplesoilModel
{
    class layerClass
    {
            //! Depth below the soil surface of the lower boundary of the layer (millimetres)
      double z_lower;
    //! Water content of the layer at field capacity, mm/mm
      double fieldCapacity;
    //! Thickness of the layer (metres)
      double thickness;  
        //! Water below permanent wilting point, mm/mm
      double capacityAtPWP;
  /**
* get z lower
*/
      public double getz_lower()  { return z_lower;}
  /**
* get field Capacity
*/
      public double getfieldCapacity() { return fieldCapacity; }
  /**
* get thickness
*/
      public double getthickness() { return thickness; }
  /**
* set z lower
*/
      public void setz_lower(double az_lower) { z_lower = az_lower; }
  /**
* set field Capacity
*/
      public void setfieldCapacity(double afieldCapacity) { fieldCapacity = afieldCapacity; }
  /**
* set thickness
*/
      public void setthickness(double athickness) { thickness = athickness; }
  /**
* set PWP
*/
      public void setPWP(double aVal) { capacityAtPWP = aVal; }
  /**
* get PWP
*/
      public double getPWP() { return capacityAtPWP; }

  /**
* mm of water when below permanent wilting point
*/
      public double GetWaterBelowPWP() { return capacityAtPWP * thickness; }

  /**
* mm of plant-available water
*/
      public double GetPlantAvailableWater() { return (fieldCapacity - capacityAtPWP) * thickness; }

  /**
* Copy constructor
*/
      public layerClass(layerClass alayerClass)  
            {
             z_lower = alayerClass.z_lower;
             fieldCapacity = alayerClass.fieldCapacity;
             thickness = alayerClass.thickness;
            }

  /**
* constructor
*/
      public layerClass()
      {
      }
  /**
       * Initialise an instance of this class
       * Read the parameters associated with a single layer
       * param z_upper depth below the soil surface of the upper boundary of the layer
        */
      public void Initialise(double z_upper, double az_lower, double afieldCapacity, double aPWP)  
    {
        z_lower = az_lower;
        fieldCapacity = afieldCapacity/100.0;
        capacityAtPWP = aPWP / 100;
        thickness = z_lower - z_upper;
    }
  /**
* reload layer
*/
      public void reload_layer(layerClass original)
    {
        z_lower = original.z_lower;
        fieldCapacity = original.fieldCapacity;
        thickness = original.thickness;
        capacityAtPWP = original.capacityAtPWP;
    }
    }
}
