/*
                                SMTPClient.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
*
*   License:        GNU General Public License
*
*   FreeTure is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*   FreeTure is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*   You should have received a copy of the GNU General Public License
*   along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*   Last modified:      26/11/2014
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
    #include <iphlpapi.h>
    #include <stdint.h>
#else
    #ifdef LINUX
        #include <boost/asio.hpp>
        #define BOOST_LOG_DYN_LINK 1
    #endif
#endif
#include "OpenSSL.h"
#include "Socket.h"
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
#include "ESmtpSecurity.h"

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

    public :

        /**
        * Send mail.
        *
        * @param server SMTP server name.
        * @param login Login to use if a secured connection to the SMTP server is required.
        * @param password Password to use if a secured connection to the SMTP server is required.
        * @param from Mail sender.
        * @param to Mail recipients.
        * @param subject Mail subject.
        * @param message Mail message.
        * @param pathAttachments Path of files to send.
        * @param imgInline
        * @param securityType Use secured connection or not.
        */
        static void sendMail(   string            server,
                                string            login,
                                string            password,
                                string            from,
                                vector<string>    to,
                                string            subject,
                                string            message,
                                vector<string>    pathAttachments,
                                SmtpSecurity      securityType);

    private :

        /**
        * Check SMTP answer.
        *
        * @param responseWaited
        * @param socket
        * @return Answer is correct or not.
        */
        static bool checkSMTPAnswer(const std::string & responseWaited, boost::asio::ip::tcp::socket & socket);

        /**
        * Send data to SMTP.
        *
        * @param data Data to send.
        * @param expectedAnswer
        * @param checkAnswer
        * @param printCmd
        */
        static void write(string data, string expectedAnswer, bool checkAnswer, boost::asio::ip::tcp::socket & socket);

        /**
        * Create MIME message.
        *
        * @return Final message to send.
        */
        static string buildMessage( string msg, vector<string> mMailAttachments,
             vector<string> mMailTo,  string mMailFrom,  string mMailSubject);

        /**
        * Get file content.
        *
        * @param filename
        * @return File's content.
        */
        static bool getFileContents(const char *filename, string &content);


        struct ReceiveFunctor{

            enum {codeLength = 3};
            const string code;

            ReceiveFunctor(int expectingCode) : code (to_string(expectingCode)){
                if(code.length() != codeLength) {
                    BOOST_LOG_SEV(logger,fail) << "SMTP code must be three-digits.";
                    throw "SMTP code must be three-digits.";
                    //throw runtime_error("SMTP code must be three-digits.");}
                }
            }

            bool operator()(const string &msg) const {

                if(msg.length() < codeLength) return false;
                if(code!=msg.substr(0,codeLength)) {
                    BOOST_LOG_SEV(logger,fail) << "SMTP code must be three-digits.";
                    throw "SMTP code must be three-digits.";
                    //throw runtime_error("SMTP code is not received");
                }

                const size_t posNewline = msg.find_first_of("\n", codeLength);
                if(posNewline == string::npos) return false;
                if(msg.at(codeLength ) == ' ') return true;
                if(msg.at(codeLength ) == '-') return this->operator()(msg.substr(posNewline + 1));
                throw "Unexpected return code received.";

            }
        };
};
