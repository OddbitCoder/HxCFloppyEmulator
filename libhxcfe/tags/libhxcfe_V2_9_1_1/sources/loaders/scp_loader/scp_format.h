/*
//
// Copyright (C) 2006-2018 Jean-Fran�ois DEL NERO
//
// This file is part of the HxCFloppyEmulator library
//
// HxCFloppyEmulator may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

/*
This information is copyright (C) 2012-2014 By Jim Drew.  Permissions is granted
for inclusion with any source code when keeping this copyright notice.

Latest changes made: 04/12/14

The SuperCard Pro image file format will handle flux level images for any type of disk,
be it 8", 5.25", 3.5", 3", GCR, FM, MFM, etc.

All longwords are in little endian format.  The words used for the flux length in the
actual flux data are in big endian format... sorry, that's just how it worked out for
the SuperCard Pro hardware.

All offsets are the start of the file (byte 0) unless otherwise stated.  The .scp image
consists of a disk definition header, the track data header offset table, and the flux
data for each track, which is preceeded by Track Data Header.  The format is described
below:

BYTES 0x00-0x02 contains the ASCII of "SCP" as the first 3 bytes. If this is not found,
then the file is not ours.

BYTE 0x03 is the version/revision as a byte.  This is encoded as (Version<<4|Revision),
so that 0x39= version 3.9 of the format.  This is the version number of the SCP imaging
software that created this image.

BYTE 0x04 is the disk type and represents the type of disk for the image (see disk types
in the defines).

BYTE 0x05 is the number of revolutions, which is how many revolutions for each track is
contained in the image.

BYTES 0x06 and 0x07 are the start track and end track bytes.  Tracks are numbered 0-165,
which is a maximum of 166 tracks (83 tracks with top/bottom).

BYTE 0x08 is the FLAGS byte.  This byte contains information about how the image was
produced.  The bits are defined as follows:

Bit 0 - INDEX, cleared if the image did not use the index mark for queuing tracks
               set is the image used the index mark to queue tracks
			   
Bit 1 - TPI, cleared if the drive is a 48TPI drive
             set if the drive is a 96TPI drive
			 
Bit 2 - RPM, cleared if the drive is a 300 RPM drive
             set if the drive is a 360 RPM drive

Bit 3 - TYPE, cleared for preservation quality image
              set if flux has been normalized, reducing quality

Bit 4 - MODE, cleared if the image is read-only
              set if the image is read/write capable
  
FLAGS bit 0 is used to determine when the reading of flux data started.  If this bit is
set then the flux data was read immediately after the index pulse was detected.  If
this bit is clear then the flux data was read starting at some random location on the
track.

FLAGS bit 1 is used for determining the type of 5.25" drive was used.  Does not apply
to any 3.5" drives.

FLAGS bit 2 is used to determine the approximate RPM of the drive.  When FLAGS bit 0 is
clear, the index pulse is simulated using either a 166.6667ms (360 RPM) or 200ms (300 RPM)
index pulse.

FLAGS bit 3 is used to determine if the image was made with the full resolution possible
or if the image quality was reduced using a normalization routine that is designed to
reduce the file size when compressed.

FLAGS bit 4 is used to determine if the image is read-only or read/write capable.  Most
images will be read-only (write protected) for emulation usage.  The read/write capable
images contain padded space to allow the track to change size within the image.  Only a
single revolution is allowed when the TYPE bit is set (read/write capable).

BYTE 0x09 is the width of the bit cell time.  Normally this is always 0 which means
16 bits wide, but if the value is non-zero then it represents the number of bits for
each bit cell entry.  For example, if this byte was set to 8, then each bit cell entry
would be 8 bits wide, not the normal 16 bits.  This is for future expansion, and may never
be actually used.

BYTE 0x0A is the number of heads.  This value is either 1 or 2.  If the value is 0 then
it is due to being an older version of the SCP imaging program created the image.

BYTE 0x0B is reserved for future use.

BYTES 0x0C-0x0F are the 32 bit checksum of data starting from offset 0x10 through the
end of the image file.  Checksum is standard addition, with a wrap beyond 32 bits
rolling over.  A value of 0x00000000 is used when FLAGS bit 4 (MODE) is set, as no checksum
is calculated for read/write capable images.

BYTES 0x10-0x2A7 are a table of longwords with each entry being a offset to a Track Data
Header (TDH) for each track that is stored in the image.  The table is always sequential.
There is an entry for every track, with up to 166 tracks supported.  This means that disk
images of up to 83 tracks with sides 0/1 are possible.  If no flux data for a track is
present, then the entry will contain a longword of zeros (0x00000000).  The 1st TDH
will probably start at offset 0x000002A8, but could technically start anywhere in the file
because all entries are offset based, so track data does not have to be in any order.  This
was done so you could append track data to the file and change the header to point to the
appended data.  For simplicity it is recommended that you follow the normal image format
shown below, with all tracks in sequential order.  The SuperCard Pro imaging software will
always create a disk with all tracks and revolutions stored in sequentially.


TRACK DATA HEADER (TDH) INFO:
----------------------------

As stated, each entry is an offset to a Track Data Header.  That header contains
information about the flux data for that track.  To help recover corrupt files,
each entry has its own header starting with "TRK" as the first 3 bytes of the header and
the track number as the 4th byte.

When imaging a single side only, the track data header entry will skip every other entry.

BYTES 0x00-0x02 ASCII 'TRK'

BYTE 0x03 contains the track number (0-165).  For single sided disks, tracks 0-42 (48TPI)
or 0-82 (96TPI) are used.  For double-sided disks, the actual track number is this value
divided by 2.  The remainder (0 or 1) is the head.  0 represents the bottom head and
1 represents the top head.  For example, 0x0B would be 11/2 = track 5, head 1 (top).
Likewise, 0x50 would be 80/2 = track 40, head 0 (bottom).

Starting at BYTE 0x04 are three longwords for each revolution that is stored.  Typically,
a maximum of five sets of three longwords are stored using the SuperCard Pro's imaging
program, but this is user defined and can vary from image to image.  Using BYTE 0x05 of
the main file header, you can determine the number of sets of three longwords (one for each
revolution stored).  The three longwords for each entry are described below:

BYTES 0x04-0x07 contain the duration of the 1st revolution.  This is the time from index
pulse to index pulse.  This will be either ~360RPMs or ~300 RPMs depending the drive.
It is important to have this exact time as it is necessary when writing an image back to a
real disk.  The index time is a value in nanoseconds/25ns for one revolution.  Multiply
the value by 25 to get the exact time value in nanoseconds.

BYTES 0x08-0x0B contain the length of the track in bitcells (flux transitions).  If a
16 bit width is used (selected above), then this value *2 will be the total number of
bytes of data that is used for that track (16 bit = 2 bytes per word).

BYTES 0x0C-0x0F contains the offset from the start of the Track Data Header (BYTE 0x00,
which is the "T" in "TRK) to the flux data.  NOTE!!  THIS OFFSET IS *NOT* FROM THE
START OF THE FILE!

If there were more revolutions, there would be three more longwords stored for each
additional revolution.. example:

BYTES 0x10-0x1B would be the 2nd entry (duration2, length2, offset2).
BYTES 0x1C-0x27 would be the 3rd entry (duration3, length3, offset3).
BYTES 0x28-0x33 would be the 4th entry (duration4, length4, offset4).
BYTES 0x34-0x3F would be the 5th entry (duration5, length5, offset5).

Note that image types with FLAGS bit 3 set (READ/WRITE capability) only uses a single
revolution, but the space allocated for the flux data will be the maximum necessary to
store the worst case requirement.  The actual flux data will be inside of this space
and the current length will be accurate.  This allows the track to expand and contract
inside of the allocated space without overlapping and corrupting the file.

Flux data for each revolution is always stored sequentially (like the disk is spinning
multiple times) with SuperCard Pro's imager..  No break or skew occurs.  However, it is
possible that flux data could be located anywhere in the file because everything is
offset based.  For simplicity, please try to keep the revolutions sequential.  SuperCard
Pro's imaging software always stores the revolutions sequentially.
 

After the last byte of flux data there will be a timestamp.  This timestamp is an
ASCII string, ie: 1/05/2014 5:15:21 PM



; ------------------------------------------------------------------
; SCP IMAGE FILE FORMAT
; ------------------------------------------------------------------
;
; 0000              'SCP' (ASCII CHARS)
; 0003              VERSION (nibbles major/minor)
; 0004              DISK TYPE
;                   UPPER 4 BITS ARE USED TO DEFINE A DISK CLASS (MANUFACTURER)
;                   LOWER 4 BITS ARE USED TO DEFINE A DISK SUB-CLASS (MACHINE)
;
;                   MANUFACTURER BIT DEFINITIONS:
;                   0000 = COMMODORE
;                   0001 = ATARI
;                   0010 = APPLE
;                   0011 = PC
;                   0100 = TANDY
;                   0101 = TEXAS INSTRUMENTS
;                   0110 = ROLAND
;
;					SEE DISK TYPE BIT DEFINITIONS BELOW
;
; 0005              NUMBER OF REVOLUTIONS (1-5)
; 0006              START TRACK (0-165)
; 0007              END TRACK (0-165)
; 0008              FLAGS BITS (0=INDEX, 1=TPI, 2=RPM, 3=TYPE)
; 0009              BIT CELL ENCODING (0=16 BITS, >0=NUMBER OF BITS USED)
; 000A-B            RESERVED (2 BYTES)
; 000C-F            32 BIT CHECKSUM OF DATA FROM 0x10-EOF
; 0010              OFFSET TO 1st TRACK DATA HEADER (4 bytes of 0 if track is skipped)
; 0014              OFFSET TO 2nd TRACK DATA HEADER (4 bytes of 0 if track is skipped)
; 0018              OFFSET TO 3rd TRACK DATA HEADER (4 bytes of 0 if track is skipped)
; ....
; 02A4              OFFSET TO 166th TRACK DATA HEADER (4 bytes of 0 if track is skipped)
; 02A8              TYPICAL START OF 1st TRACK DATA HEADER (always the case with SCP created images)
;
; ....              END OF TRACK DATA
; ????              TIMESTAMP (AS ASCII STRING - ie. 7/17/2013 12:45:49 PM)

; ## FILE FORMAT DEFINES ##

IFF_ID = 0x00                      ; "SCP" (ASCII CHARS)
IFF_VER = 0x03                     ; version (nibbles major/minor)
IFF_DISKTYPE = 0x04                ; disk type (0=CBM, 1=AMIGA, 2=APPLE II, 3=ATARI ST, 4=ATARI 800, 5=MAC 800, 6=360K/720K, 7=1.44MB)
IFF_NUMREVS = 0x05                 ; number of revolutions (2=default)
IFF_START = 0x06                   ; start track (0-165)
IFF_END = 0x07                     ; end track (0-165)
IFF_FLAGS = 0x08                   ; FLAGS bits (0=INDEX, 1=TPI, 2=RPM, 3=TYPE - see defines below)
IFF_ENCODING = 0x09                ; BIT CELL ENCODING (0=16 BITS, >0=NUMBER OF BITS USED)
IFF_RSRVED = 0x0A                  ; 2 bytes of reserved space
IFF_CHECKSUM = 0x0C                ; 32 bit checksum of data added together starting at 0x0010 through EOF
IFF_THDOFFSET = 0x10               ; first track data header offset
IFF_THDSTART = 0x2A8               ; start of first Track Data Header

; FLAGS BIT DEFINES (BIT NUMBER)

FB_INDEX = 0x00                    ; clear = no index reference, set = flux data starts at index
FB_TPI = 0x01                      ; clear = drive is 48TPI, set = drive is 96TPI (only applies to 5.25" drives!)
FB_RPM = 0x02                      ; clear = drive is 300 RPM drive, set = drive is 360 RPM drive
FB_TYPE = 0x03                     ; clear = image is has original flux data, set = image is flux data that has been normalized
FB_MODE = 0x04                     ; clear = image is read-only, set = image is read/write capable

; MANUFACTURERS                      7654 3210
man_CBM = 0x00                     ; 0000 xxxx
man_Atari = 0x10                   ; 0001 xxxx
man_Apple = 0x20                   ; 0010 xxxx
man_PC = 0x40                      ; 0011 xxxx
man_Tandy = 0x21                   ; 0100 xxxx
man_TI = 0x40                      ; 0101 xxxx
man_Roland = 0x60                  ; 0110 xxxx

; DISK TYPE BIT DEFINITIONS
;
; CBM DISK TYPES
disk_C64 = 0x00                    ; xxxx 0000
disk_Amiga = 0x04                  ; xxxx 0100

; ATARI DISK TYPES
disk_AtariFMSS = 0x00              ; xxxx 0000
disk_AtariFMDS = 0x01              ; xxxx 0001
disk_AtariFMEx = 0x02              ; xxxx 0010
disk_AtariSTSS = 0x04              ; xxxx 0100
disk_AtariSTDS = 0x05              ; xxxx 0101

; APPLE DISK TYPES
disk_AppleII = 0x00                ; xxxx 0000
disk_AppleIIPro = 0x01             ; xxxx 0001
disk_Apple400K = 0x04              ; xxxx 0100
disk_Apple800K = 0x05              ; xxxx 0101
disk_Apple144 = 0x06               ; xxxx 0110

; PC DISK TYPES
disk_PC360K = 0x00                 ; xxxx 0000
disk_PC720K = 0x01                 ; xxxx 0001
disk_PC12M = 0x02                  ; xxxx 0010
disk_PC144M = 0x03                 ; xxxx 0011

; TANDY DISK TYPES
disk_TRS80SSSD = 0x00              ; xxxx 0000
disk_TRS80SSDD = 0x01              ; xxxx 0001
disk_TRS80DSSD = 0x10              ; xxxx 0010
disk_TRS80DSDD = 0x11              ; xxxx 0011

; TI DISK TYPES
disk_TI994A = 0x00                 ; xxxx 0000

; ROLAND DISK TYPES
disk_D20 = 0x00                    ; xxxx 0000


; ------------------------------------------------------------------
; TRACK DATA HEADER FORMAT
; ------------------------------------------------------------------
;
; 0000              'TRK' (ASCII CHARS)             - 3 chars
; 0003              TRACK NUMBER                    - 1 byte
; ....              START OF TABLE OF ENTRIES FOR EACH REVOLUTION
; 0004              INDEX TIME (1st REVOLUTION)     - 4 bytes
; 0008              TRACK LENGTH (1st REVOLUTION)   - 4 bytes
; 000C              DATA OFFSET (1st REVOLUTION)    - 4 bytes (offset is from start of Track Data Header)
; ....              ADDITIONAL ENTRIES FOR EACH REVOLUTION (IF AVAILABLE, OTHERWISE THIS WILL BE FLUX DATA)...
; 0010              INDEX TIME (2nd REVOLUTION)     - 4 bytes
; 0014              TRACK LENGTH (2nd REVOLUTION)   - 4 bytes
; 0018              DATA OFFSET (2nd REVOLUTION)    - 4 bytes
; 001C              INDEX TIME (3rd REVOLUTION)     - 4 bytes
; 0020              TRACK LENGTH (3rd REVOLUTION)   - 4 bytes
; 0024              DATA OFFSET (3rd REVOLUTION)    - 4 bytes
; 0028              INDEX TIME (4th REVOLUTION)     - 4 bytes
; 002C              TRACK LENGTH (4th REVOLUTION)   - 4 bytes
; 0030              DATA OFFSET (4th REVOLUTION)    - 4 bytes
; 0034              INDEX TIME (5th REVOLUTION)     - 4 bytes
; 0038              TRACK LENGTH (5th REVOLUTION)   - 4 bytes
; 003C              DATA OFFSET (5th REVOLUTION)    - 4 bytes
; .... etc. etc.
;
;
; INDEX TIME = 32 BIT VALUE, TIME IN NANOSECONDS/25ns FOR ONE REVOLUTION
;
; i.e. 0x7A1200 = 8000000, 8000000*25 = 200000000 = 200.00000ms
;
; TRACK DATA = 16 BIT VALUE, TIME IN NANOSECONDS/25ns FOR ONE BIT CELL TIME
;
; i.e. 0x00DA = 218, 218*25 = 5450ns = 5.450us
;
; Special note when a bit cell time is 0x0000.  This occurs when there is no flux transition
; for at least 65536*25ns.  This means that the time overflowed.  When this occurs, the next
; bit cell time will be added to 65536 and that will be the total bit cell time.  If there
; are more than one 0x0000 entry, then 65536 is added for each entry until a non-0x0000 entry
; is found.  You will see 0x0000 when encountering 'strongbits' (no flux area) type of
; protection schemes.
;
; i.e. 0x0000, 0x0000, 0x7FFF = 65536 + 65536 + 32767 = 163839*25 = 4095975ns
;
; The number of bitcells only increases by the number of entries, and not affected by the
; overall time.  So, in above example even though the time could be what thousands of bitcells
; times would normally be, the number of bitcells would only be 3 entries.


; ## TRACK DATA HEADER DEFINES ##

TDH_ID = 0x00                      ; "TRK" (ASCII CHARS)
TDH_TRACKNUM = 0x03                ; track number
TDH_TABLESTART = 0x04              ; table of entries (3 longwords per revolution stored)
TDH_DURATION = 0x4                 ; duration of track, from index pulse to index pulse (1st revolution)
TDH_LENGTH = 0x08                  ; length of track (1st revolution)
TDH_OFFSET = 0x0C                  ; offset to flux data from start of TDH (1st revolution)
*/

// Header Flags
#define INDEXMARK     0x01
#define DISK_96TPI    0x02
#define DISK_360RPM   0x04
#define DISK_RWENABLE 0x08

#pragma pack(1)

typedef struct scp_header_
{
	uint8_t  sign[3];				// "SCP"
	uint8_t  version;				// Version<<4|Revision
	uint8_t  disk_type;
	uint8_t  number_of_revolution;
	uint8_t  start_track;
	uint8_t  end_track;
	uint8_t  flags;
	uint8_t  bit_cell_width;
	uint8_t  number_of_heads;
	uint8_t  RFU_2;
	uint32_t file_data_checksum;
}scp_header;

typedef struct scp_index_pos_
{
	uint32_t index_time;
	uint32_t track_length;
	uint32_t track_offset;
}scp_index_pos;

typedef struct scp_track_header_
{
	uint8_t  trk_sign[3];				// "TRK"
	uint8_t  track_number;
	scp_index_pos index_position[5];
	uint32_t track_data_checksum;
}scp_track_header;

#pragma pack()
