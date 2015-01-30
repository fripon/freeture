/*
								ManageFiles.cpp

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
* \file    ManageFiles.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    17/06/2014
* \brief   Create/Remove directories.
*/

#include "ManageFiles.h"

//http://stackoverflow.com/questions/8593608/how-can-i-copy-a-directory-using-boost-filesystem
bool ManageFiles::copyDirectory( path const & source, path const & destination){

    double t = (double)getTickCount();

    try{

        // Check whether the function call is valid
        if( !exists(source) || !is_directory(source) ){

            cerr << "Source directory " << source.string() << " does not exist or is not a directory." << '\n';
            return false;
        }

        if(exists(destination)){

            cerr << "Destination directory " << destination.string() << " already exists." << '\n';
            remove_all(destination);

           // return false;
        }

        // Create the destination directory
        if(!create_directory(destination)){
            cerr << "Unable to create destination directory" << destination.string() << '\n';
            return false;
        }

    }catch(filesystem_error const & e){

        cerr << e.what() << '\n';
        return false;
    }

    t = (((double)getTickCount() - t)/getTickFrequency())*1000;
    cout << "COPY TIME step 1: " <<std::setprecision(3)<< std::fixed<< t << " ms"<< endl;

    t = (double)getTickCount();

    // Iterate through the source directory
    for(directory_iterator file(source);file != directory_iterator(); ++file){

        try{
            path current(file->path());

            if(is_directory(current)){

                // Found directory: Recursion
                if(!copyDirectory(current,destination / current.filename())){
                    return false;
                }

            }else{

                //Check the limit of the number of frames to copy


                // Found file: Copy
                copy_file(current,destination / current.filename());
            }

        }catch(filesystem_error const & e){
            cerr << e.what() << '\n';
        }
    }

    t = (((double)getTickCount() - t)/getTickFrequency())*1000;
    cout << "COPY TIME step 2 : " <<std::setprecision(3)<< std::fixed<< t << " ms"<< endl;

    return true;
}

bool ManageFiles::removeDirectory(string const & source){

    try{

        path p(source);

        if(exists(p)){

            remove_all(p);
            return true;

        }else{

            cout << "can't delete the frameBuffer directory" << endl;
            return false;
        }

    }catch(filesystem_error const & e){

        cerr << e.what() << '\n';
        return false;

    }

}

bool ManageFiles::createDirectory( string const & source ){

    bool res = true;

    try{

        path p(source);

        if(exists(p)){

            cerr << "Destination directory " << p.string() << " already exists." << '\n';
            cout << "Remove and creation of a new one..." << endl;

            if(removeDirectory(source)){

                if(!create_directory(p)){
                    cerr << "Unable to create frameBuffer directory" << p.string() << '\n';
                    return false;
                }

                return true;

            }else{

                 return false;
            }

        }else{

            if(!create_directory(p)){
                cerr << "Unable to create frameBuffer directory" << p.string() << '\n';
                return false;
            }

            return true;

        }

    }catch(filesystem_error const & e){

        cerr << e.what() << '\n';
        return false;
    }

}

string ManageFiles::get_file_contents(const char *filename){

	ifstream in(filename, ios::in | ios::binary);

	if (in){

		string contents;
		in.seekg(0, ios::end);
		contents.resize(in.tellg());
		in.seekg(0, ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);

	}

	throw(errno);

}



