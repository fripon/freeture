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

#ifdef WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <boost/asio.hpp>
    #include <windows.h>
#else
    #ifdef LINUX
        #include <boost/asio.hpp>
        #define BOOST_LOG_DYN_LINK 1
    #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <ostream>
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

class SMTPClient {

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public:

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("SMTPClient"));

                }

        }initializer;

        string                          mMailServerHostname;
        string                          mMailSmtpServer;
        string                          mMailUserName;
        string                          mMailPassword;
        string                          mMailFrom;
        vector<string>                  mMailTo;
        vector<string>                  mMailAttachments;
        string                          mMailSubject;
        string                          mMailMessage;
        unsigned int                    mMailPort;
        bool                            mImageInline;
        boost::asio::io_service         mIo_service;
        boost::asio::ip::tcp::socket    mSocket;

    public :

        /**
        * Constructor.
        *
        * @param smtpServer
        * @param port
        * @param hostname
        */
        SMTPClient(string smtpServer, unsigned int port, string hostname ):mMailSmtpServer(smtpServer), mMailPort(port), mMailServerHostname(hostname), mSocket(mIo_service){};

        /**
        * Get server response of a request.
        *
        * @param request
        */
        void getServerResponse(string request);

        /**
        * Check SMTP answer.
        *
        * @param responseWaited
        * @param socket
        * @return Answer is correct or not.
        */
        bool checkSMTPAnswer(const std::string & responseWaited, boost::asio::ip::tcp::socket & socket);

        /**
        * Send data to SMTP.
        *
        * @param data Data to send.
        * @param expectedAnswer
        * @param checkAnswer
        * @param printCmd
        */
        void write(string data, string expectedAnswer, bool checkAnswer, bool printCmd);

        /**
        * Create message to send.
        *
        * @return Final composed message.
        */
        string message();

        /**
        * Create a SMTP connection.
        *
        */
        void smtpServerConnection();

        /**
        * Send mail.
        *
        * @param from
        * @param to Recipients.
        * @param subject
        * @param msg Message.
        * @param pathAttachments
        * @param imgInline
        */
        void send(string from, vector<string> to, string subject, string msg, vector<string> pathAttachments,bool imgInline);

    private :

        /**
        * Get file content.
        *
        * @param filename
        * @return File's content.
        */
        string getFileContents(const char *filename);

};


