/*
								ImgThread.cpp

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
 * @file    ImgThread.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    13/06/2014
 */


//#include "stdafx.h"
#include "ImgThread.h"

ImgThread::ImgThread(string path,
					   int intervalCap,
					   double expTime,
					   bool imgCapGammaCorrEnable,
					   double imgCapGammaCorrValue,
					   Fifo<Frame>* sharedQueue,
					   int acqFormat,
					   boost::mutex *sharedQueueMutex,
					   boost::condition_variable *sharedQueueFill,
					   boost::condition_variable *sharedQueueNewElem){

	formatPixel				= acqFormat;
	thread					= NULL;						// image capture thread initialization
	imgPath					= path;						// Path where the image captured will be recorded
	framesQueue				= sharedQueue;				// shared queue of frames
	interval				= intervalCap;				// interval of images captured
	mutexQueue				= sharedQueueMutex;			// used to synchronise access to the shared queue
	condQueueFill			= sharedQueueFill;			// condition over the access to the shared queue (wait if not full)
	condNewFrame			= sharedQueueNewElem;
	mustStop				= false;					// used to stop the image capture 's thread
	exposureTime			= expTime;			        // number of images summed to create the image that will be record. Must be inferior to the shared queue size
	gammaCorrectionEnable	= imgCapGammaCorrEnable;	// enable gamma correction
	gammaCorrectionValue	= imgCapGammaCorrValue;		// define gamma correction value

}

ImgThread::~ImgThread(void){

	if (thread!=NULL) delete thread;

}

void ImgThread::startCapture(){

    BOOST_LOG_SEV(log, notification) << "Start image capture thread ";
    thread=new boost::thread(boost::ref(*this));

}

void ImgThread::join(){

	thread->join();

}

void ImgThread::stopCapture(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop=true;
	mustStopMutex.unlock();

    BOOST_LOG_SEV(log, notification) << "Wait 5 seconds for image cap thread finish...";

    while(thread->timed_join(boost::posix_time::seconds(5)) == false){

        BOOST_LOG_SEV(log, notification) << "Thread not stopped, interrupt it now";
        thread->interrupt();
        BOOST_LOG_SEV(log, notification) << "Interrupt Request sent";

    }

    BOOST_LOG_SEV(log, notification) << "Thread stopped";

}

//string ty =  type2str( floatMat.type() );
//cout<<"Matrix floatMat : "<<ty.c_str()<<" "<<floatMat.cols<<"x"<< floatMat.rows <<endl;
string type2str(int type) {

  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {

    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;

  }

  r += "C";
  r += (chans+'0');

  return r;

}

// Thread function
void ImgThread::operator () (){

	bool stop = false;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "imgThread");
	BOOST_LOG_SEV(log, critical) << "\n";
	BOOST_LOG_SEV(log,notification) << "Image Capture thread started.";

	int fps = 30;
	int totalImgToSummed = (int)(exposureTime*fps);
    int nbSummedImg;
	Mat resImg, img;

	do{

        try{

            // Put to sleep current thread
          /*  BOOST_LOG_SEV(log,notification) << "Thread is sleeping during : "<<interval*1000<<" ms";
            boost::this_thread::sleep(boost::posix_time::millisec(interval*1000));

            //auto start = boost::chrono::high_resolution_clock::now();

            boost::mutex::scoped_lock lock(*mutexQueue);

            while (!framesQueue->getFifoIsFull()) condQueueFill->wait(lock);

            resImg  = Mat::zeros(framesQueue->getFifoElementAt(0).getImg().rows, framesQueue->getFifoElementAt(0).getImg().cols, CV_32FC1);
            img     = Mat::zeros(framesQueue->getFifoElementAt(0).getImg().rows, framesQueue->getFifoElementAt(0).getImg().cols, CV_32FC1);
            nbSummedImg = 0;

            for(int i = 0; i<framesQueue->getSizeQueue(); i++){

                if(nbSummedImg<totalImgToSummed){

                    framesQueue->getFifoElementAt(i).getImg().convertTo(img, CV_32FC1);
                    accumulate(img,resImg);
                    nbSummedImg++;


                }else{

                    BOOST_LOG_SEV(log,notification) << "Sum process status : " << (nbSummedImg*100)/totalImgToSummed <<"%";
                    BOOST_LOG_SEV(log,notification) << "Break loop on frame's queue.";

                    break;

                }
            }

            while(nbSummedImg<totalImgToSummed){

                while (!framesQueue->getThreadReadStatus("imgCap")) condNewFrame->wait(lock);
                framesQueue->setThreadRead("imgCap",false);

                framesQueue->getFifoElementAt(0).getImg().convertTo(img, CV_32FC1);
                accumulate(img,resImg);
                nbSummedImg++;

            };

            BOOST_LOG_SEV(log,notification) << "Sum process status : " << (nbSummedImg*100)/totalImgToSummed <<"%";

            BOOST_LOG_SEV(log,notification) << "Terminate to sum frames";

            double minVal, maxVal;
            minMaxLoc(resImg, &minVal, &maxVal);

            //build histogram of summed mat
            Histogram hist(resImg, maxVal);


            string dateT = TimeDate::localDateTime(second_clock::universal_time(),"%Y%m%d_%H%M%S");

           // saveImageToJPEG(hist.drawHist(), "hist_"+dateT+"_"+intToString(totalImgToSummed));

            int newMax = hist.highLimitValue(99.975, resImg.cols * resImg.rows);

            Mat mat2jpeg;

            double alpha = 255.0 / (newMax - minVal);
            double beta  = -minVal * alpha;

            resImg.convertTo(mat2jpeg,CV_8UC1,alpha,beta);

            if(gammaCorrectionEnable){

                double inverse_gamma = 1.0 / gammaCorrectionValue;
                Mat lut_matrix;
                if(formatPixel == 8)
                    lut_matrix = Mat(1, 256, CV_8UC1,Scalar(0));
                else
                    lut_matrix = Mat(1, 256, CV_16UC1,Scalar(0));

                uchar * ptr = lut_matrix.ptr();
                for( int i = 0; i < 256; i++ )
                    ptr[i] = (int)( pow( (double) i / 255.0, inverse_gamma ) * 255.0 );

                Mat result;
                LUT( mat2jpeg, lut_matrix, result );

                //saveImageToJPEG(resTest, dateT+"_UT_gc_");

            }else{

               // saveImageToJPEG(mat2jpeg, dateT+"_UT_nogc_"+intToString(totalImgToSummed));

            }

            //auto end = boost::chrono::high_resolution_clock::now();

            //double timeTakenInSeconds = (end - start).count() * ((double) (boost::chrono::high_resolution_clock::period::num) / (boost::chrono::high_resolution_clock::period::den));
            //BOOST_LOG_SEV(log,notification) << "Stack size : " << stackSize << " Execution time : " << timeTakenInSeconds<<" s";
*/
            // Get the "must stop" state (thread-safe)
            mustStopMutex.lock();
            stop=mustStop;
            mustStopMutex.unlock();


		}catch(const boost::thread_interrupted&){

            cout<< "Image capture thread INTERRUPTED" <<endl;
            break;

        }

	}while (!stop);

	BOOST_LOG_SEV(log,notification) << "Thread terminated";

}
