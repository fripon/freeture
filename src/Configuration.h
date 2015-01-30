/*
				Configuration.h

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
* \file    Configuration.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Get FreeTure's parameters from a configuration file.
*/

#pragma once

#include "includes.h"

using namespace std;

class Configuration{

	public:

		// Clear all values.
		void Clear();

		// Load a configuration file.
		bool Load(const string& File);

		// Check if value associated with given key exists.
		bool Contains(const string& key) const;

		// Get value associated with given key.
		bool Get(const string& key, string& value) const;
		bool Get(const string& key, int&    value) const;
		bool Get(const string& key, long&   value) const;
		bool Get(const string& key, double& value) const;
		bool Get(const string& key, bool&   value) const;

        Configuration(void);

	private:

		// Container.
		map<string,string> data;

		// Remove leading and trailing tabs and spaces.
		static string Trim(const string& str);
};

