/****************************************************************************\
 $URL$
 $LastChangedDate$
 $LastChangedRevision$
 $LastChangedBy$
\****************************************************************************/
//!Utility link list class
/*!Utility link list class.  Each node contains a pointer to some class and to the next node in the list
 * */
#ifndef LINKLIST_H
   #define LINKLIST_H

#include "common.h"

template <class T>
class linkList
{
protected:
	//!Node contains a pointer to some class and to the next node in the list
	struct Snode
   {
		T * element;
		Snode * next;
	};
public:
	//!Define a pointer to a node
	typedef Snode * PS;
protected:
	//!First node in list
	PS Head,
	//!Last node in list
	Tail;
	//!Number of nodes in the list
	int numOfNodes;
	//!Make the first node
	void MakeFirstNode(PS &P,T * E);
public:
	//!Constructor
	linkList();
	//!Destructor
	~linkList();
	//!Returned a pointer to the first node
	void PeekHead(PS &P);
	//!Return a pointer to the last node
	void PeekTail(PS &P);
	//!Returned total number of nodes
	int NumOfNodes();
	//!Move one step in list
	void OneStep(PS &P);
	//!Remove this node and delete the instance of the class held
	void Remove(PS &R);
	//!Remove this node but do not delete the instance of the class held
	void Release(PS &R);
	//!Insert a new node before a specified node
	/*!
	 * \param S specified node
	 * \param P node created (I think)
	 * \param E  pointer to the class that should be added
	 * */
	void InsertBefore(PS &S,PS &P,T * E);
	//!Insert a new node after a specified node
	/*!
	 * \param S specified node
	 * \param P node created (I think)
	 * \param E  pointer to the class that should be added
	 * */
	void InsertAfter(PS &S,PS &P,T * E);
	//!Inserts a new node at the end of the list
	void InsertLast(T * E);
	//!Removal all nodes from the list, deleting all elements
	void Reset();
	//!Perform a given function on each element in the list
	/*!
	 * \param funcp address of the function to be performed
	 * */
	void ForEach(void (*funcp)(T*));
	//!uncertain
	double AddForEach(double (*funcp)(T*));
	//!uncertain
   void FirstThat(PS &FT,int (*funcp)(T*));
	//!uncertain
	void LastThat(PS &FT,int (*funcp)(T*));
	//!Return the location of a given node (I think)
	void SearchByNumber(PS &FT,int num);
	//! Return the address of the node with a given logical number in the list
	/*!
	 * \param num logical number of the node in the list
	 * */
   T* ElementAtNumber(int num);
   //!Delete all notes without deleting the elements
   void ForgetAll();
};

#ifdef __BCplusplus__
   #pragma option -Jgx
#endif

template<class T> linkList<T>::linkList()
{
	Head=nullptr;
	Tail=nullptr;
	numOfNodes=0;
}

template<class T> linkList<T>::~linkList()
{
	Reset();
}

template<class T> void linkList<T>::MakeFirstNode(PS &P,T * E)
{
	P=new Snode;
	P->next=nullptr;
	P->element=E;
	Head=P;
	Tail=P;
	numOfNodes++;
}

template<class T> void linkList<T>::PeekHead(PS &P)
{
	P=Head;
}

template<class T> void linkList<T>::PeekTail(PS &P)
{
	P=Tail;
}

template<class T> int linkList<T>::NumOfNodes()
{
	return numOfNodes;
}

template<class T> void linkList<T>::InsertAfter(PS &S,PS &P,T * E)
{
	if (Head==nullptr)
		MakeFirstNode(P,E);
	else
	{
      if (nullptr==S)
      {
         cout << "linkList<T>::InsertAfter - function called with nullptr, element inserted as last!" << endl;
         S=Tail;
      }
		P=new Snode;
		if (S==Tail)
			Tail=P;
		P->next=S->next;
		S->next=P;
		P->element=E;
		numOfNodes++;
	}
}


template<class T> void linkList<T>::InsertLast(T * E)
{
	PS S1,S2;
	PeekTail(S1);
	InsertAfter(S1,S2,E);
}

template<class T> void linkList<T>::OneStep(PS &P)
{
	if (Head==nullptr)
		P=nullptr;
	else
      if (P!=nullptr)
	   	P=P->next;
}

