/*
				SMTPClient.h

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
*	Last modified:		26/11/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    SMTPClient.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/12/2014
* \brief   SMTP connection and send mails.
*/

#pragma once

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <ostream>

#ifdef WINDOWS
	#define WIN32_LEAN_AND_MEAN
	//#define _WIN32_WINNT = 0x0501
	#include <boost/asio.hpp>
	#include <windows.h>
#else
	#ifdef LINUX
	#include <boost/asio.hpp>
	#define BOOST_LOG_DYN_LINK 1
	#endif
#endif

#include <fstream>
#include <sstream>
#include <boost/archive/iterators/ostream_iterator.hpp>
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
#include <iterator>
#include <algorithm>
#include "Conversion.h"
#include "Base64.h"
#include <cerrno>

using namespace std;

class SMTPClient{

	private:

		static boost::log::sources::severity_logger< LogSeverityLevel > logger;

		static class _Init{

			public:
				_Init()
				{
					logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("SMTPClient"));
				}
		} _initializer;	

		string mailServerHostname;
		string mailSmtpServer;
		string mailUserName;
		string mailPassword;
		string mailFrom;
		vector<string> mailTo;
		vector<string> mailAttachments;
		string mailSubject;
		string mailMessage;
		unsigned int mailPort;
		bool imageInline;

		boost::asio::io_service io_service;
		boost::asio::ip::tcp::socket socket;

    public:

        SMTPClient(string smtpServer, unsigned int port, string hostname ):mailSmtpServer(smtpServer), mailPort(port), mailServerHostname(hostname), socket(io_service){};

		void getServerResponse(string request);
		bool checkSMTPAnswer(const std::string & responseWaited, boost::asio::ip::tcp::socket & socket);
		void write(string data, string expectedAnswer, bool checkAnswer, bool printCmd);
		void smtpServerConnection();
		string message();
        void send(string from, vector<string> to, string subject, string msg, vector<string> pathAttachments,bool imgInline);

	private:

		string get_file_contents(const char *filename);


};


