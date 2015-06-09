
#include <stdlib.h>/*
#include "SpiceUsr.h"
#include "SpiceGF.h"
#include "SpiceZfc.h"
#include "SpiceZmc.h"
#include "zzalloc.h"*/
#include <string>
#include <iostream>
#include <stdio.h>

class Ephemeris {

    public :

        static void ephemeris() {

            //#define       MAXWIN   750
            //#define       TIMFMT   "YYYY-MON-DD HR:MN:SC.###### (TDB) ::TDB ::RND"
            //#define       TIMLEN   41

            ///*
            //Create the needed windows. Note, one window
            //consists of two values, so the total number
            //of cell values to allocate is twice
            //the number of intervals.
            //*/
            //SPICEDOUBLE_CELL ( result, 2*MAXWIN );
            //SPICEDOUBLE_CELL ( cnfine, 2        );

            //SpiceDouble       begtim;
            //SpiceDouble       endtim;
            //SpiceDouble       step;
            //SpiceDouble       adjust;
            //SpiceDouble       refval;
            //SpiceDouble       beg;
            //SpiceDouble       end;

            //SpiceChar         begstr [ TIMLEN ];
            //SpiceChar         endstr [ TIMLEN ];
            //SpiceChar       * relate = "ABSMAX";
            //SpiceChar       * crdsys = "RA/DEC";
            //SpiceChar       * coord  = "DECLINATION";
            //SpiceChar       * targ   = "SUN";
            //SpiceChar       * obsrvr = "EARTH";
            //SpiceChar       * frame  = "J2000";
            //SpiceChar       * abcorr = "NONE";

            //SpiceInt          count;
            //SpiceInt          i;

            ////Load kernels.
            //furnsh_c( "C:/Users/Yoan/Documents/GitHub/freeture/libraries/standard.tm" );

            //// Store the time bounds of our search interval in the cnfine confinement window.
            ////http://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/str2et_c.html
            //str2et_c( "2015 JAN 10 00:01:00", &begtim );
            //str2et_c( "2015 JAN 10 23:59:00", &endtim );

            //// Insert an interval into a double precision window. 
            //// http://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/wninsd_c.html
            //wninsd_c( begtim, endtim, &cnfine );

            ///*
            //The latitude varies relatively slowly, ~46 degrees during the
            //year. The extrema occur approximately every six months.
            //Search using a step size less than half that value (180 days).
            //For this example use ninety days (in units of seconds).
            //*/
            //// spd_c() -> Return the number of seconds in a day.
            //step   = (90.)*spd_c();//((3.)/24.)*spd_c();
            //adjust = 0.;
            //refval = 20. * rpd_c();//(48.7063906 - 108.)*rpd_c(); //(48.7063906 + 108)

            //printf("refval : %f \n",refval);
            //printf("step : %f \n",step);

            ///*
            //List the beginning and ending points in each interval
            //if result contains data.
            //*/
            //gfposc_c (  targ,
            //            frame,
            //            abcorr,
            //            obsrvr,
            //            crdsys,
            //            coord,
            //            relate,
            //            refval,
            //            adjust,
            //            step,
            //            MAXWIN,
            //            &cnfine,
            //            &result  );

            //// Return the cardinality (number of intervals) of a double precision window.
            ////http://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/wncard_c.html
            //count = wncard_c( &result );
            //printf( "nb d : %d \n " ,count);

            ///*
            //Display the results.
            //*/
            //if(count == 0 ) {

            //    printf ( "Result window is empty.\n\n" );

            //}else {

            //    for( i = 0;  i < count;  i++ ){

            //        /*
            //        Fetch the endpoints of the Ith interval
            //        of the result window.
            //        */
            //        wnfetd_c ( &result, i, &beg, &end );

            //        if( beg == end ) {
            //            timout_c ( beg, TIMFMT, TIMLEN, begstr );
            //            printf ( "Event time: %s\n", begstr );
            //            
            //        }else {

            //            timout_c ( beg, TIMFMT, TIMLEN, begstr );
            //            timout_c ( end, TIMFMT, TIMLEN, endstr );

            //            printf ( "Interval %d\n", i + 1);
            //            printf ( "From : %s \n", begstr );
            //            printf ( "To   : %s \n", endstr );
            //            printf( " \n" );

            //        }
            //    }
            //}

            //kclear_c();
            
        }

};