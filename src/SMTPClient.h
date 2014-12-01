/*
								SMTPClient.h

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
*	Last modified:		26/11/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    SMTPClient.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    26/11/2014
 */

#pragma once

#include "includes.h"


#include <boost/asio.hpp>
#include <fstream>
#include <sstream>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <iterator>
#include <algorithm>
#include "Conversion.h"
#include "Base64.h"
#include "ManageFiles.h"

using namespace std;


class SMTPClient{

	private:

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


};


