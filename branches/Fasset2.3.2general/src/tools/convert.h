#include <stdlib.h>
#include <iostream>
#include <string>

#ifndef CONVERT_H_
#define CONVERT_H_

using namespace std;
class convert{
private:


public:
	convert();
	int StringToInt(string convert);
	string IntToString(int conv);
	string DoubleToChar(double conv);
	double StringToDouble(string conv);
	string EatSpaces(string s);
	bool StringToBool(string input);

	string GetUnitTypeIntToString(int aMainUnit);
	int GetUnitTypeStringToInt(string aMainUnit);

	int GetChemType(string aChemType);
	string partOfAString(string input, int number);

private:
	string partOfAStringBorland(string input, int number);
	string partOfAStringC(string input, int number);


};

#endif /* FILEACCESS_H_ */
