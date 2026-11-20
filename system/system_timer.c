#include <time.h>
#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "system_timer.h"

//////////////////////////////////////////////////////////////////////////////////////

static struct timeval ms_start;

//////////////////////////////////////////////////////////////////////////////////////

uint32_t system_timer_get_ms( void )
{
    struct timeval ms_value;
    long mtime, seconds, useconds;
    
    gettimeofday( &ms_value, NULL );
    
    seconds = ms_value.tv_sec - ms_start.tv_sec;
    useconds = ms_value.tv_usec - ms_start.tv_usec;
    mtime = ( seconds * 1000 ) + ( useconds / 1000 );
    
    return ( uint32_t )mtime;
}

uint64_t system_timer_get_us( void )
{
    struct timeval us_value;
    long mtime, seconds, useconds;

    gettimeofday( &us_value, NULL );

    seconds = us_value.tv_sec - ms_start.tv_sec;
    useconds = us_value.tv_usec - ms_start.tv_usec;
    mtime = ( seconds * 1000000 ) + useconds;

    return ( uint64_t )mtime;
}

void system_timer_init( void )
{
    gettimeofday( &ms_start, NULL );
}
