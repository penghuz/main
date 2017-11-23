/*
 * commonData.cpp
 *
 *  Created on: 16/02/2010
 *      Author: beer
 */
#include "../tools/fileAccess.h"
#include "../tools/convert.h"
#include "commonData.h"
#include "settings.h"
#include "id.h"
convert conv;
#include <iostream>
#include <algorithm>
#include <vector>
#include <chrono>
using namespace std;
commonData::commonData()
{
	critical=true;
	sectionPlace=-1;
	sectionPlace2=-1;
	secnodaryFileInUse=false;
	outputWarning=true;
	sectionName="nothing";
}
commonData::commonData(const commonData& p)
{
	static int copy=0;
	copy++;
	sectionPlace=p.sectionPlace;
	conv=p.conv;

	sectionName=p.sectionName;

	fileNameInUse=p.fileNameInUse;
	secnodaryFileInUse=p.secnodaryFileInUse;
	sectionPlace2=p.sectionPlace2;
	fileNameInUse2=p.fileNameInUse2;
	critical=p.critical;
	outputWarning=p.outputWarning;




	itemsName= p.itemsName;
	for(int i=0;i<p.itemsValue.size();i++)
		{
			itemsValue.push_back(p.itemsValue[i]);

		}

	for(int i=0;i<p.used.size();i++)
		{
		used.push_back(p.used[i]);
		}
	for(int i=0;i<p.itemsName2.size();i++)
		{
		itemsName2.push_back(p.itemsName2[i]);
		}
	for(int i=0;i<p.itemsValue2.size();i++)
		{
		itemsValue2.push_back(p.itemsValue2[i]);
		}
	for(int i=0;i<p.used2.size();i++)
		{
		used2.push_back(p.used2[i]);
		}

}
commonData::~commonData()
{
	if(outputWarning==true)
	{
		int id=threadID->getID(std::this_thread::get_id());
	for(unsigned int i=0;i<used.size();i++)
	{
		if(used[i]==false)
		{
			if(itemsName[i].compare("NULL2")!=0)
			{
				if(itemsName[i].compare("NULL")!=0)
					if(itemsName[i].compare("[end]")!=0)
					{
						string output= itemsName[i]+" has not been used in file " +fileNameInUse+" at line "+conv.IntToString(i);
						theMessage[id]->Warning(output);
					}
			}
		}

	}
	for(unsigned int i=0;i<used2.size();i++)
		{
			if(used2[i]==false)
			{
				if(itemsName2[i].compare("NULL2")!=0)
				{
					if(itemsName2[i].compare("NULL")!=0)
						if(itemsName2[i].compare("[end]")!=0)
						{
							string output= itemsName2[i]+" has not been used in file " +fileNameInUse2+" at line "+conv.IntToString(i);
							theMessage[id]->Warning(output);
						}
				}
			}

		}
	}
}
bool commonData::readFile(string fileName)
{

	threadID->fileLocks.lock();
	int id=threadID->getID(std::this_thread::get_id());
	if(fileName.compare("system.dat")!=0)
	{

  	string input=globalSettings[id]->getInputDirectory();

  	fileAccess hd;
  	hd.changeDir(input);

	}
	fileNameInUse=fileName;
	while(!itemsName.empty())
	{
		itemsName.pop_back();
		itemsValue.pop_back();
		used.pop_back();
	}
	fileAccess file;

	bool returnValue= file.openFileWithoutExit(fileName);

	static bool firstTime=true;
	 static fstream filestr;
	if(firstTime==true)
	{
		filestr.open ("fileList.txt", fstream::out);
		firstTime=false;

	}


	if(returnValue==true)
		filestr<<fileName<<" is open with success"<<endl;
	else
		filestr<<fileName<<" failed to be opened"<<endl;
	filestr<<flush;
	if(returnValue==false&&critical==true)
	{
		string input=globalSettings[id]->getInputDirectory();

	  	fileAccess hd;
		cout<<hd.changeDir(input)<< "is change input dir"<<endl;
		bool returnValue= file.openFileWithoutExit(fileName);
		cout<<" cannot open file "<<fileName<<" at "<<file.getCurrentPath()<<" and "<<globalSettings[id]->getInputDirectory()<<endl;
		exit(99);
	}
   if(returnValue==true)
   {
	while(!file.endOfFile())
	{
		string lineRead=file.getLineFile();
		string nameOfItem;
		string Value;
#ifndef __BCplusplus__
		stringstream totalSString( lineRead);

				totalSString>>nameOfItem;
		#else
				 char fin[200];
				 strcpy(fin,items[i].c_str());
				 istrstream ins(fin, strlen(fin));
				 ins>>nameOfItem;
		      	#endif
#ifndef __BCplusplus__
		totalSString>>Value;
#else
		 char fin[200];
		 strcpy(fin,items[i].c_str());
		 istrstream ins(fin, strlen(fin));
		 ins>>nameOfItem>>Value;
	#endif
		itemsName.push_back(nameOfItem);
		itemsValue.push_back(Value);
		used.push_back(false);
	}

   }
   file.closeFile();
   threadID->fileLocks.unlock();
	return returnValue;
}
bool commonData::readSecondaryFile(string fileName)
{
	std::mutex locks;
	locks.lock();
	secnodaryFileInUse=true;
	fileNameInUse2=fileName;

	while(!itemsName2.empty())
	{

		itemsName2.pop_back();
		itemsValue2.pop_back();
		used2.pop_back();
	}
	fileAccess file;
	bool returnValue= file.openFileWithoutExit(fileName);
   if(returnValue==true)
   {
	while(!file.endOfFile())
	{
		string lineRead=file.getLineFile();
		string nameOfItem;
		string Value;
#ifndef __BCplusplus__
		stringstream totalSString( lineRead);

				totalSString>>nameOfItem;
		#else
				 char fin[200];
				 strcpy(fin,items[i].c_str());
				 istrstream ins(fin, strlen(fin));
				 ins>>nameOfItem;
		      	#endif
#ifndef __BCplusplus__
		totalSString>>Value;
#else
		 char fin[200];
		 strcpy(fin,items[i].c_str());
		 istrstream ins(fin, strlen(fin));
		 ins>>nameOfItem>>Value;
	#endif
		 itemsValue2.push_back(Value);
		itemsName2.push_back(nameOfItem);
		used2.push_back(false);
	}
	file.closeFile();
   }
	locks.unlock();
	return returnValue;
}
bool commonData::FindItem(string ItemName, double * output, int inVector)
{
	string out;
	bool findIt=FindItem(ItemName,&out,inVector);
	if(findIt==true)
	{
		*output= conv.StringToDouble(out);
		return true;
	}
	return false;
}
bool commonData::FindItem(string ItemName, int * output, int inVector)
{
	string out;
	bool findIt=FindItem(ItemName,&out,inVector);
	if(findIt==true)
	{

		*output= conv.StringToInt(out);
		return true;
	}
	return false;
}
bool commonData::FindItem(string ItemName, bool * output, int inVector)
{
	string out;
	bool findIt=FindItem(ItemName,&out,inVector);
	if(findIt==true)
	{
		*output= conv.StringToBool(out);
		return true;
	}
	return false;
}
bool commonData::FindItem(string ItemName,string * output, int inVector)
{


	if(inVector!=-1)
	{
		ItemName=ItemName+'('+conv.IntToString(inVector)+')';

	}
	if(secnodaryFileInUse==true)
	{
		for(unsigned int i=sectionPlace2+1;i<itemsName2.size();i++)
		{

			if(ItemName.compare(itemsName2[i])==0)
			{
					*output=itemsValue2[i];

				 	 used2[i]=true;
					return true;
			}
			if(itemsName2[i][0]=='['&&itemsName2[i][itemsName2[i].size()-1]==']')
			{
					break;
			}
		}
	}
	for(unsigned int i=sectionPlace+1;i<itemsName.size();i++)
	{




		if(ItemName.compare(itemsName[i])==0)
		{
			*output=itemsValue[i];
			 used[i]=true;
			return true;
		}
		if(itemsName[i][0]=='['&&itemsName[i][itemsName[i].size()-1]==']')
		{
				break;
		}

	}
	string outputForMessage="could not find "+ItemName + " in " +sectionName+" in file "+fileNameInUse;

	if(critical==true)
	{
		int id=threadID->getID(std::this_thread::get_id());
		for(int i;i<itemsName.size();i++)
		cout<<itemsName[i]<<endl;
		theMessage[id]->FatalError(outputForMessage);
	}

	return false;

}
bool commonData::FindSection(string aSectionName, int sectionNumber)
{


	sectionName=aSectionName;
	sectionPlace2=-1;
	sectionPlace=-1;
	if(sectionNumber!=-1)
	{
		aSectionName=aSectionName+'('+conv.IntToString(sectionNumber)+')';
	}
	aSectionName='['+aSectionName+']';

	if(secnodaryFileInUse==true)
	{
		
		for(unsigned int i=0;i<itemsName2.size();i++)
		{

			if(aSectionName.compare(itemsName2[i])==0)
			{

				sectionPlace2=i;
				used2[i]=true;

			}
		}

	}

	for(unsigned int i=0;i<itemsName.size();i++)
	{


		if(aSectionName.compare(itemsName[i])==0)
		{
		
			sectionPlace=i;
			used[i]=true;
			return true;

		}
	}


	if(critical==true)
		{
		int id=threadID->getID(std::this_thread::get_id());
		string output="could not find "+sectionName+" "+conv.IntToString(sectionNumber);
			theMessage[id]->FatalError(output);
		}



	return false;
}
bool commonData::FindVectorSize(string VectorName,int *small,int * large)
{
	*small=-1;
	*large=-2;
	bool found=false;
	int size=VectorName.size();
	for(unsigned int i=sectionPlace+1;i<itemsName.size();i++)
	{
		string itemName=itemsName[i];

		string itemNameShort=itemName.substr(0,size);
		if(itemNameShort.compare(VectorName)==0)
		{
			int tmp=conv.StringToInt(itemName.substr(size+1,itemName.size()-2));
			if(found==false)
			{
				*small=tmp;
				*large=tmp;
				found=true;
			}

			else
			{
				if(*small>tmp)
				{
					*small=tmp;
				}
				if(*large<tmp)
				{
				*large=tmp;
				}
			}


		}

		if(itemName[0]=='['&&itemName[itemName.size()-1]==']')
		{
			break;
		}
	}
	return false;
}
void commonData::setCritical(bool aCritical)
{
	critical=aCritical;
};
void commonData::print()
{
	static int tmp=0;
	tmp++;
	cout<<"at "<<tmp<<endl;
	for(int i=0;i<itemsName.size();i++)
	{
		cout<<itemsName[i]<<" print in common "<<tmp<<endl;
	}
}
int commonData::findSectionThatContains(string common,string item, string section)
{
	int small=9999;
	int large=0;
	getSectionsNumbers(section, &small, &large);

	string result="NULL";
	for(int i=small;i<=large;i++)
	{
		FindSection(section, i);
		FindItem(common,&result);

		if(item.compare(result)==0)
			return i;
	}
	if(critical==true)
	{
		int id=threadID->getID(std::this_thread::get_id());
		string output="could not find "+common + " in " +section;
		theMessage[id]->FatalError(output);
	}
	return -1;

}
bool  commonData::getSectionsNumbers(string sectionName, int * small, int * large)
{
	*small=-1;
	*large=-2;
	bool found=false;
	string forCompare="["+sectionName;
	int size=forCompare.size();

	for(unsigned int i=0;i<itemsName.size();i++)
	{

		if(itemsName[i].substr(0,size).compare(forCompare)==0)
		{
			int last=itemsName[i].find("]",size)-size;
			int tmp =conv.StringToInt(itemsName[i].substr(size+1,last-2));
			if(found==false)
			{
				*small=tmp;
				*large=tmp;
				found=true;
			}
			else
			{
				if(*large<tmp)
				{
					*large=tmp;
				}
				if(*small>tmp)
				{
					*small=tmp;
				}
			}


		}

	}
	if(found==false&&critical==true)
	{
		int id=threadID->getID(std::this_thread::get_id());
		theMessage[id]->FatalError("cannot find section "+ sectionName);
	}
	return found;
}

