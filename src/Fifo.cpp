/*
								Fifo.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
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

//#include "stdafx.h"
#include "Fifo.h"
/*
template <class T>
fifo<T>::~fifo(void){

}

template <class T>
fifo<T>::fifo(){

	nbElements	=	0;
	front		=	NULL;
	back		=	NULL;

}

template <class T>
void fifo<T>::setFifoSize(int size){

	sizeQueue = size;

}

template <class T>
fifo<T>::fifo(int size){

	sizeQueue	=	size;
	nbElements	=	0;
	front		=	NULL;
	back		=	NULL;

//	boost::mutex Utils::outputMutex;

}

//access the fifo's element at a specific indicates index
template <class T>
T fifo<T>::getFifoElementAt(int index){

	// Acquire lock on the queue
	//boost::unique_lock<boost::mutex> lock(m_mutex);
	//m_mutex->lock();

	// When there is no data, wait till someone fills it.
	// Lock is automatically released in the wait and obtained
	// again after the wait
	//while (nbElements!=sizeQueue) m_cond.wait(lock);

		fifoNode *tempNode = new fifoNode;
		tempNode = front;

		for(int i=1; i<sizeQueue-index;i++){

			tempNode = tempNode->next;

		}

		//m_mutex->unlock();

		return tempNode->value;

}// Lock is automatically released here

// enqueue and dequeue automatically
template <typename  T>
void fifo<T>::pushInFifo(T img){

	// Acquire lock on the queue
	//boost::unique_lock<boost::mutex> lock(m_mutex);
	//m_mutex->lock();

	fifoNode *tempNode = new fifoNode;

	tempNode->value = img;

	//queue the first element
	if ( front == NULL ){
      front = tempNode;
      back = tempNode;
	  nbElements++;
    }
	//fifo doesn't fill
    else if( (front != NULL) && (nbElements != sizeQueue ) ){
      back->next = tempNode;
      back = tempNode;
	  nbElements++;
    }
	//fifo is fill
	else{
		//dequeue
		front=front->next;

		//queue new element
		back->next = tempNode;
		back = tempNode;

		// Notify others that data is ready
		//m_cond.notify_one();
	}
	//m_mutex->unlock();

}	// Lock is automatically released here

/*
void fifo::displayFrontOfFifo(){

	cout << "front : "<<front->value<<endl;

}

void fifo::displayFifo(){

	fifoNode *tempNode = new fifoNode;
	tempNode = front;
	int k = sizeQueue;

	cout << "BACK	[ ";

	do{

		for(int i=0; i<k;i++){

			if(i==k-1)
				cout <<" "<<tempNode->value<<" ";
			else
				tempNode = tempNode->next;
		}

		tempNode=front;

		k--;

	}while(k>0);

	cout << " ]	  FRONT"<<endl;

}
*//*
template <class T>
int fifo<T>::getNbElements(){

	return nbElements;

}

template <class T>
int fifo<T>::getSizeQueue(){

	return sizeQueue;

}*/
