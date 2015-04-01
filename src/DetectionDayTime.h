/*
						DetectionDayTime.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
* \file    DetectionDayTime.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    16/03/2015
* \brief   Daytime detection method.
*/

#pragma once


#include "config.h"

#ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
#endif

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/tokenizer.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/core.hpp>
#include "ELogSeverityLevel.h"
#include "TimeDate.h"
#include "Fits2D.h"
#include "Fits.h"
#include "Frame.h"
#include "EStackMeth.h"
#include "ECamBitDepth.h"
#include "GlobalEvent.h"
#include "LocalEvent.h"
#include "Detection.h"
#include "EParser.h"
#include "SaveImg.h"
#include <vector>
#include <iterator>
#include <algorithm>

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

using namespace std;
using namespace cv;

class DetectionDayTime : public Detection{

	/*private :

		static boost::log::sources::severity_logger< LogSeverityLevel > logger;

		static class _Init{

			public:
				_Init()
				{
					logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("DetectionDayTime"));
				}
		} _initializer;

		bool initStatus;

	public:

		DetectionDayTime(){

            initStatus = false;

        };

		~DetectionDayTime(){}

		bool run(Frame &c, Frame &p){

		    return false;

        };

		int getNumFirstEventFrame(){return 0;};

		string getDateEvent(){return "";};

		int getNumLastEventFrame(){return 0;};

		void resetDetection(){};

		void saveDetectionInfos(string p){};

		bool initMethod(string cfg_path){

            Configuration cfg;
            cfg.Load(cfg_path);

		    return true;

        };*/


    private :

		static boost::log::sources::severity_logger< LogSeverityLevel > logger;

		static class _Init{

