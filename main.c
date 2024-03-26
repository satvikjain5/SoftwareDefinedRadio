/* Ground Penetrating Radar for bladeRF 2.0 micro*/
#include <pthread.h>
#include "common.h"
#include <math.h>


#ifdef USE_BLADERF
#include <libbladeRF.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <getopt.h>
#include <unistd.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

// ----------------------------------------------------------------some sys stuff----------------------------------------------------------------------------//
#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif


int gettimeofday(struct timeval *tv, void *ignored)
{
  FILENAME ft;
  unsigned __int64 tmp = 0;
  if (NULL != tv){
    GetSystemTimeAsFileTime
  }
}
