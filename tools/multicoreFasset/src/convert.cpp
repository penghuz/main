/*
 * converters.cpp
 *
 *  Created on: Aug 21, 2009
 *      Author: JOVE
 */

#include "convert.h"
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
using namespace std;
convert::convert()
{

}

string convert::IntToString(int conv) {
#ifdef __BCplusplus__
	 char *myBuff;
  string strRetVal;

  // Create a new char array
  myBuff = new char[100];

  // Set it to empty
  memset(myBuff,'\0',100);

  // Convert to string
  itoa(conv,myBuff,10);

  // Copy the buffer into the string object
  strRetVal = myBuff;
  
  // Delete the buffer
  delete[] myBuff;

  return(strRetVal);
#else

     std::stringstream Num;

     std::string str;

     Num << conv;

     str = Num.str();
     return str;
#endif

}

double convert::StringToDouble(string conv) {

	return atof(conv.c_str());
}
int convert::StringToInt(string conv) {
  int intReturn;
  intReturn = atoi(conv.c_str());
  return	intReturn;
}
string convert::DoubleToChar(double conv) {
	char output[200];
	sprintf(output,"%f",conv);
  return	output;
}


string convert::EatSpaces(string s)
{
   int source=0;
   int dest=0;
   while (s[source]!='\0')
   {
      while (s[source]==' ')
         source++;
      s[dest]=s[source];
      dest++;
      source++;
   }
   s[dest]='\0';
   return s;
}
bool convert::StringToBool(string input)
{
	bool output;
	if(input.compare("0")==0)
		output=false;
	if(input.compare("FALSE")==0)
		output=false;
	if(input.compare("False")==0)
		output=false;
	if(input.compare("false")==0)
		output=false;
	if(input.compare("F")==0)
		output=false;
	if(input.compare("f")==0)
		output=false;
	if(input.compare("NO")==0)
		output=false;
	if(input.compare("No")==0)
		output=false;
	if(input.compare("no")==0)
		output=false;
	if(input.compare("N")==0)
		output=false;
	if(input.compare("n")==0)
		output=false;
	if(input.compare("1")==0)
		output=true;
	if(input.compare("TRUE")==0)
		output=true;
	if(input.compare("True")==0)
		output=true;
	if(input.compare("true")==0)
		output=true;
	if(input.compare("T")==0)
		output=true;
	if(input.compare("t")==0)
		output=true;
	if(input.compare("YES")==0)
		output=true;
	if(input.compare("Yes")==0)
		output=true;
	if(input.compare("yes")==0)
		output=true;
	if(input.compare("Y")==0)
		output=true;
	if(input.compare("y")==0)
		output=true;
   return output;
}

string convert::partOfAString(string input, int number)
{
	#ifndef __BCplusplus__
		return partOfAStringC(input, number);
	#endif
	#ifdef __BCplusplus__
		return partOfAStringBorland(input, number);
	#endif
}
#ifndef __BCplusplus__
string convert::partOfAStringC(string input, int number)
{
  stringstream totalSString( input );
  string nameOfItem;

   for(int i=0;i<number;i++)
	   totalSString >>nameOfItem;
   return nameOfItem;
}
#endif
#ifdef __BCplusplus__
string convert::partOfAStringBorland(string input, int number)
{
   char fin[200];
   strcpy(fin,input.c_str());
   istrstream ins(fin, strlen(fin));
   string text1;
   for(int i=0;i<number;i++)
  		ins>>text1;
   return text1;
}
#endif

