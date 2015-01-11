/*
				CameraSimu.h

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

/**
 * @file    CameraSimu.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    03/06/2014
 */

#pragma once

#include "includes.h"
#include "Camera.h"
#include "Fifo.h"
#include "Frame.h"
#include "EnumLog.h"
#include <boost/filesystem.hpp>
#include <iterator>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using boost::shared_ptr;

using namespace cv;
using namespace std;
using namespace logenum;

class CameraSimu : public Camera{

    public:
        CameraSimu(Fifo<Frame> *queue, boost::mutex *m_mutex_queue, boost::condition_variable *m_cond_queue_fill, boost::condition_variable *m_cond_queue_new_element);
        virtual ~CameraSimu();

        int			getWidth                                ();
		int			getHeight                               ();
		double      getFPS                                  ();

		int			grabStart								();
		void		grabStop								();
		void		join									();

		// Thread's acquisition function
		void		operator								()();
		void		stopThread();


    private:

        src::severity_logger< severity_level > log;			// logger

        Fifo<Frame> * framesQueue;

		bool mustStop;
		boost::mutex mustStopMutex;
		boost::mutex				*mutexQueue;
		boost::condition_variable	*condQueueFill;
		boost::condition_variable	*condQueueNewElement;
		//bool* grabState;

		boost::thread *m_thread;

		int camSizeH;
		int camSizeW;
		double camFPS;
		int Ax, Ay;
		int Bx, By;





};