bool commonData::loadSection(string sectionName, int &sectionLevel, string nameOfItem)
{
	convert conv;
	int size=sectionName.size();
	if(nameOfItem.substr(1,size).compare(sectionName)==0)
	{
		string substring;
		int times=0;
		int multiplier=1;
		substring= nameOfItem.substr(size+2,1);
		int number=0;
		while(substring.compare(")"))
		{
			number=number*multiplier+conv.StringToInt(substring);
			multiplier=multiplier*10;
			times++;
			substring= nameOfItem.substr(size+2+times,1);

		}
		if(!(sectionLevel+1==number))
		{
			int id=threadID->getID(std::this_thread::get_id());
			string output=conv.IntToString(sectionLevel+1)+" is missing for "+sectionName;
			theMessage[id]->FatalError(output);
		}
		sectionLevel=number;
		return true;
	}
	return false;
}

//! Will try to find a section where name has the value of a Alias
  /*!
   * \param org contains base like PigHousing(0).Section(0).floor(0)
   * \param base should start with first section that should be include eg 0 for PigHousing
   * \param name of the given keyword
   * \param Alias is the value of that keyword
   * */

bool commonData::findSection(string org,int base, string name,string Alias, string * path, int * Index)
{

	convert conv;
	unsigned int first=org.find('(', base+1);


	if (org.find('(', base+1)==string::npos)
	{
		bool tmp=critical;
		critical=false;
		string test=org;
		int place=findSectionThatContains(name,Alias,test);
		*path=org;
		*Index=place;

		critical=tmp;
		return true;
	}
	string SectionInOld=org.substr(0,first);
	int endbrakst=org.find(')',first);
	int largest=0;
	int small=0;

	getSectionsNumbers(SectionInOld, &small, &largest);
	bool returnVal=false;
	for(int i=small;i<=largest;i++)
	{

		string Newstring=SectionInOld+"("+conv.IntToString(i)+org.substr(endbrakst,org.size());

		returnVal=findSection(Newstring,first+1, name, Alias, path, Index);

		return returnVal;

	}
	return returnVal;

}
