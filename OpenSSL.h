/*
                                OpenSSL.h

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
*   Last modified:      20/07/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    OpenSSL.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    30/05/2015
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

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <ostream>
#include <fstream>
#include <sstream>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <iterator>
#include <algorithm>
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

using namespace std;

class OpenSSL {

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public:

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("OpenSSL"));

                }

        }initializer;

    public :

        unique_ptr< SSL_CTX, decltype(SSL_CTX_free)*> ctx_;
        unique_ptr< SSL, decltype( SSL_free )* > ssl_;
        enum {
            errorBufSize = 256,
            readBufSize = 256
        };

        /**
        * Constructor : Create SSL connection.
        *
        * @param socket Network connection.
        */
        OpenSSL(int socket);

        /**
        * Write data on the SSL connection.
        *
        * @param msg Data to write.
        */
        void Write(const string &msg);

        /**
        * Read data on the SSL connection.
        *
        * @param isDoneReceiving Struct to handle response.
        * @return Response.
        */
        template<typename IsDoneReceivingFunctorType> string Read(IsDoneReceivingFunctorType isDoneReceiving) {

            char buf[readBufSize];
            string read;

            while(true) {

                const int rstRead = SSL_read(ssl_.get(), buf, readBufSize);
                if(0==rstRead) {
                    BOOST_LOG_SEV(logger,fail) << "Connection lost while read.";
                    throw "Connection lost while read.";
                    //throw runtime_error("Connection lost while read.");
                }
                if(0>rstRead && SSL_ERROR_WANT_READ == SSL_get_error(ssl_.get(), rstRead))
                    continue;
                read+= string(buf, buf + rstRead);
                if(isDoneReceiving(read)) return read;
            }
        }

        /**
        * Destructor : Shutdown SSL connection.
        *
        */
        ~OpenSSL();

        /**
        * OpenSSL's library initialization.
        *
        */
        struct StaticInitialize {

            StaticInitialize() {

                ERR_load_crypto_strings();
                SSL_load_error_strings();
                SSL_library_init();
            }

            ~StaticInitialize() {
                ERR_free_strings();
            }
        };
};
