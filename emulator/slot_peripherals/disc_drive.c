#include "disc_drive.h"
#include "board.h"
#include "timer.h"
#include "filesys_drv.h"

//////////////////////////////////////////////////////////////////////////////////////

#define WRITE_ENABLED	0

//////////////////////////////////////////////////////////////////////////////////////

#define  DRIVES			2
#define  TRACKS			35

#ifndef MAX
#define MAX(a,b)		( ( a > b ) ? a : b )
#endif

#ifndef MIN
#define MIN(a,b)		( ( a > b ) ? b : a )
#endif

#define  VOLUME 		0xFE
#define  IMAGETYPES 	7
//#define  MAX_PATH		255

#define  TRACK_BUFFER_SIZE		4096
#define  NIBBLIZED_BUFFER_SIZE  7168

#define ADDVALUE(a) value = (value << 2) |        \
                            (((a) & 0x01) << 1) | \
                            (((a) & 0x02) >> 1)

#define CODE44A(a) ((((a) >> 1) & 0x55) | 0xAA)
#define CODE44B(a) (((a) & 0x55) | 0xAA)

//////////////////////////////////////////////////////////////////////////////////////

typedef struct _disc_info_t
{
	bool_t track_ready;
	bool_t write_pending;

	uint8_t step_phase;
	uint8_t curr_phase;
	uint8_t curr_disc;
	uint8_t sector_order;

	char * disc0_hdl;
	char * disc1_hdl;
	char * curr_hdl;

	uint32_t curr_block;
	uint32_t data_offset;
	uint32_t block_size;
	uint32_t block_cnt;

} disc_info_t;

//////////////////////////////////////////////////////////////////////////////////////

#define DISK_IMAGE_S6D1			"discs/solo-flight.dsk"
#define DISK_IMAGE_S6D2			NULL

//////////////////////////////////////////////////////////////////////////////////////

const static uint8_t disk_drive_fw[] =
{
	0xA2,0x20,0xA0,0x00,0xA2,0x03,0x86,0x3C,0x8A,0x0A,0x24,0x3C,0xF0,0x10,0x05,0x3C,
	0x49,0xFF,0x29,0x7E,0xB0,0x08,0x4A,0xD0,0xFB,0x98,0x9D,0x56,0x03,0xC8,0xE8,0x10,
	0xE5,0x20,0x58,0xFF,0xBA,0xBD,0x00,0x01,0x0A,0x0A,0x0A,0x0A,0x85,0x2B,0xAA,0xBD,
	0x8E,0xC0,0xBD,0x8C,0xC0,0xBD,0x8A,0xC0,0xBD,0x89,0xC0,0xA0,0x50,0xBD,0x80,0xC0,
	0x98,0x29,0x03,0x0A,0x05,0x2B,0xAA,0xBD,0x81,0xC0,0xA9,0x56,0x20,0xA8,0xFC,0x88,
	0x10,0xEB,0x85,0x26,0x85,0x3D,0x85,0x41,0xA9,0x08,0x85,0x27,0x18,0x08,0xBD,0x8C,
	0xC0,0x10,0xFB,0x49,0xD5,0xD0,0xF7,0xBD,0x8C,0xC0,0x10,0xFB,0xC9,0xAA,0xD0,0xF3,
	0xEA,0xBD,0x8C,0xC0,0x10,0xFB,0xC9,0x96,0xF0,0x09,0x28,0x90,0xDF,0x49,0xAD,0xF0,
	0x25,0xD0,0xD9,0xA0,0x03,0x85,0x40,0xBD,0x8C,0xC0,0x10,0xFB,0x2A,0x85,0x3C,0xBD,
	0x8C,0xC0,0x10,0xFB,0x25,0x3C,0x88,0xD0,0xEC,0x28,0xC5,0x3D,0xD0,0xBE,0xA5,0x40,
	0xC5,0x41,0xD0,0xB8,0xB0,0xB7,0xA0,0x56,0x84,0x3C,0xBC,0x8C,0xC0,0x10,0xFB,0x59,
	0xD6,0x02,0xA4,0x3C,0x88,0x99,0x00,0x03,0xD0,0xEE,0x84,0x3C,0xBC,0x8C,0xC0,0x10,
	0xFB,0x59,0xD6,0x02,0xA4,0x3C,0x91,0x26,0xC8,0xD0,0xEF,0xBC,0x8C,0xC0,0x10,0xFB,
	0x59,0xD6,0x02,0xD0,0x87,0xA0,0x00,0xA2,0x56,0xCA,0x30,0xFB,0xB1,0x26,0x5E,0x00,
	0x03,0x2A,0x5E,0x00,0x03,0x2A,0x91,0x26,0xC8,0xD0,0xEE,0xE6,0x27,0xE6,0x3D,0xA5,
	0x3D,0xCD,0x00,0x08,0xA6,0x2B,0x90,0xDB,0x4C,0x01,0x08,0x00,0x00,0x00,0x00,0x00,
};

