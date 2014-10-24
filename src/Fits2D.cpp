/*
								Fits2D.cpp

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
*	Last modified:		22/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    Fits2D.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    22/10/2014
 */

#include "Fits2D.h"

Fits2D::Fits2D(){

}

Fits2D::~Fits2D(void){

}

/******************************************************/
/* Create a FITS primary array containing a 2-D image */
/******************************************************/
bool Fits2D::writeimage( Mat img, int bitDepth,string nb, bool dtANDstation){

    // Pointer to the FITS file, defined in fitsio.h
    fitsfile *fptr;
    int status, i, j;
    long  firstPixel, nbelements;

    int hImg = img.rows;
    int wImg = img.cols;

    TimeDate stringDate;

    string fitsCreationDate	= TimeDate::localDateTime(second_clock::universal_time(),"%Y-%m-%dT %H:%M:%S");
    string dateT1	= TimeDate::localDateTime(second_clock::universal_time(),"%Y%m%d_%H%M%S");
    string name		= kTELESCOP+"_"+dateT1+"UT"+"-"+nb;

    cout << name<<endl;
    cout << savedFitsPath<<endl;
    // string name		= "summ"+vTELESCOP+".fit";
    string pathAndname ="";

    if(dtANDstation){

        pathAndname		= savedFitsPath+name+".fit";

    }else{

        pathAndname		= savedFitsPath+".fit";

    }

    const char * filename;

    filename		= pathAndname.c_str();
    long naxis		= 2;					// 2-dimensional image
    long naxes[2]	= { wImg, hImg };

	firstPixel = 1;                        // First pixel to write
    nbelements = naxes[0] * naxes[1];	   // Number of pixels to write

    bool returnValue = true;

    switch(bitDepth){

        case 8 :
        {
                //https://www-n.oca.eu/pichon/Tableau_2D.pdf
                unsigned char ** tab = (unsigned char * *) malloc( hImg * sizeof( unsigned char * ) ) ;
                //float ** tab = (float* *) malloc( hImg * sizeof( float * ) ) ;

                if(tab == NULL){returnValue = false; break;}

                tab[0] = (unsigned char  *) malloc( naxes[0] * naxes[1] * sizeof(unsigned char) ) ;
                //tab[0] = (float  *) malloc( naxes[0] * naxes[1] * sizeof(float)) ;

                if(tab[0] == NULL){returnValue = false; break;}

                int a;
                for( a=1; a<naxes[1]; a++ )
                  tab[a] = tab[a-1] + naxes[0];

                // Delete old file if it already exists
                remove(filename);

                // Initialize status before calling fitsio routines
                status = 0;

                // Create new FITS file
                if (fits_create_file(&fptr, filename, &status))
                     printerror( status );
                     else
                     cout << "file created"<<endl;

                if ( fits_create_img(fptr,  BYTE_IMG, naxis, naxes, &status) )
                     printerror( status );

                // Initialize the values in the fits image with the mat's values
                 for (j = 0; j < naxes[1]; j++){
                     //uchar * matPtr = img.ptr<uchar>(j);
                     unsigned char * matPtr = img.ptr<unsigned char>(j);
                     //float * matPtr = img.ptr<float>(j);

                     for (i = 0; i < naxes[0]; i++){
                         //affect a value and inversed the image
                         tab[hImg-1-j][i] = (unsigned char)matPtr[i];
                         //tab[hImg-1-j][i] = (float)matPtr[i];
                    }
                }

                // write the array of unsigned short to the FITS file
                 if(fits_write_img(fptr, TBYTE, firstPixel, nbelements, tab[0], &status))
                    printerror( status );

                //free( *tab);
                 //free( *tab );
            free( tab[0]);    // free previously allocated memory

            break;
        }

        case 16 :
        {

            //https://www-n.oca.eu/pichon/Tableau_2D.pdf
            unsigned short ** tab = (unsigned short * *) malloc( hImg * sizeof( unsigned short * ) ) ;

            if(tab == NULL){returnValue = false; break;}

            tab[0] = (unsigned short  *) malloc( naxes[0] * naxes[1] * sizeof(unsigned short) ) ;

            if(tab[0] == NULL){returnValue = false; break;}

            int a;

            for( a=1; a<naxes[1]; a++ )
              tab[a] = tab[a-1] + naxes[0];

            // Delete old file if it already exists
            remove(filename);

            // Initialize status before calling fitsio routines
            status = 0;

            // Create new FITS file
            if (fits_create_file(&fptr, filename, &status))
                 printerror( status );

            if ( fits_create_img(fptr,  SHORT_IMG, naxis, naxes, &status) )
                 printerror( status );

            // Initialize the values in the fits image with the mat's values
            for (j = 0; j < naxes[1]; j++){

                 unsigned short * matPtr = img.ptr<unsigned short>(j);

                 for (i = 0; i < naxes[0]; i++){
                     //affect a value and inversed the image
                     tab[hImg-1-j][i] = (unsigned short)matPtr[i];
                }
            }

            // write the array of unsigned short to the FITS file
            if ( fits_write_img(fptr, TUSHORT, firstPixel, nbelements, tab[0], &status) )
                printerror( status );

            free( *tab);
            free( tab );  // free previously allocated memory

            break;

        }

        case 32 :
        {

            //https://www-n.oca.eu/pichon/Tableau_2D.pdf
            float ** tab = (float * *) malloc( hImg * sizeof( float * ) ) ;

            if(tab == NULL){returnValue = false; break;}

            tab[0] = (float *) malloc( naxes[0] * naxes[1] * sizeof(float) ) ;

            if(tab[0] == NULL){returnValue = false; break;}

            int a;

            for( a=1; a<naxes[1]; a++ )
              tab[a] = tab[a-1] + naxes[0];

            // Delete old file if it already exists
            remove(filename);

            // Initialize status before calling fitsio routines
            status = 0;

            // Create new FITS file
            if (fits_create_file(&fptr, filename, &status))
                 printerror( status );

            if ( fits_create_img(fptr,  FLOAT_IMG, naxis, naxes, &status) )
                 printerror( status );

            // Initialize the values in the fits image with the mat's values
            for (j = 0; j < naxes[1]; j++){

                 float * matPtr = img.ptr<float>(j);

                 for (i = 0; i < naxes[0]; i++){
                     //affect a value and inversed the image
                     tab[hImg-1-j][i] = (float)matPtr[i];
                 }
            }

            // write the array of unsigned short to the FITS file
           if ( fits_write_img(fptr, TFLOAT, firstPixel, nbelements, tab[0], &status) )
                printerror( status );


           // realloc(ptr, 0);
            free( *tab );
            free( tab);  // free previously allocated memory


            break;
        }

        default :

            returnValue = false;

            break;

    }


    if(returnValue){

        if(ffdkey(fptr, "COMMENT", &status))
           printerror( status );

        if(ffdkey(fptr, "COMMENT", &status))
           printerror( status );

        //FILENAME
        char * fn = new char[name.length()+1];
        strcpy(fn,name.c_str());

        if(fits_write_key(fptr,TSTRING,"FILENAME",fn,"",&status))
            printerror( status );

        delete fn;

        //DATE
        char * fdc = new char[fitsCreationDate.length()+1];
        strcpy(fdc,fitsCreationDate.c_str());

        if(fits_write_key(fptr,TSTRING,"DATE",fdc,"fits creation date",&status))
            printerror( status );

        delete fdc;

        //DATE-OBS
        char * d = new char[kDATEOBS.length()+1];
        strcpy(d,kDATEOBS.c_str());

        if(fits_write_key(fptr,TSTRING,"DATE-OBS",d,"UT date of Observation",&status))
            printerror( status );

        delete d;

        //OBS_MODE
        if(fits_write_key(fptr,TINT,"OBS_MODE",&kOBSMODE,"Acquisition frequence",&status))
            printerror( status );

        //ELAPTIME - date de fin obs - date de debut obs
        if(fits_write_key(fptr,TINT,"ELAPTIME",&kELAPTIME,"Time between the obs's start and end",&status))
            printerror( status );

        //ONTIME
        if(fits_write_key(fptr,TDOUBLE,"ONTIME",&kONTIME,"Integration time",&status))
            printerror( status );

        //EXPOSURE
        if(fits_write_key(fptr,TDOUBLE,"EXPOSURE",&kEXPOSURE,"Camera's exposure time (sec)",&status))
            printerror( status );

        //RADESYS
        char * radesys = new char[kRADESYS.length()+1];
        strcpy(radesys,kRADESYS.c_str());

        if(fits_write_key(fptr,TSTRING,"RADESYS",radesys,"",&status))
            printerror( status );

        delete radesys;

        //FILTER
        char * f = new char[kFILTER.length()+1];
        strcpy(f,kFILTER.c_str());

        if(fits_write_key(fptr,TSTRING,"FILTER",f,"",&status))
            printerror( status );

        delete f;

        //TELESCOP
        char * t = new char[kTELESCOP.length()+1];
        strcpy(t,kTELESCOP.c_str());

        if(fits_write_key(fptr,TSTRING,"TELESCOP",t,"",&status))
            printerror( status );

        delete t;

        //OBSERVER
        char * o = new char[kOBSERVER.length()+1];
        strcpy(o,kOBSERVER.c_str());

        if(fits_write_key(fptr,TSTRING,"OBSERVER",o,"",&status))
            printerror( status );

        delete o;

        //INSTRUME
        char * i = new char[kINSTRUME.length()+1];
        strcpy(i,kINSTRUME.c_str());

        if(fits_write_key(fptr,TSTRING,"INSTRUME",i,"",&status))
            printerror( status );

        delete i;

        //CAMERA
        char * cam = new char[kCAMERA.length()+1];
        strcpy(cam,kCAMERA.c_str());

        if(fits_write_key(fptr,TSTRING,"CAMERA",cam,"",&status))
            printerror( status );

        delete cam;

        //FOCAL
        if(fits_write_key(fptr,TDOUBLE,"FOCAL",&kFOCAL,"",&status))
            printerror( status );

        //APERTURE
        if(fits_write_key(fptr,TDOUBLE,"APERTURE",&kAPERTURE,"",&status))
            printerror( status );



        //SITELONG
        /*char * sitelong = new char[vSITELONG.length()+1];
        strcpy(sitelong,vSITELONG.c_str());*/

        if(fits_write_key(fptr,TDOUBLE,"SITELONG",&kSITELONG,"Longitude observatory",&status))
            printerror( status );

        //delete sitelong;

        //SITELAT
        /*char * sitelat = new char[vSITELAT.length()+1];
        strcpy(sitelat,vSITELAT.c_str());*/

        if(fits_write_key(fptr,TDOUBLE,"SITELAT",&kSITELAT,"Latitude observatory",&status))
            printerror( status );

        //delete sitelat;

        //SITEELEV
        /*char * siteelev = new char[vSITEELEV.length()+1];
        strcpy(siteelev,vSITEELEV.c_str());*/

        if(fits_write_key(fptr,TDOUBLE,"SITEELEV",&kSITEELEV,"Observatory elevation",&status))
            printerror( status );

       // delete siteelev;

        //GAINDB
        if(fits_write_key(fptr,TINT,"GAINDB",&kGAINDB,"Camera's gain",&status))
            printerror( status );

        //SATURATE
        if(fits_write_key(fptr,TDOUBLE,"SATURATE",&kSATURATE,"Max value in image",&status))
            printerror( status );

        //PROGRAM
        char * p = new char[kPROGRAM.length()+1];
        strcpy(p,kPROGRAM.c_str());

        if(fits_write_key(fptr,TSTRING,"PROGRAM",p,"Name of acquisition program",&status))
            printerror( status );

        delete p;

        //CREATOR
        char * c = new char[kCREATOR.length()+1];
        strcpy(c,kCREATOR.c_str());

        if(fits_write_key(fptr,TSTRING,"CREATOR",c,"Creator's name",&status))
            printerror( status );

        delete c;

        //CENTAZ
        char * centaz = new char[kCENTAZ.length()+1];
        strcpy(centaz,kCENTAZ.c_str());

        if(fits_write_key(fptr,TSTRING,"CENTAZ",centaz,"Nominal Azimuth of center of image in deg",&status))
            printerror( status );

        delete centaz;

        //CENTALT
        char * centalt = new char[kCENTALT.length()+1];
        strcpy(centalt,kCENTALT.c_str());

        if(fits_write_key(fptr,TSTRING,"CENTALT",centalt,"Nominal Altitude of center of image in deg",&status))
            printerror( status );

        delete centalt;

        //CENTOR
        char * centor = new char[kCENTOR.length()+1];
        strcpy(centor,kCENTOR.c_str());

        if(fits_write_key(fptr,TSTRING,"CENTOR",centor,"Orientation camera",&status))
            printerror( status );

        delete centor;

        //CRPIX1
        if(fits_write_key(fptr,TINT,"CRPIX1",&kCRPIX1,"Center fish eyes X",&status))
            printerror( status );

        //CRPIX2
        if(fits_write_key(fptr,TINT,"CRPIX2",&kCRPIX2,"Center fish eyes Y",&status))
            printerror( status );

        //K1
        if(fits_write_key(fptr,TDOUBLE,"K1",&kK1,"R = K1 * f * sin(theta/K2)",&status))
            printerror( status );

        //K2
        if(fits_write_key(fptr,TDOUBLE,"K2",&kK2,"R = K1 * f * sin(theta/K2)",&status))
            printerror( status );

        //EQUINOX
         if(fits_write_key(fptr,TDOUBLE,"EQUINOX",&kEQUINOX,"",&status))
            printerror( status );

        //CTYPE1
        char * ctype1 = new char[kCTYPE1.length()+1];
        strcpy(ctype1,kCTYPE1.c_str());

        if(fits_write_key(fptr,TSTRING,"CTYPE1",ctype1,"Projection and reference system",&status))
            printerror( status );

        //CTYPE2
        char * ctype2 = new char[kCTYPE2.length()+1];
        strcpy(ctype2,kCTYPE2.c_str());

        if(fits_write_key(fptr,TSTRING,"CTYPE2",ctype2,"Projection and reference system",&status))
            printerror( status );

        //CD1_1
        if(fits_write_key(fptr,TDOUBLE,"CD1_1",&kCD1_1,"deg/pix",&status))
            printerror( status );

        //CD1_2
        if(fits_write_key(fptr,TDOUBLE,"CD1_2",&kCD1_2,"deg/pix",&status))
            printerror( status );

        //CD2_1
        if(fits_write_key(fptr,TDOUBLE,"CD2_1",&kCD2_1,"deg/pix",&status))
            printerror( status );

        //CD2_2
        if(fits_write_key(fptr,TDOUBLE,"CD2_2",&kCD2_2,"deg/pix",&status))
            printerror( status );

        //CRVAL1
        if(fits_write_key(fptr,TDOUBLE,"CRVAL1",&kCRVAL1,"degree",&status))
            printerror( status );

        //CRVAL2
        if(fits_write_key(fptr,TDOUBLE,"CRVAL2",&kSITELAT,"deg/pix",&status))
            printerror( status );

        //XPIXEL
        if(fits_write_key(fptr,TDOUBLE,"XPIXEL",&kXPIXEL,"in micro meter",&status))
            printerror( status );

        //YPIXEL
        if(fits_write_key(fptr,TDOUBLE,"YPIXEL",&kYPIXEL,"in micro meter",&status))
            printerror( status );

        // close the file
        if ( fits_close_file(fptr, &status) )
             printerror( status );

    }

    return returnValue;
}

