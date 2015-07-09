/*
                                Socket.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
*   Last modified:      03/03/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    Socket.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/03/2015
*/

#pragma once

#include "config.h"

#ifdef WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <boost/asio.hpp>
    #include <windows.h>
    #include <stdint.h>
    #include <openssl\err.h>
    #include <openssl\ssl.h>

#else
    #ifdef LINUX
        #include <boost/asio.hpp>
        #define BOOST_LOG_DYN_LINK 1
    #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
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
#include "Conversion.h"

using namespace std;

class Socket {

    boost::asio::io_service mIoService;
    boost::asio::ip::tcp::socket mSocket;

    public :

        /**
        * Constructor.
        *
        * @param sever SMTP server.
        * @param port Connection port.
        */
        Socket(string server, uint16_t port):mSocket(mIoService) {

            boost::asio::ip::tcp::resolver resolver(mIoService);

            boost::asio::ip::tcp::resolver::query query(server, to_string(port));

            boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

            boost::asio::connect(mSocket, endpoint_iterator);
        }

        /**
        * Get socket.
        *
        * @return Pointer on the socket.
        */
        boost::asio::ip::tcp::socket * GetSocket() {
            return &mSocket;
        }

};