// Ved afslutning peger R p� elementet efter det slettede.
template<class T> void linkList<T>::Remove(PS &R)
{
	PS P=Head;
	if ((R==Head) && (Head!=nullptr))
   { // R peger p� "Head"
      if (R==Tail)
         Tail=nullptr;
		Head=R->next;
		delete R->element;
		delete R;
		numOfNodes--;
		R=Head;
	}
	else
   {
		while ((P!=nullptr) && (R!=P->next))
      	P=P->next;
 		if (P!=nullptr)
      {
         if (R==Tail)
            Tail=P;
			P->next=P->next->next;
			delete R->element;
			delete R;
			numOfNodes--;
			R=P->next;
		} // Ellers er knuden ikke fundet
	}
}

// Ved afslutning peger R p� elementet efter det flyttede.
template<class T> void linkList<T>::Release(PS &R)
{
	PS P=Head;
	if ((R==Head) && (Head!=nullptr))
   { // R peger p� "Head"
      if (R==Tail)
         Tail=nullptr;
		Head=R->next;
		delete R;
		numOfNodes--;
		R=Head;
	}
	else
   {
		while ((P!=nullptr) && (R!=P->next))
			P=P->next;
		if (P!=nullptr)
      {
         if (R==Tail)
            Tail=P;
			P->next=P->next->next;
			delete R;
			numOfNodes--;
			R=P->next;
		} // Ellers er knuden ikke fundet
	}
}

template<class T> void linkList<T>::InsertBefore(PS &S,PS &P,T * E)
{
	if (Head==nullptr)
		MakeFirstNode(P,E);
	else
   {
      if (nullptr==S)
      {
         cout << "linkList<T>::InsertBefore - function called with nullptr, element inserted as first!" << endl;
         S=Head;
      }
		if (Head==S)
      {
			P=new Snode;
			P->next=Head;
			Head=P;
			P->element=E;
			numOfNodes++;
		}
		else
      {
			PS PT=Head;
			while ((PT!=nullptr) && (S!=PT->next))
				PT=PT->next;
			if (PT!=nullptr)
         {
				P=new Snode;
				P->next=PT->next;
				PT->next=P;
				P->element=E;
				numOfNodes++;
			}
         else
            cout << "linkList<T>::InsertBefore - function called with invalid pointer, element not inserted!" << endl;
		}
   }
}


template<class T> void linkList<T>::Reset()
{
	PS P1;
	PS P=Head;
	while (P!=nullptr)
   {
		P1=P->next;
		delete P->element;
		delete P;
		P=P1;
	}
	Head=nullptr;
	Tail=nullptr;
	numOfNodes=0;
}

template<class T> void linkList<T>::ForEach(void (*funcp)(T*))
{
	PS P=Head;
	while (P!=nullptr)
   {
		funcp(P->element);
		P=P->next;
	}
}

template<class T> double linkList<T>::AddForEach(double (*funcp)(T*))
{
   double sum = 0.0;
	PS P=Head;
	while (P!=nullptr)
   {
		sum += funcp(P->element);
		P=P->next;
	}
   return sum;
}

// Returnerer nullptr n�r ikke fundet
template<class T> void linkList<T>::FirstThat(PS &FT,int (*funcp)(T*))
{
	PS P=Head;
	while ((P!=nullptr) && (!funcp(P->element)))
		P=P->next;
	FT=P;
}

// Returnerer nullptr n�r ikke fundet
template<class T> void linkList<T>::LastThat(PS &FT,int (*funcp)(T*))
{
	FT=nullptr;
	PS P=Head;
	while (P!=nullptr)
   {
		if (funcp(P->element))
			FT=P;
		P=P->next;
	}
}

// Returnerer nullptr n�r ikke fundet.
// F�rste element har nummer 0.
template<class T> void linkList<T>::SearchByNumber(PS &FT,int num)
{
	FT=Head;
	int i=0;
	while ((FT!=nullptr) && (i<num))
   {
		FT=FT->next;
		i++;
	}
}

template<class T> T* linkList<T>::ElementAtNumber(int num)
{
	PS FT=Head;
	int i=0;
	while ((FT!=nullptr) && (i<num))
   {
		FT=FT->next;
		i++;
	}
   if (FT==nullptr)
   	return nullptr;
   else
	   return FT->element;
}

template<class T> void linkList<T>::ForgetAll()
{
	PS P1;
	PS P=Head;
	while (P!=nullptr)
   {
		P1=P->next;
		delete P;
		P=P1;
	}
	Head=nullptr;
	Tail=nullptr;
	numOfNodes=0;
}

#ifdef __BCplusplus__

      #pragma option -Jg

#endif

#endif