bool Fits2D::readFitsToMat(Mat &img, string filePath){

        float * ptr = NULL;
        float  * ptr1 = NULL;

        fitsfile *fptr;       // pointer to the FITS file, defined in fitsio.h
        int status,  nfound, anynull;
        long naxes[2], fpixel, nbuffer, npixels;

        float  nullval;
        //char filename[]  = fichier.c_str();     /* name of existing FITS file   */
        const char * filename;

        filename = filePath.c_str();
        status = 0;

        fits_open_file(&fptr, filename, READONLY, &status) ;

        // read the NAXIS1 and NAXIS2 keyword to get image size
        fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status) ;

        Mat image = Mat::zeros( naxes[1],naxes[0], CV_32FC1);

        npixels  = naxes[0] * naxes[1];         // number of pixels in the image
        fpixel   = 1;                           // first pixel
        nullval  = 0;                           // don't check for null values in the image

        nbuffer = npixels;

        float  buffer[npixels];

        fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,buffer, &anynull, &status) ;

        memcpy(image.ptr(), buffer, npixels * 4);

        Mat loadImg = Mat::zeros( naxes[1],naxes[0], CV_32FC1 );

        int nbpix = 0;

        for(int i = 0; i < naxes[1]; i++){ //y

            float * ptr = image.ptr<float>(i);
            float * ptr1 = loadImg.ptr<float >(naxes[1] - 1 - i);

            for(int j = 0; j < naxes[0]; j++){ //x

                //float val = ptr[j];
                  ptr1[j] = ptr[j];
              //  loadImg.at<float>(naxes[1] - i, j) = image.at<float>(i, j);
                nbpix++;

            }
        }

        loadImg.copyTo(img);

        fits_close_file(fptr, &status);


        return true;

}


