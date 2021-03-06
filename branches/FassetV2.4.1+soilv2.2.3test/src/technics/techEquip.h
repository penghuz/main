/****************************************************************************\
 $URL$
 $LastChangedDate$
 $LastChangedRevision$
 $LastChangedBy$
\****************************************************************************/
/****************************************************************************\
   Author   Lars Noboe Andersen & Poul Lassen
     Date
 Revision   Bj�rn Molt Petersen, May-June 1997
\****************************************************************************/

#ifndef __TECHEQP_H
   #define __TECHEQP_H

#include "../base/message.h"

#include "../base/base.h"
#include "../base/commonData.h"
/****************************************************************************\
 Class: techEquip
\****************************************************************************/

class techEquip : public base
{
   /* Attributes */
   protected:
      string techEquipName;
      double size;
      double carryCapacity;
      int age;
      double hoursPrimo;
      double hoursUltimo;

   /* Actions */
   private:
      techEquip(); // Dissable use of default constructor

   public:
      // Constructor with arguments
      techEquip(const char * aName, const int aIndex = -1, const base * aOwner = nullptr);
      // Copy Constructor
      techEquip(const techEquip& atechEquip);
      // Destructor
      virtual ~techEquip();
      // Set functions
      void SettechEquipName(string& atechEquipName) {techEquipName = atechEquipName;}
      void Setsize(double asize) {size = asize;}
      void SetCarryCapacity(double acarryCapacity) {carryCapacity = acarryCapacity;}
      void SethoursUltimo(double ahoursUltimo) {hoursUltimo = ahoursUltimo;}
      void AddhoursUltimo(double ahoursUltimo) {hoursUltimo += ahoursUltimo;}
      // Get functions
      string GettechEquipName(void) {return techEquipName;}
      double Getsize(void) {return size;}
      double GetCarryCapacity(void) {return carryCapacity;}


      // Other functions
      virtual void ReadParameters(commonData * &file);
//      void UpdateAge();
      void UpdateHours();
      virtual void GiveIndicators();
};

#endif