			public:
				_Init()
				{
					logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("DetectionDayTime"));
				}
		} _initializer;

		bool initStatus;

		CamBitDepth ACQ_BIT_DEPTH;

		bool DEBUG;

		vector<int> numFrame;
		int maxNumFrame;
		vector<float> stdev;
		float maxstdev;

		Mat grad_prev1;
		Mat grad_prev2;
		Mat diffgradprev;

	public:

		DetectionDayTime(){

            initStatus = false;
            maxNumFrame = 0;

        };

		~DetectionDayTime(){




		}

		bool run(Frame &c, Frame &p){

		    Mat curr, prev;
		    c.getImg().copyTo(curr);
		    p.getImg().copyTo(prev);
		    int n = c.getNumFrame();

		    double minVal, maxVal;
            minMaxLoc(curr, &minVal, &maxVal);
            cout << "minVal : " << minVal << endl;
            cout << "maxVal : " << maxVal << endl;

            /*Scalar imgMean;
            Scalar imgStddev;

            meanStdDev(curr, imgMean, imgStddev);
            cout << "mean : " << imgMean[0] << endl;
            cout << "stdev : " << imgStddev[0]/*5*(int)(cvRound(imgStddev.val[0])+1)*/ /*<< endl;*/


            cout << "DEBUG : " << DEBUG<< endl;


            if(DEBUG) SaveImg::saveBMP(curr, "/home/fripon/debug/curr/frame_" + Conversion::intToString(n));

            // Absdiff
            /*Mat absDiff = Mat(curr.rows, curr.cols, CV_8UC1, Scalar(0));
            absdiff(curr, prev, absDiff);
            if(DEBUG) SaveImg::saveBMP(absDiff, "/home/fripon/debug/absdiff/frame_" + Conversion::intToString(n));*/

            /*double tdiff = (double)getTickCount();
            Mat diff = Mat(curr.rows, curr.cols, CV_8UC1, Scalar(0));
            unsigned short * ptr_c;
            unsigned short * ptr_p;
            unsigned char * ptr_d;

            for(int i = 0; i < curr.rows; i++){

                ptr_c = curr.ptr<unsigned short>(i);
                ptr_p = prev.ptr<unsigned short>(i);
                ptr_d = diff.ptr<unsigned char>(i);

                for(int j = 0; j < curr.cols; j++){

                    int val = ptr_c[j]- ptr_p[j];

                    if(val > 3 * imgMean[0])
                        ptr_d[j] = 255;

                }
            }



            tdiff = (((double)getTickCount() - tdiff)/getTickFrequency())*1000;
            cout << "> Difference Time : " << tdiff << endl;*/

            Mat grad;

            int scale = 1;
            int delta = 0;
            int ddepth = CV_16S;

            GaussianBlur(curr, curr, Size(3,3),0,0,BORDER_DEFAULT);

            Mat grad_x, grad_y;

            Mat abs_grad_x, abs_grad_y;

            /// Gradient X
            Sobel( curr, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
            /// Gradient Y
            Sobel( curr, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );

            convertScaleAbs( grad_x, abs_grad_x );
            convertScaleAbs( grad_y, abs_grad_y );

            addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );


            if(DEBUG) SaveImg::saveBMP(grad, "/home/fripon/debug/grad/grad_"+Conversion::intToString(n));

            if(!grad_prev1.data){
                grad.copyTo(grad_prev1);
            }else if(!grad_prev2.data){
                grad.copyTo(grad_prev2);


                absdiff(grad, grad_prev1, diffgradprev);



            }else{
                Mat diffgrad;


                cout << "mat type : " <<Conversion::matTypeToString(grad.type())<<endl;
                absdiff(grad, grad_prev2, diffgrad);
                cout << "end abs diff" << endl;
                Mat mcm = Mat(grad.rows, grad.cols, CV_8UC1, Scalar(0));

                unsigned char * ptr_diffgrad;
                unsigned char * ptr_diffgradprev;
                unsigned char * ptr_mcm;

                for(int i = 0; i < diffgrad.rows; i++){

                    ptr_diffgrad = diffgrad.ptr<unsigned char>(i);
                    ptr_diffgradprev = diffgradprev.ptr<unsigned char>(i);
                    ptr_mcm = mcm.ptr<unsigned char>(i);

                    for(int j = 0; j < diffgrad.cols; j++){

                        if(ptr_diffgradprev[j] < ptr_diffgrad[j])
                            ptr_mcm[j] = ptr_diffgradprev[j];
                        else
                            ptr_mcm[j] = ptr_diffgrad[j];

                    }
                }


                if(DEBUG) SaveImg::saveBMP(mcm, "/home/fripon/debug/mcm/mcm_"+Conversion::intToString(n));


                grad.copyTo(grad_prev2);
                diffgrad.copyTo(diffgradprev);

                Mat mapThreshold = Mat(mcm.rows, mcm.cols, CV_8UC1, Scalar(0));
                threshold(mcm, mapThreshold, defineThreshold(mcm), 255, THRESH_BINARY);

                Scalar imgMean;
                Scalar imgStddev;

                meanStdDev(mcm, imgMean, imgStddev);

                stdev.push_back(5*(int)(cvRound(imgStddev.val[0])+1));
                numFrame.push_back(n);

                if(n> maxNumFrame)
                    maxNumFrame = n;

                if(imgStddev[0] > maxstdev)
                    maxstdev = imgStddev[0];

                    if(DEBUG) SaveImg::saveBMP(mapThreshold, "/home/fripon/debug/mapThreshold/mapThreshold_"+Conversion::intToString(n));

                }

            //Fits2D newFits("/home/fripon/data2/seq_17032015_10h_150e_300g_mono12/");


            //newFits.writeFits(c.getImg(), UC8, "frame_" +Conversion::intToString(c.getNumFrame()));

         //   newFits.writeFits(c.getImg(), S16, "frame_" +Conversion::intToString(c.getNumFrame()));

		    return false;


        };

        int defineThreshold(Mat i){

            Scalar imgMean;
            Scalar imgStddev;

            meanStdDev(i, imgMean, imgStddev);


            return 5*(int)(cvRound(imgStddev.val[0])+1);

        };

		int getNumFirstEventFrame(){return 0;};

		string getDateEvent(){return "";};

		int getNumLastEventFrame(){return 0;};

		void resetDetection(){};

		void saveDetectionInfos(string p){


		Point graphSize = Point(1280, 960);

            const string labelAxisX = "frame";
            const string labelAxisY = "stdev";

            Mat graph_stdev = Mat(graphSize.y, graphSize.x, CV_8UC3, Scalar(0,0,0));

            // axis x
            line(graph_stdev, Point(40, graphSize.y - 40), Point(graphSize.x - 40, graphSize.y - 40), Scalar(255,255,255), 2);
            putText(graph_stdev, labelAxisX, cvPoint(graphSize.x - 70,graphSize.y - 20),FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(255,255,255), 1, CV_AA);

            // axis y
            line(graph_stdev, Point(40, graphSize.y - 40), Point( 40, 40), Scalar(255,255,255), 2);
            putText(graph_stdev, labelAxisY, cvPoint(10,20),FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(255,255,255), 1, CV_AA);

            // origin
            putText(graph_stdev, "0", cvPoint(30, graphSize.y - 20),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255,255,255), 1, CV_AA);


            float maxH = maxstdev;

            int mawW = maxNumFrame;

            int nbDigitMaxH = Conversion::roundToUpperRange((int)maxH);
            int nbDigitMaxW = Conversion::roundToUpperRange((int)mawW);

            cout << "nbDigitMaxH : " << nbDigitMaxH<< endl;
            cout << "nbDigitMaxW : " << nbDigitMaxW<< endl;

            int sizeH = nbDigitMaxH / 5; //-> 20
            int sizeW = nbDigitMaxW / 5; // -> 200


            for(int i =1; i< 5; i++){

                // axe x
                line(graph_stdev, Point(((i * sizeW * 1200)/nbDigitMaxW) + 40, graphSize.y - 40 - 10), Point(((i * sizeW * 1200)/nbDigitMaxW)  + 40, graphSize.y - 40 + 10), Scalar(255,255,255), 1);
                putText(graph_stdev, Conversion::intToString(i * sizeW), Point(((i * sizeW * 1200)/nbDigitMaxW) + 40, graphSize.y - 40 +30),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255,255,255), 1, CV_AA);

                // axe y
                line(graph_stdev, Point(40-10, graphSize.y - 40 - ((i * sizeH * 880)/nbDigitMaxH) ), Point(40+10, graphSize.y - 40 - ((i * sizeH * 880)/nbDigitMaxH)), Scalar(255,255,255), 1);
                putText(graph_stdev, Conversion::intToString(i * sizeH), Point(1, graphSize.y - 40 - ((i * sizeH * 880)/nbDigitMaxH) ),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255,255,255), 1, CV_AA);

            }


            int s = numFrame.size();

            if(numFrame.size() == stdev.size()){

                Point p0 = Point((( numFrame.at(0) * 1200)/nbDigitMaxW) + 40, graphSize.y - 40 - ((stdev.at(0) * 880)/nbDigitMaxH));

                for(int i = 1; i< s; i++){

                    Point p1 = Point((( numFrame.at(i) * 1200)/nbDigitMaxW) + 40, graphSize.y - 40 - ((stdev.at(i) * 880)/nbDigitMaxH));
                    line(graph_stdev, p0, p1, Scalar(0,0,255), 1);

                    p0 = p1;

                }

            }






            if(DEBUG) SaveImg::saveBMP(graph_stdev, "/home/fripon/debug/graph_mean");

            cout << "graph saved" << endl;




		};

		bool initMethod(string cfg_path){

            Configuration cfg;
            cfg.Load(cfg_path);

            cfg.Get("DEBUG", DEBUG);

            if(DEBUG){

                // Delete curr directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/curr");

                    if(boost::filesystem::exists(path))
                        boost::filesystem::remove_all(path);

                }


                // Create curr directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/curr");
                    boost::filesystem::create_directories(path);
                }

                // Delete diff directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/diff");

                    if(boost::filesystem::exists(path))
                        boost::filesystem::remove_all(path);

                }

                // Create diff directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/diff");
                    boost::filesystem::create_directories(path);
                }

                // Delete absdiff directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/absdiff");

                    if(boost::filesystem::exists(path))
                        boost::filesystem::remove_all(path);

                }

                // Create absdiff directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/absdiff");
                    boost::filesystem::create_directories(path);
                }

                // Delete grad directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/grad");

                    if(boost::filesystem::exists(path))
                        boost::filesystem::remove_all(path);

                }

                // Create grad directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/grad");
                    boost::filesystem::create_directories(path);
                }

                // Delete mcm directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/mcm");

                    if(boost::filesystem::exists(path))
                        boost::filesystem::remove_all(path);

                }

                // Create mcm directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/mcm");
                    boost::filesystem::create_directories(path);
                }

                {
                    const boost::filesystem::path path("/home/fripon/debug/mapThreshold");

                    if(boost::filesystem::exists(path))
                        boost::filesystem::remove_all(path);

                }

                // Create mapThreshold directory
                {
                    const boost::filesystem::path path("/home/fripon/debug/mapThreshold");
                    boost::filesystem::create_directories(path);
                }



            }


		    return true;

        };
};

