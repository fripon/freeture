/*
								Fifo.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014 Yoan Audureau -- FRIPON-GEOPS-UPSUD
*
*	License:		GNU General Public License
*
*	FreeTure is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*	FreeTure is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	You should have received a copy of the GNU General Public License
*	along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*	Last modified:		20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    Fifo.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    13/06/2014
 */

#pragma once

#include "includes.h"
#include "Frame.h"

using namespace std;
using namespace cv;

/******************************************************
	|*|
	|*|			   back			   front
	|*|				|*|	|*|	|*|	|*|	|*|	  Dequeue
	|*|	--------->	|*|	|*|	|*|	|*|	|*|	---------->	|*|
		 Enqueue	|*|	|*|	|*|	|*|	|*|				|*|
					|*|	|*|	|*|	|*|	|*|				|*|
													|*|
index :				 0   1	 2	 3	 4

*******************************************************/

template <class T> class Fifo {

	typedef struct fifoNode{
		struct fifoNode *next;
		T value;
	};

	private:
		int			sizeQueue;			// Static size of the fifo
		int			nbElements;			// Number of elements present in fifo
		bool		fifoIsFull;			// Indicates fifo is full
		bool		newElement;	// This flags indicates that a new element has been pushed on the fifo's head
		//int			nbThreadReading;	// Nombre de thread qui vont lire la frame la plus récente qui a été pushée
		//int			nbThreadRead;
		bool		fifoRenewedState;	// This flags indicates that the fifo has been completly renewed
		fifoNode	*front;				// Pointer on the fifo's head
		fifoNode	*back;				// Pointer on the fifo's back
		map <string, bool> threadRead;
	public:

		~Fifo(void){};

		Fifo(){

			nbElements	=	0;
			fifoIsFull	=	false;
			front		=	NULL;
			back		=	NULL;

		};

		Fifo(int size, bool imgCapEnable, bool imgAstroEnable, bool detectionEnable){

			if(imgCapEnable)
				threadRead["imgCap"] = false;
			if(imgAstroEnable)
				threadRead["astCap"] = false;
			if(detectionEnable)
				threadRead["det"] = false;

			//nbThreadRead = 0;
			sizeQueue	=	size;
			nbElements	=	0;
			front		=	NULL;
			back		=	NULL;
			fifoIsFull	=	false;

			newElement		= false;
			fifoRenewedState	= false;

		};

        Fifo(int size){

			sizeQueue	=	size;
			nbElements	=	0;
			front		=	NULL;
			back		=	NULL;
			fifoIsFull	=	false;

			newElement		= false;
			fifoRenewedState	= false;

		};

		// Access fifo's element at a specific index
		T getFifoElementAt(int index){

				fifoNode *tempNode = new fifoNode;
				tempNode = front;

				for(int i=1; i<sizeQueue-index;i++){

					tempNode = tempNode->next;

				}

				return tempNode->value;
		};

		// Enqueue data and dequeue automatically according to the fifo size
		void pushInFifo(const T& img){

			fifoNode *tempNode = new fifoNode;

			tempNode->value = img;

			if ( front == NULL ){

				front = tempNode;
				back = tempNode;
				nbElements++;

			}else if( (front != NULL) && (nbElements != sizeQueue ) ){

				back->next = tempNode;
				back = tempNode;
				nbElements++;

			}else{

				fifoIsFull = true;

				fifoNode *temp = front;

				if(temp!=NULL){

				    //dequeue
                    front=front->next;
                    //queue new element
                    back->next = tempNode;
                    back = tempNode;
                    delete temp;

				}

			}
		};

		void	setFifoSize				(int size)		{sizeQueue = size;};
		bool	getFifoIsFull			()				{return fifoIsFull;};
		int		getNbElements			()				{return nbElements;};
		int		getSizeQueue			()				{return sizeQueue;};
	/*	int		getNbThread				()				{return nbThreadReading;};
		void	setNbThread	     		(int nb)		{nbThreadReading = nb;};
		int		getNbThreadRead			()				{return nbThreadRead;};
		void	setNbThreadRead	     	(int nb)		{nbThreadRead += nb; if(nbThreadRead==nbThreadReading)newElement=false;};*/
		void	setNewElement		    (bool state)	{newElement = state;};
		bool	getNewElement		    ()				{return newElement;};
		void	setFifoRenewedState		(bool state)	{fifoRenewedState = state;};
		bool	getfifoRenewedState		()				{return fifoRenewedState;};

		void	setThreadRead (string threadName, bool status){

			if(threadRead.find(threadName) != threadRead.end()){

				threadRead.at(threadName) = status;

			}
		};

		bool	getThreadReadStatus (string threadName){

			if(threadRead.find(threadName) != threadRead.end()){

				return threadRead.at(threadName);

			}

			return false;

		};

		/*void	setnbRenewedElements	(int n)			{nbRenewedElements = n;};
		bool	getnbRenewedElements	()				{return nbRenewedElements;};*/

};