//int fits::readFitsKeyWords(){
//
//	fitsfile *fptr;
//    char card[FLEN_CARD];
//    int status = 0,  nkeys, ii;  /* MUST initialize status */
//
//    fits_open_file(&fptr, "D:\\fitsTest.fits", READONLY, &status);
//    fits_get_hdrspace(fptr, &nkeys, NULL, &status);
//
//    for (ii = 1; ii <= nkeys; ii++)  {
//        fits_read_record(fptr, ii, card, &status); /* read keyword */
//        printf("%s\n", card);
//    }
//    printf("END\n\n");  /* terminate listing with END */
//    fits_close_file(fptr, &status);
//
//    if (status)          /* print any error messages */
//        fits_report_error(stderr, status);
//
//	return(status);
//
//}
//
//
//
///*--------------------------------------------------------------------------*/
//void fits::writeascii ( void )
//
//    /*******************************************************************/
//    /* Create an ASCII table extension containing 3 columns and 6 rows */
//    /*******************************************************************/
//{
//    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
//    int status;
//    long firstrow, firstelem;
//
//    int tfields = 3;       /* table will have 3 columns */
//    long nrows  = 6;       /* table will have 6 rows    */
//
//    char filename[] = "fitsTest.fit";           /* name for new FITS file */
//    char extname[] = "PLANETS_ASCII";             /* extension name */
//
//    /* define the name, datatype, and physical units for the 3 columns */
//    char *ttype[] = { "Planet", "Diameter", "Density" };
//    char *tform[] = { "a8",     "I6",       "F4.2"    };
//    char *tunit[] = { "\0",      "km",       "g/cm"    };
//
//    /* define the name diameter, and density of each planet */
//    char *planet[] = {"Mercury", "Venus", "Earth", "Mars","Jupiter","Saturn"};
//    long  diameter[] = {4880,    12112,    12742,   6800,  143000,   121000};
//    float density[]  = { 5.1,     5.3,      5.52,   3.94,    1.33,    0.69};
//
//    status=0;
//
//    /* open with write access the FITS file containing a primary array */
//    if ( fits_open_file(&fptr, filename, READWRITE, &status) )
//         printerror( status );
//
//    /* append a new empty ASCII table onto the FITS file */
//    if ( fits_create_tbl( fptr, ASCII_TBL, nrows, tfields, ttype, tform,
//                tunit, extname, &status) )
//         printerror( status );
//
//    firstrow  = 1;  /* first row in table to write   */
//    firstelem = 1;  /* first element in row  (ignored in ASCII tables) */
//
//    /* write names to the first column (character strings) */
//    /* write diameters to the second column (longs) */
//    /* write density to the third column (floats) */
//
//    fits_write_col(fptr, TSTRING, 1, firstrow, firstelem, nrows, planet,
//                   &status);
//    fits_write_col(fptr, TLONG, 2, firstrow, firstelem, nrows, diameter,
//                   &status);
//    fits_write_col(fptr, TFLOAT, 3, firstrow, firstelem, nrows, density,
//                   &status);
//
//    if ( fits_close_file(fptr, &status) )       /* close the FITS file */
//         printerror( status );
//    return;
//}
///*--------------------------------------------------------------------------*/
//void fits::writebintable ( void )
//
//    /*******************************************************************/
//    /* Create a binary table extension containing 3 columns and 6 rows */
//    /*******************************************************************/
//{
//    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
//    int status, hdutype;
//    long firstrow, firstelem;
//
//    int tfields   = 3;       /* table will have 3 columns */
//    long nrows    = 6;       /* table will have 6 rows    */
//
//    char filename[] = "fitsTest.fit";           /* name for new FITS file */
//    char extname[] = "PLANETS_Binary";           /* extension name */
//
//    /* define the name, datatype, and physical units for the 3 columns */
//    char *ttype[] = { "Planet", "Diameter", "Density" };
//    char *tform[] = { "8a",     "1J",       "1E"    };
//    char *tunit[] = { "\0",      "km",       "g/cm"    };
//
//    /* define the name diameter, and density of each planet */
//    char *planet[] = {"Mercury", "Venus", "Earth", "Mars","Jupiter","Saturn"};
//    long  diameter[] = {4880,     12112,   12742,   6800,  143000,   121000};
//    float density[]  = { 5.1,      5.3,     5.52,   3.94,   1.33,     0.69};
//
//    status=0;
//
//    /* open the FITS file containing a primary array and an ASCII table */
//    if ( fits_open_file(&fptr, filename, READWRITE, &status) )
//         printerror( status );
//
//    if ( fits_movabs_hdu(fptr, 2, &hdutype, &status) ) /* move to 2nd HDU */
//         printerror( status );
//
//    /* append a new empty binary table onto the FITS file */
//    if ( fits_create_tbl( fptr, BINARY_TBL, nrows, tfields, ttype, tform,
//                tunit, extname, &status) )
//         printerror( status );
//
//    firstrow  = 1;  /* first row in table to write   */
//    firstelem = 1;  /* first element in row  (ignored in ASCII tables) */
//
//    /* write names to the first column (character strings) */
//    /* write diameters to the second column (longs) */
//    /* write density to the third column (floats) */
//
//    fits_write_col(fptr, TSTRING, 1, firstrow, firstelem, nrows, planet,
//                   &status);
//    fits_write_col(fptr, TLONG, 2, firstrow, firstelem, nrows, diameter,
//                   &status);
//    fits_write_col(fptr, TFLOAT, 3, firstrow, firstelem, nrows, density,
//                   &status);
//
//    if ( fits_close_file(fptr, &status) )       /* close the FITS file */
//         printerror( status );
//    return;
//}
//
///*--------------------------------------------------------------------------*/
void Fits2D::printerror( int status){
//    /*****************************************************/
//    /* Print out cfitsio error messages and exit program */
//    /*****************************************************/


    if (status){
       fits_report_error(stderr, status); /* print error report */

       exit( status );    /* terminate the program, returning error status */
    }
    return;
}
//
//void fits::readheader ( void )
//
//    /**********************************************************************/
//    /* Print out all the header keywords in all extensions of a FITS file */
//    /**********************************************************************/
//{
//    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
//
//    int status, nkeys, keypos, hdutype, ii, jj;
//    char filename[]  = "fitsTest.fit";     /* name of existing FITS file   */
//    char card[FLEN_CARD];   /* standard string lengths defined in fitsioc.h */
//
//    status = 0;
//
//    if ( fits_open_file(&fptr, filename, READONLY, &status) )
//         printerror( status );
//
//    /* attempt to move to next HDU, until we get an EOF error */
//    for (ii = 1; !(fits_movabs_hdu(fptr, ii, &hdutype, &status) ); ii++)
//    {
//        /* get no. of keywords */
//        if (fits_get_hdrpos(fptr, &nkeys, &keypos, &status) )
//            printerror( status );
//
//        printf("Header listing for HDU #%d:\n", ii);
//        for (jj = 1; jj <= nkeys; jj++)  {
//            if ( fits_read_record(fptr, jj, card, &status) )
//                 printerror( status );
//
//            printf("%s\n", card); /* print the keyword card */
//        }
//        printf("END\n\n");  /* terminate listing with END */
//    }
//
//    if (status == END_OF_FILE)   /* status values are defined in fitsioc.h */
//        status = 0;              /* got the expected EOF error; reset = 0  */
//    else
//       printerror( status );     /* got an unexpected error                */
//
//    if ( fits_close_file(fptr, &status) )
//         printerror( status );
//
//    return;
//}
//
//void fits::readtable( void )
//
//    /************************************************************/
//    /* read and print data values from an ASCII or binary table */
//    /************************************************************/
//{
//    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
//    int status, hdunum, hdutype,  nfound, anynull, ii;
//    long frow, felem, nelem, longnull, dia[6];
//    float floatnull, den[6];
//    char strnull[10], *name[6], *ttype[3];
//
//    char filename[]  = "fitsTest.fit";     /* name of existing FITS file   */
//
//    status = 0;
//
//    if ( fits_open_file(&fptr, filename, READONLY, &status) )
//         printerror( status );
//
//    for (ii = 0; ii < 3; ii++)      /* allocate space for the column labels */
//        ttype[ii] = (char *) malloc(FLEN_VALUE);  /* max label length = 69 */
//
//    for (ii = 0; ii < 6; ii++)    /* allocate space for string column value */
//        name[ii] = (char *) malloc(10);
//
//    for (hdunum = 2; hdunum <= 3; hdunum++) /*read ASCII, then binary table */
//    {
//      /* move to the HDU */
//      if ( fits_movabs_hdu(fptr, hdunum, &hdutype, &status) )
//           printerror( status );
//
//      if (hdutype == ASCII_TBL)
//          printf("\nReading ASCII table in HDU %d:\n",  hdunum);
//      else if (hdutype == BINARY_TBL)
//          printf("\nReading binary table in HDU %d:\n", hdunum);
//      else
//      {
//          printf("Error: this HDU is not an ASCII or binary table\n");
//          printerror( status );
//      }
//
//      /* read the column names from the TTYPEn keywords */
//      fits_read_keys_str(fptr, "TTYPE", 1, 3, ttype, &nfound, &status);
//
//      printf(" Row  %10s %10s %10s\n", ttype[0], ttype[1], ttype[2]);
//
//      frow      = 1;
//      felem     = 1;
//      nelem     = 6;
//      strcpy(strnull, " ");
//      longnull  = 0;
//      floatnull = 0.;
//
//      /*  read the columns */
//      fits_read_col(fptr, TSTRING, 1, frow, felem, nelem, strnull,  name,
//                    &anynull, &status);
//      fits_read_col(fptr, TLONG, 2, frow, felem, nelem, &longnull,  dia,
//                    &anynull, &status);
//      fits_read_col(fptr, TFLOAT, 3, frow, felem, nelem, &floatnull, den,
//                    &anynull, &status);
//
//      for (ii = 0; ii < 6; ii++)
//        printf("%5d %10s %10ld %10.2f\n", ii + 1, name[ii], dia[ii], den[ii]);
//    }
//
//    for (ii = 0; ii < 3; ii++)      /* free the memory for the column labels */
//        free( ttype[ii] );
//
//    for (ii = 0; ii < 6; ii++)      /* free the memory for the string column */
//        free( name[ii] );
//
//    if ( fits_close_file(fptr, &status) )
//         printerror( status );
//
//    return;
//}
