/*
                                CfgLoader.h

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
*   Last modified:      20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CfgLoader.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Load parameters from a configuration file.
*/

#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include <stdlib.h>

using namespace std;

class CfgLoader{

    private :

        map<string,string> mData; // Container.

    public :

        /**
         * Constructor.
         *
         */
        CfgLoader(void);

        /**
         * Clear all values
         *
         */
        void Clear();

        /**
         * Load parameters name and value from configuration file.
         *
         * @param file Path of the configuration file.
         * @return Success status to load parameters.
         */
        bool Load(const string& file);

        /**
         * Check if value associated with given key exists.
         *
         * @param key Freeture's parameter.
         * @return Key has a value or not.
         */
        bool Contains(const string& key) const;

        /**
         * Get string value associated with given key
         *
         * @param key Freeture's parameter.
         * @param value Key's value.
         * @return Success to get value associated with given key.
         */
        bool Get(const string& key, string& value) const;

        /**
         * Get int value associated with given key
         *
         * @param key Freeture's parameter.
         * @param value Key's value.
         * @return Success to get value associated with given key.
         */
        bool Get(const string& key, int& value) const;

        /**
         * Get long value associated with given key
         *
         * @param key Freeture's parameter.
         * @param value Key's value.
         * @return Success to get value associated with given key.
         */
        bool Get(const string& key, long& value) const;

        /**
         * Get double value associated with given key
         *
         * @param key Freeture's parameter.
         * @param value Key's value.
         * @return Success to get value associated with given key.
         */
        bool Get(const string& key, double& value) const;

        /**
         * Get bool value associated with given key
         *
         * @param key Freeture's parameter.
         * @param value Key's value.
         * @return Success to get value associated with given key.
         */
        bool Get(const string& key, bool& value) const;

    private :

        /**
         * Remove spaces in configuration file's lines.
         *
         * @param str Configuration file's line.
         * @return String without space.
         */
        static string Trim(const string& str);

};

