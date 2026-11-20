#include "filesys_drv.h"

////////////////////////////////////////////////////////////////////////

#define BOARD_PATH          "/home/marcio/Projects/emulators/apple_cpld_emulator/images"

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

static char image_filename[256];

////////////////////////////////////////////////////////////////////////

char * convert_filename( const char * handler )
{
    //char * p_name;
    
    strcpy( image_filename, BOARD_PATH );
    strcat( image_filename, "/" );
    strcat( image_filename, handler );
    
#ifdef WIN32
    while( ( p_name = strchr( image_filename, '/' ) ) != NULL )
    {
        *p_name = '\\';
    }
#endif
    
    return &image_filename[0];
}

void filesys_drv_init( void )
{
}

uint32_t filesys_drv_image_access( const char * handler, uint8_t mode, uint32_t pos, uint8_t * buffer, uint32_t size )
{
    uint32_t ret = 0;
    FILE * fp = NULL;
    
    if( mode == Disc_Mode_Remove )
    {
        return ( uint32_t )remove( convert_filename( handler ) );
    }
    
    else if( mode == Disc_Mode_Append )
    {
        fp = fopen( convert_filename( handler ), "ab" );
    }
    else
    {
        fp = fopen( convert_filename( handler ), "r+b" );
        if( fp == NULL )
        {
            if( mode == Disc_Mode_Write )
            {
                fp = fopen( convert_filename( handler ), "wb" );
            }
            
            if( fp == NULL ) return 0;
        }
        
        if( pos != 0 ) fseek( fp, ( long )pos, SEEK_SET );
    }
    
    switch( mode )
    {
        case Disc_Mode_Read:
            ret = ( uint32_t )fread( buffer, 1, size, fp );
            break;
            
        case Disc_Mode_Append:
        case Disc_Mode_Write:
            ret = ( uint32_t )fwrite( buffer, 1, size, fp );
            break;
            
        case Disc_Mode_Size:
            fseek( fp, ( long )0, SEEK_END );
            ret = ( uint32_t )ftell(fp);
            break;
    }
    
    fclose( fp );
    
    return( ret );
}