const static uint8_t diskbyte[ 0x40 ] = 
{ 
	0x96, 0x97, 0x9A, 0x9B, 0x9D, 0x9E, 0x9F, 0xA6,
	0xA7, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB2, 0xB3,
	0xB4, 0xB5, 0xB6, 0xB7, 0xB9, 0xBA, 0xBB, 0xBC,
	0xBD, 0xBE, 0xBF, 0xCB, 0xCD, 0xCE, 0xCF, 0xD3,
	0xD6, 0xD7, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE,
	0xDF, 0xE5, 0xE6, 0xE7, 0xE9, 0xEA, 0xEB, 0xEC,
	0xED, 0xEE, 0xEF, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6,
	0xF7, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

const static uint8_t sectornumber[ 3 ][ 0x10 ] = 
{
	{	0x00, 0x08, 0x01, 0x09, 0x02, 0x0A, 0x03, 0x0B,
		0x04, 0x0C, 0x05, 0x0D, 0x06, 0x0E, 0x07, 0x0F },
		
	{	0x00, 0x07, 0x0E, 0x06, 0x0D, 0x05, 0x0C, 0x04,
		0x0B, 0x03, 0x0A, 0x02, 0x09, 0x01, 0x08, 0x0F },
		
	{	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};


//////////////////////////////////////////////////////////////////////////////////////

static uint8_t track_buffer[8192];
static uint8_t block_buffer[8192];
static uint8_t * nibble_buffer1;
static uint8_t * nibble_buffer2;

static uint8_t sixbitbyte[0x80];

static disc_info_t disc_info; // [MAX_SUPPORTED_DISCS];

//static board_data_t * board;
// static disc_info_t * p_info;

//static uint8_t curr_slot;
static uint8_t write_mode;
static uint8_t latch_data;

//static loop_hdl_t local_loop;
static timer_data_t disc_led_timer;

//////////////////////////////////////////////////////////////////////////////////////

static void led_show( uint16_t led_id, bool_t is_on )
{
	// to be implemented by emulator infrastructure
}

static void disc_local_loop( uint64_t cycles )
{
	if( timer_Check( &disc_led_timer ) == TRUE )
	{
		timer_Stop( &disc_led_timer );
		led_show( LED_ID_DISC_D1, FALSE );
		led_show( LED_ID_DISC_D2, FALSE );
	}
}

static void gen_sixbit_table( void )
{
	// GENERATE A TABLE FOR CONVERTING
	// DISK BYTES BACK INTO 6-BIT BYTES
	int loop = 0;

	memset( sixbitbyte, 0x00, sizeof( sixbitbyte ) );

	while (loop < 0x40) 
	{
		sixbitbyte[diskbyte[loop]-0x80] = loop << 2;
		loop++;
	}
}

static void encode_62( uint8_t * input_byte, uint8_t * output_nibble ) 
{
	// CONVERT THE 256 8-BIT BYTES INTO 342 6-BIT BYTES, WHICH WE STORE
	// STARTING AT 4K INTO THE WORK BUFFER.
	{
		uint8_t * sectorbase = input_byte;
		uint8_t * resultptr  = nibble_buffer1;
		uint8_t   offset     = 0xAC;

		while (offset != 0x02) 
		{
			uint8_t value = 0;

			ADDVALUE(*(sectorbase+offset));  offset -= 0x56;
			ADDVALUE(*(sectorbase+offset));  offset -= 0x56;
			ADDVALUE(*(sectorbase+offset));  offset -= 0x53;

			*(resultptr++) = value << 2;
		}

		*(resultptr-2) &= 0x3F;
		*(resultptr-1) &= 0x3F;

		offset = 0;

		do
		{
			*(resultptr++) = *(sectorbase+(offset++));
		} while (offset);
	}

	// EXCLUSIVE-OR THE ENTIRE DATA BLOCK WITH ITSELF OFFSET BY ONE BYTE,
	// CREATING A 343RD BYTE WHICH IS USED AS A CHECKSUM.  STORE THE NEW
	// BLOCK OF 343 BYTES STARTING AT 5K INTO THE WORK BUFFER.
	{
		uint8_t   savedval  = 0;
		uint8_t * sourceptr = nibble_buffer1;
		uint8_t * resultptr = nibble_buffer2;

		int    loop      = 342;

		while (loop--) 
		{
			*(resultptr++) = savedval ^ *sourceptr;
			savedval = *(sourceptr++);
		}

		*resultptr = savedval;
	}

	// USING A LOOKUP TABLE, CONVERT THE 6-BIT BYTES INTO DISK BYTES.  A VALID
	// DISK BYTE IS A BYTE THAT HAS THE HIGH BIT SET, AT LEAST TWO ADJACENT
	// BITS SET (EXCLUDING THE HIGH BIT), AND AT MOST ONE PAIR OF CONSECUTIVE
	// ZERO BITS.  THE CONVERTED BLOCK OF 343 BYTES IS STORED STARTING AT 4K
	// INTO THE WORK BUFFER.
	{
		uint8_t * sourceptr = nibble_buffer2;
		uint8_t * resultptr = output_nibble;
		int       loop      = 343;

		while (loop--)
		{
			*(resultptr++) = diskbyte[(*(sourceptr++)) >> 2];
		}
	}
}

void decode_62( uint8_t * input_nibble, uint8_t * output_byte )
{
	// CONVERT THE DISK BYTES BACK INTO 6-BIT BYTES
	{
		uint8_t * sourceptr = input_nibble;
		uint8_t * resultptr = nibble_buffer2;

		int    loop      = 343;

		while (loop--)
		{
			*(resultptr++) = sixbitbyte[*(sourceptr++) & 0x7F];
		}
	}

	// EXCLUSIVE-OR THE ENTIRE DATA BLOCK WITH ITSELF OFFSET BY ONE BYTE
	// TO UNDO THE EFFECTS OF THE CHECKSUMMING PROCESS
	{
		uint8_t   savedval  = 0;
		uint8_t * sourceptr = nibble_buffer2;
		uint8_t * resultptr = nibble_buffer1;
		int    loop      = 342;
		while (loop--) 
		{
			*resultptr = savedval ^ *(sourceptr++);
			savedval = *(resultptr++);
		}
	}

	// CONVERT THE 342 6-BIT BYTES INTO 256 8-BIT BYTES
	{
		uint8_t * lowbitsptr = nibble_buffer1;
		uint8_t * sectorbase = &nibble_buffer1[0x56];
		uint8_t   offset     = 0xAC;

		while (offset != 0x02) 
		{
			if (offset >= 0xAC)
			{
				*(output_byte+offset) = (*(sectorbase+offset) & 0xFC)
									| (((*lowbitsptr) & 0x80) >> 7)
									| (((*lowbitsptr) & 0x40) >> 5);
			}

			offset -= 0x56;
			*(output_byte+offset) = (*(sectorbase+offset) & 0xFC)
								| (((*lowbitsptr) & 0x20) >> 5)
								| (((*lowbitsptr) & 0x10) >> 3);
			offset -= 0x56;
			*(output_byte+offset) = (*(sectorbase+offset) & 0xFC)
								| (((*lowbitsptr) & 0x08) >> 3)
								| (((*lowbitsptr) & 0x04) >> 1);
			offset -= 0x53;
			lowbitsptr++;
		}
	}
}

uint32_t nibblize_track( uint8_t * input, uint8_t * output, uint8_t sector_order, uint32_t track )
{
	uint8_t * imageptr = output;
	uint32_t sector_index;
	uint8_t sector;
	int loop;

	sector = 0;

	// WRITE GAP ONE, WHICH CONTAINS 48 SELF-SYNC BYTES
	for( loop = 0; loop < 48; loop++ ) *(imageptr++) = 0xFF;

	while (sector < 16) 
	{
		// WRITE THE ADDRESS FIELD, WHICH CONTAINS:
		//   - PROLOGUE (D5AA96)
		//   - VOLUME NUMBER ("4 AND 4" ENCODED)
		//   - TRACK NUMBER ("4 AND 4" ENCODED)
		//   - SECTOR NUMBER ("4 AND 4" ENCODED)
		//   - CHECKSUM ("4 AND 4" ENCODED)
		//   - EPILOGUE (DEAAEB)
		*(imageptr++) = 0xD5;
		*(imageptr++) = 0xAA;
		*(imageptr++) = 0x96;
		*(imageptr++) = 0xFF;
		*(imageptr++) = 0xFE;

		*(imageptr++) = CODE44A((uint8_t)track);
		*(imageptr++) = CODE44B((uint8_t)track);
		*(imageptr++) = CODE44A(sector);
		*(imageptr++) = CODE44B(sector);
		*(imageptr++) = CODE44A(0xFE ^ ((uint8_t)track) ^ sector);
		*(imageptr++) = CODE44B(0xFE ^ ((uint8_t)track) ^ sector);

		*(imageptr++) = 0xDE;
		*(imageptr++) = 0xAA;
		*(imageptr++) = 0xEB;

		// WRITE GAP TWO, WHICH CONTAINS SIX SELF-SYNC BYTES
		for (loop = 0; loop < 6; loop++) *(imageptr++) = 0xFF;

		// WRITE THE DATA FIELD, WHICH CONTAINS:
		//   - PROLOGUE (D5AAAD)
		//   - 343 6-BIT BYTES OF NIBBLIZED DATA, INCLUDING A 6-BIT CHECKSUM
		//   - EPILOGUE (DEAAEB)
		*(imageptr++) = 0xD5;
		*(imageptr++) = 0xAA;
		*(imageptr++) = 0xAD;

		sector_index = sectornumber[sector_order][sector] << 8;
		encode_62( &input[ sector_index ], imageptr );
		imageptr += 343;

		*(imageptr++) = 0xDE;
		*(imageptr++) = 0xAA;
		*(imageptr++) = 0xEB;

		// WRITE GAP THREE, WHICH CONTAINS 27 SELF-SYNC BYTES
		for (loop = 0; loop < 27; loop++) *(imageptr++) = 0xFF;

		sector++;
	}

	return ( ( uint32_t )( imageptr - output ) );
}

void denibblize_track( uint8_t * input, uint8_t * output, uint8_t sector_order, uint32_t input_size )
{
    uint32_t offset    = 0;
    int partsleft = 33;
    int sector    = 0;

	while (partsleft--) 
	{
		uint8_t byteval[3] = {0,0,0};
		int  bytenum    = 0;
		int  loop       = input_size;

		while ((loop--) && (bytenum < 3)) 
		{
			if (bytenum)
			{
				byteval[bytenum++] = input[offset++];
			}
			else if (input[offset++] == 0xD5)
			{
				bytenum = 1;
			}

			if (offset >= input_size)
			{
				offset = 0;
			}
		}
		if ((bytenum == 3) && (byteval[1] = 0xAA)) 
		{
			uint8_t * nibble_sector;
			uint8_t * byte_sector;

			nibble_sector = &input[ offset ];

			if (byteval[2] == 0x96)
			{
				sector = ((nibble_sector[4] & 0x55) << 1) | (nibble_sector[5] & 0x55);
			}
			else if (byteval[2] == 0xAD) 
			{
				byte_sector = &output[sectornumber[sector_order][sector]];
				decode_62( nibble_sector, byte_sector );
				sector = 0;
			}
		}
	}
}

uint16_t ReadTrack( uint16_t track, uint8_t * buffer )
{
	uint16_t nibble_size;

	memset( track_buffer, 0xFF, sizeof( track_buffer ) );

	if( disc_info.curr_hdl == NULL ) return 0;

	if( filesys_drv_image_access( disc_info.curr_hdl, Disc_Mode_Read, disc_info.curr_block << 12, block_buffer, TRACK_BUFFER_SIZE ) < TRACK_BUFFER_SIZE )
	{
		return 0;
	}

	nibble_size = nibblize_track( block_buffer, track_buffer, disc_info.sector_order, track );

	return nibble_size;
}

void WriteTrack( uint16_t track, uint8_t * buffer )
{
	if( disc_info.curr_hdl == NULL ) return;

	denibblize_track( track_buffer, block_buffer, disc_info.sector_order, disc_info.block_size );

	filesys_drv_image_access( disc_info.curr_hdl, Disc_Mode_Write, disc_info.curr_block << 12, block_buffer, TRACK_BUFFER_SIZE );
}

static uint16_t get_rand( void )
{
	return( 0 );
}

static uint8_t get_rand_data( uint8_t highbit )
{
	static const uint8_t retval[16] = 
	{
		0x00,0x2D,0x2D,0x30,0x30,0x32,0x32,0x34,
		0x35,0x39,0x43,0x43,0x43,0x60,0x7F,0x7F
	};
	
	uint8_t r = ( uint8_t )( get_rand() & 0xFF );
	
 	if( r <= 170 ) return( 0x20 | ( highbit ? 0x80 : 0 ) );
	else return( retval[ r & 15 ] | ( highbit ? 0x80 : 0 ) );
}

static uint8_t DiskControlStepper( uint8_t address, uint8_t data, bool_t is_write )
{
	int phase     = ( address >> 1 ) & 3;
	int phase_bit = ( 1 << phase );
	int direction = 0;

	// cpu_boost();

	// update the magnet states
	if( address & 1 )
	{
		// phase on
		disc_info.step_phase |= phase_bit;
	}
	else
	{
		// phase off
		disc_info.step_phase &= ~phase_bit;
	}

	// check for any stepping effect from a magnet
	// - move only when the magnet opposite the cog is off
	// - move in the direction of an adjacent magnet if one is on
	// - do not move if both adjacent magnets are on
	// momentum and timing are not accounted for ... maybe one day!
	if ((disc_info.step_phase & (1 << disc_info.curr_phase)) == 0)
	{
		if (disc_info.step_phase & (1 << ((disc_info.curr_phase + 1) & 3)))
			direction += 1;
		
		if (disc_info.step_phase & (1 << ((disc_info.curr_phase + 3) & 3)))
			direction -= 1;
	}

	// apply magnet step, if any
	if (direction)
	{
		int newtrack;
		
		disc_info.curr_phase = MAX(0, MIN(79, disc_info.curr_phase + direction));
		newtrack = MIN(TRACKS-1, disc_info.curr_phase >> 1); // (round half tracks down)

		if( newtrack != disc_info.curr_block )
		{
			if( disc_info.write_pending )
			{
				// there is a write pending
				// denibblize the temporary track buffer (nibbles) into image data (4kb)
				// and write to the disc image
				#if( WRITE_ENABLED )
				WriteTrack( disc_info.curr_block, track_buffer );
				#endif
			}
			
			disc_info.write_pending = FALSE;
			disc_info.track_ready = FALSE;
			disc_info.curr_block = newtrack;

			printf( "Disc head moved to track %d\n", newtrack );
		}
	}
	
	return (address == 0xE0) ? 0xFF : get_rand_data(1);
}

static uint8_t DiskControlMotor( uint8_t address, uint8_t data, bool_t is_write )
{
	uint8_t motor_state = address & 0x01;
	//uint8_t slot = ( address & 0x070 ) >> 4;

	if( motor_state == 0 )
	{
		if( disc_info.write_pending )
		{
			// there is a write pending
			// denibblize the temporary track buffer (nibbles) into image data (4kb)
			// and write to the disc image
			#if( WRITE_ENABLED )
			WriteTrack( disc_info.curr_block, track_buffer );
			#endif
		}

		timer_Start( &disc_led_timer, 500 );

		disc_info.track_ready = FALSE;
		disc_info.write_pending = FALSE;
	}

	return 0;
}

static uint8_t DiskEnable( uint8_t address, uint8_t data, bool_t is_write )
{
	if( disc_info.write_pending )
	{
		// there is a write pending
		// denibblize the temporary track buffer (nibbles) into image data (4kb)
		// and write to the disc image
		#if( WRITE_ENABLED )
		WriteTrack( disc_info.curr_block, track_buffer );
		#endif
	}

	disc_info.track_ready = FALSE;
	disc_info.write_pending = FALSE;

	disc_info.curr_disc = address & 1;

	if( disc_info.curr_disc == 0 )
	{
		led_show( LED_ID_DISC_D1, TRUE );
		led_show( LED_ID_DISC_D2, FALSE );
	}
	else
	{
		led_show( LED_ID_DISC_D1, FALSE );
		led_show( LED_ID_DISC_D2, TRUE );
	}

	disc_info.curr_hdl = ( disc_info.curr_disc ) ? disc_info.disc1_hdl : disc_info.disc0_hdl;

	return 0;
}

static uint8_t DiskReadWrite( uint8_t address, uint8_t data, bool_t is_write )
{
	uint8_t result = 0;

	// cpu_boost();

	if( disc_info.track_ready == FALSE )
	{
		// temporary track buffer (nibbles) is empty, read image track (4kb), nibblize and put into the temporary buffer
		disc_info.block_size = ReadTrack( disc_info.curr_block, track_buffer );
		if( disc_info.block_size > 0 )
		{
			disc_info.track_ready = TRUE;
			disc_info.data_offset = 0;
		}
		else
		{
			return 0xFF;
		}
	}

	if( write_mode == TRUE )
	{
		if( latch_data & 0x80 )
		{
			// write data to the temporary track buffer (nibbles)
			track_buffer[disc_info.data_offset] = latch_data;
			disc_info.write_pending = TRUE;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		// read from the temporary track buffer (nibbles)
		result = track_buffer[disc_info.data_offset];
	}

	if( ++disc_info.data_offset >= disc_info.block_size )
	{
		disc_info.data_offset = 0;
	}
	
	return result;
}

static uint8_t DiskSetLatchValue( uint8_t address, uint8_t data, bool_t is_write )
{
	//cpu_boost();

	if ( is_write == FALSE )
	{
		latch_data = data;
	}

	return data;
}

static uint8_t DiskSetReadMode( uint8_t address, uint8_t data, bool_t is_write )
{
	write_mode = 0;
	return get_rand_data( 0 );
}

static uint8_t DiskSetWriteMode( uint8_t address, uint8_t data, bool_t is_write )
{
	write_mode = 1;
	return get_rand_data( 1 );
}

void disc_process( uint64_t cycles )
{
	disc_local_loop( cycles );
}

uint8_t disc_rom_access( uint8_t address )
{
	return disk_drive_fw[ address & 0x0FF ];
}

uint8_t disc_io_access( uint8_t address, uint8_t data_in, bool_t is_write )
{
	switch( address & 0x0F )
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			return DiskControlStepper( address, data_in, is_write );

		case 0x08:
		case 0x09:
			return DiskControlMotor( address, data_in, is_write );

		case 0x0A:
		case 0x0B:
			return DiskEnable( address, data_in, is_write );

		case 0x0C:
			return DiskReadWrite( address, data_in, is_write );

		case 0x0D:
			return DiskSetLatchValue( address, data_in, is_write );

		case 0x0E:
			return DiskSetReadMode( address, data_in, is_write );

		case 0x0F:
			return DiskSetWriteMode( address, data_in, is_write );
	}

	return 0;
}

void disc_drive_init( void )
{
	memset( &disc_info, 0x00, sizeof( disc_info_t ) );

	nibble_buffer1 = &block_buffer[TRACK_BUFFER_SIZE];
	nibble_buffer2 = &block_buffer[TRACK_BUFFER_SIZE+512];

	gen_sixbit_table();

	disc_info.disc0_hdl = DISK_IMAGE_S6D1;
	disc_info.disc1_hdl = DISK_IMAGE_S6D2;
	disc_info.sector_order = Sector_Order_DSK;

	timer_Stop( &disc_led_timer );
}
