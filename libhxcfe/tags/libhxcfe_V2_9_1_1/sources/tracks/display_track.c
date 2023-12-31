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
///////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-----------H----H--X----X-----CCCCC----22222----0000-----0000------11----------//
//----------H----H----X-X-----C--------------2---0----0---0----0--1--1-----------//
//---------HHHHHH-----X------C----------22222---0----0---0----0-----1------------//
//--------H----H----X--X----C----------2-------0----0---0----0-----1-------------//
//-------H----H---X-----X---CCCCC-----222222----0000-----0000----1111------------//
//-------------------------------------------------------------------------------//
//----------------------------------------------------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////////
// File : display_track.c
// Contains:
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "types.h"

#include "internal_libhxcfe.h"
#include "tracks/track_generator.h"
#include "libhxcfe.h"

#include "version.h"

#include "crc.h"
#include "std_crc32.h"

#include "floppy_utils.h"

#include "display_track.h"

#include "font.h"

#include "loaders/bmp_loader/bmp_loader.h"
#include "loaders/bmp_loader/bmp_file.h"

#define PI    ((float)  3.141592654f)

uint8_t bitcount_array[]=
{
	0x00,0x01,0x01,0x02,0x01,0x02,0x02,0x03,
	0x01,0x02,0x02,0x03,0x02,0x03,0x03,0x04,
	0x01,0x02,0x02,0x03,0x02,0x03,0x03,0x04,
	0x02,0x03,0x03,0x04,0x03,0x04,0x04,0x05,
	0x01,0x02,0x02,0x03,0x02,0x03,0x03,0x04,
	0x02,0x03,0x03,0x04,0x03,0x04,0x04,0x05,
	0x02,0x03,0x03,0x04,0x03,0x04,0x04,0x05,
	0x03,0x04,0x04,0x05,0x04,0x05,0x05,0x06,
	0x01,0x02,0x02,0x03,0x02,0x03,0x03,0x04,
	0x02,0x03,0x03,0x04,0x03,0x04,0x04,0x05,
	0x02,0x03,0x03,0x04,0x03,0x04,0x04,0x05,
	0x03,0x04,0x04,0x05,0x04,0x05,0x05,0x06,
	0x02,0x03,0x03,0x04,0x03,0x04,0x04,0x05,
	0x03,0x04,0x04,0x05,0x04,0x05,0x05,0x06,
	0x03,0x04,0x04,0x05,0x04,0x05,0x05,0x06,
	0x04,0x05,0x05,0x06,0x05,0x06,0x06,0x07,
	0x01,0x02,0x02,0x03,0x02,0x03,0x03,0x04,
	0x02,0x03,0x03,0x04,0x03,0x04,0x04,0x05,
	0x02,0x03,0x03,0x04,0x03,0x04,0x04,0x05,
	0x03,0x04,0x04,0x05,0x04,0x05,0x05,0x06,
	0x02,0x03,0x03,0x04,0x03,0x04,0x04,0x05,
	0x03,0x04,0x04,0x05,0x04,0x05,0x05,0x06,
	0x03,0x04,0x04,0x05,0x04,0x05,0x05,0x06,
	0x04,0x05,0x05,0x06,0x05,0x06,0x06,0x07,
	0x02,0x03,0x03,0x04,0x03,0x04,0x04,0x05,
	0x03,0x04,0x04,0x05,0x04,0x05,0x05,0x06,
	0x03,0x04,0x04,0x05,0x04,0x05,0x05,0x06,
	0x04,0x05,0x05,0x06,0x05,0x06,0x06,0x07,
	0x03,0x04,0x04,0x05,0x04,0x05,0x05,0x06,
	0x04,0x05,0x05,0x06,0x05,0x06,0x06,0x07,
	0x04,0x05,0x05,0x06,0x05,0x06,0x06,0x07,
	0x05,0x06,0x06,0x07,0x06,0x07,0x07,0x08
};

int dummy_graph_progress(unsigned int current,unsigned int total,void * td,void * user)
{
    return 0;
}

HXCFE_TD * hxcfe_td_init(HXCFE* floppycontext,uint32_t xsize,uint32_t ysize)
{
	HXCFE_TD * td;

	td = malloc(sizeof(HXCFE_TD));
	if(td)
	{
		memset(td,0,sizeof(HXCFE_TD));
		td->hxcfe = floppycontext;
		td->xsize = xsize;
		td->ysize = ysize;

		td->x_us=200*1000;
		td->y_us=64;

		td->x_start_us=0;

		td->framebuffer = malloc(td->xsize*td->ysize*sizeof(unsigned int));

		td->hxc_setprogress = dummy_graph_progress;
		td->progress_userdata = 0;

		if(td->framebuffer)
		{
			memset(td->framebuffer,0,td->xsize*td->ysize*sizeof(unsigned int));
		}
		else
		{
			free(td);
			td = 0;
		}
	}
	return td;
}

int32_t hxcfe_td_setProgressCallback( HXCFE_TD *td, HXCFE_TDPROGRESSOUT_FUNC progress_func, void * userdata )
{
	if(td)
	{
		if(progress_func)
		{
			td->progress_userdata = userdata;
			td->hxc_setprogress = progress_func;
		}
	}
	return 0;
}

void hxcfe_td_setparams( HXCFE_TD *td, uint32_t x_us, uint32_t y_us, uint32_t x_start_us )
{
	if(td)
	{
		td->x_us=x_us;
		td->y_us=y_us;

		td->x_start_us=x_start_us;
	}
}

#pragma pack(1)
typedef struct s_col_
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t spare;
}s_col;
#pragma pack()


struct s_sectorlist_ * getlastelement(struct s_sectorlist_ * element)
{
	while(element->next_element)
	{
		element=element->next_element;
	}

	return element;
}

struct s_sectorlist_ * addelement(struct s_sectorlist_ * element,void *object)
{
	while(element->next_element)
	{
		element=element->next_element;
	}

	element->next_element = malloc(sizeof(struct s_sectorlist_));
	if(element->next_element)
	{
		memset(element->next_element,0,sizeof(struct s_sectorlist_));
	}

	return element->next_element;
}

void freelist(struct s_sectorlist_ * element)
{
	struct s_sectorlist_ * nextelement;

	nextelement = element->next_element;

	if(element)
		free(element);

	if(nextelement)
		freelist(nextelement);

	return;
}

double getOffsetTiming(HXCFE_SIDE *currentside,int offset,double timingoffset,int start)
{
	int i,j,totaloffset;
	uint32_t bitrate;
	double timinginc;
	uint32_t tracklen;
	uint32_t oldbitrate;

	tracklen = currentside->tracklen;

	if( offset >= start )
	{
		totaloffset = offset;
	}
	else
	{
		totaloffset = start + ((tracklen - start) + offset);
	}

	i = start;
	j = start%tracklen;

	if(currentside->bitrate==VARIABLEBITRATE)
	{
		if( i < totaloffset )
		{
			bitrate = currentside->timingbuffer[j>>3];
			oldbitrate = bitrate;
			timinginc = ((double)(500000)/(double)bitrate);

			while( i < totaloffset )
			{
				if(!(j&7))
				{
					bitrate = currentside->timingbuffer[j>>3];

					if( oldbitrate != bitrate)
					{
						timinginc = ((double)(500000)/(double)bitrate);
						oldbitrate = bitrate;
					}
				}

				timingoffset = timingoffset + timinginc;
				i++;
				j = ( j + 1 ) % tracklen;
			}
		}
	}
	else
	{
		timingoffset = timingoffset + ( ((double)(500000)/(double)currentside->bitrate) * ( totaloffset - start ) );
	}

	return timingoffset;
}

void putchar8x8(HXCFE_TD *td,int x_pos,int y_pos,unsigned char c,uint32_t color,uint32_t bgcolor,int vertical,int transparent)
{
	int charoffset;
	int xpos,ypos;
	int i,j;
	s_col * col;

	charoffset=(c&0x7F)*8;

	for(j=0;j<8;j++) // y
	{
		for(i=0;i<8;i++) // x
		{
			if(vertical)
			{
				xpos=(x_pos + j);
				ypos=(y_pos + (8-i));
			}
			else
			{
				xpos=(x_pos + i);
				ypos=(y_pos + j);
			}

			if(xpos>=0 && xpos<td->xsize)
			{
				if(ypos>=0 && ypos<td->ysize)
				{
					col=(s_col *)&td->framebuffer[(td->xsize*ypos) + xpos];

					if( ( font8x8[charoffset + j ] & (0x01<<(i&7)) ) )
					{
						if(!color)
						{
							col->blue=(unsigned char)(col->blue/2);
							col->red=(unsigned char)(col->red/2);
							col->green=(unsigned char)(col->green/2);
						}
						else
						{
							col->blue=(unsigned char)((color>>16)&0xFF);
							col->red=(unsigned char)(color&0xFF);
							col->green=(unsigned char)((color>>8)&0xFF);
						}
					}
					else
					{
						if(!transparent)
						{
							col->blue=(unsigned char)((bgcolor>>16)&0xFF);
							col->red=(unsigned char)(bgcolor&0xFF);
							col->green=(unsigned char)((bgcolor>>8)&0xFF);
						}
					}
				}
			}
		}
	}
}

void putstring8x8(HXCFE_TD *td,int x_pos,int y_pos,char * str,uint32_t color,uint32_t bgcolor,int vertical,int transparent)
{
	int i;

	i=0;

	if( td && str )
	{
		while(str[i])
		{
			if(vertical)
			{
				putchar8x8(td,x_pos,y_pos-(i*8),str[i],color,bgcolor,vertical,transparent);
			}
			else
			{
				putchar8x8(td,x_pos+(i*8),y_pos,str[i],color,bgcolor,vertical,transparent);
			}
			i++;
		}
	}
}

s_sectorlist * display_sectors(HXCFE_TD *td,HXCFE_FLOPPY * floppydisk,int track,int side,double timingoffset_offset, int TRACKTYPE)
{
	int tracksize;
	int i,j,old_i;
	char tempstr[512];
	char tempstr2[32];
	double timingoffset;
	double timingoffset2;
	double timingoffset3;
	double xresstep;
	int xpos_startheader,xpos_endsector,xpos_startdata;
	int xpos_tmp;
	int endfill,loop;
	s_col * col;
	HXCFE_SECTORACCESS* ss;
	HXCFE_SECTCFG* sc;
	HXCFE_SIDE * currentside;
	s_sectorlist * sl,*oldsl;

	xresstep = (double)td->x_us/(double)td->xsize;

	old_i=0;
	timingoffset=0;//timingoffset_offset;
	loop=0;
	endfill=0;

	sl=td->sl;
	oldsl=0;

	currentside=floppydisk->tracks[track]->sides[side];
	tracksize=currentside->tracklen;
	do
	{

		timingoffset = ( getOffsetTiming(currentside,tracksize,0,0) * loop );
		xpos_tmp = (int)( ( timingoffset - timingoffset_offset ) / xresstep );
		if( xpos_tmp < td->xsize )
		{

			old_i=0;

			ss=hxcfe_initSectorAccess(td->hxcfe,floppydisk);
			if(ss)
			{
				endfill=0;
				do
				{
					sc=hxcfe_getNextSector(ss,track,side,TRACKTYPE);
					if(sc)
					{

						oldsl = sl;
						sl=malloc(sizeof(s_sectorlist));
						memset(sl,0,sizeof(s_sectorlist));

						sl->track = track;
						sl->side = side;

						sl->next_element = oldsl;

						timingoffset = getOffsetTiming(currentside,sc->startsectorindex,timingoffset,old_i);
						old_i = sc->startsectorindex;
						xpos_startheader = (int)( ( timingoffset - timingoffset_offset ) / xresstep );

						if(sc->endsectorindex<sc->startsectorindex)
							timingoffset2 = getOffsetTiming(currentside,sc->startsectorindex+ ((tracksize - sc->startsectorindex) + sc->endsectorindex),timingoffset,old_i);
						else
							timingoffset2 = getOffsetTiming(currentside,sc->endsectorindex,timingoffset,old_i);

						if(sc->startdataindex<sc->startsectorindex)
						{
							timingoffset3 = getOffsetTiming(currentside,sc->startsectorindex+ ((tracksize - sc->startsectorindex) + sc->startdataindex),timingoffset,old_i);
						}
						else
							timingoffset3 = getOffsetTiming(currentside,sc->startdataindex,timingoffset,old_i);

						xpos_startdata = (int)( ( timingoffset3 - timingoffset_offset ) / xresstep );

						xpos_endsector = (int)( ( timingoffset2 - timingoffset_offset ) / xresstep );

						sl->x_pos1 = xpos_startheader;
						sl->y_pos1 = 50;
						sl->x_pos2 = xpos_endsector;
						sl->y_pos2 = td->ysize-10;

						sl->sectorconfig=sc;

						// Main Header block
						for(i= xpos_startheader;i<xpos_startdata;i++)
						{
							if(i>=0)
							{
								if( (i<td->xsize) && i>=0)
								{
									if( sc->use_alternate_header_crc )
									{
										// Bad header block -> Red
										for(j=50;j<(td->ysize-10);j++)
										{
											col=(s_col *)&td->framebuffer[(td->xsize*j) + i];
											col->blue=(unsigned char)((3*col->blue)/4);
											col->green=(unsigned char)((3*col->green)/4);
										}
									}
									else
									{
										// Sector ok -> Green
										for(j=50;j<(td->ysize-10);j++)
										{
											col=(s_col *)&td->framebuffer[(td->xsize*j) + i];
											col->blue=(unsigned char)((3*col->blue)/4);
											col->red=(unsigned char)((3*col->red)/4);
										}
									}
								}
								else
								{
									endfill=1;
								}
							}
						}

						// Main Data block
						for(i = xpos_startdata;i < xpos_endsector;i++)
						{
							if(i>=0)
							{
								if( (i<td->xsize) && i>=0)
								{
									if(sc->startdataindex == sc->startsectorindex)
									{
										// Unknow size (no header) : blue
										for(j=50;j<(td->ysize-10);j++)
										{
											col=(s_col *)&td->framebuffer[(td->xsize*j) + i];
											col->green=(unsigned char)((3*col->green)/4);
											col->red=(unsigned char)((3*col->red)/4);
										}
									}
									else
									{
										if(sc->use_alternate_data_crc)
										{
											// CRC error -> Red
											for(j=50;j<(td->ysize-10);j++)
											{
												col=(s_col *)&td->framebuffer[(td->xsize*j) + i];
												col->blue=(unsigned char)((3*col->blue)/4);
												col->green=(unsigned char)((3*col->green)/4);
											}
										}
										else
										{
											// Sector ok -> Green
											for(j=50;j<(td->ysize-10);j++)
											{
												col=(s_col *)&td->framebuffer[(td->xsize*j) + i];
												if(!sc->fill_byte_used)
												{
													col->blue=(unsigned char)((3*col->blue)/6);
													col->red=(unsigned char)((3*col->red)/6);
												}
												else
												{
													col->blue=(unsigned char)((3*col->blue)/4);
													col->red=(unsigned char)((3*col->red)/4);
												}
											}
										}
									}
								}
								else
								{
									endfill=1;
								}
							}
						}

						// Left Line
						for(j=50;j<(td->ysize-10);j++)
						{
							if( (xpos_startheader < td->xsize) && xpos_startheader>=0)
							{
								if(j&8)
								{
									if(sc->use_alternate_data_crc)
									{
										col=(s_col *)&td->framebuffer[(td->xsize*j) + xpos_startheader];
										col->blue=(unsigned char)((3*col->blue)/8);
										col->green=(unsigned char)((3*col->green)/8);
									}
									else
									{
										col=(s_col *)&td->framebuffer[(td->xsize*j) + xpos_startheader];
										col->blue=(unsigned char)((3*col->blue)/8);
										col->red=(unsigned char)((3*col->red)/8);
									}
								}
							}
						}


						if(sc->startdataindex != sc->startsectorindex)
						{
							// Data Line
							for(j=50;j<(td->ysize-10);j++)
							{
								if( (xpos_startdata<td->xsize) && xpos_startdata >=0)
								{
									if(!(j&7))
									{
										if(sc->use_alternate_data_crc)
										{
											col=(s_col *)&td->framebuffer[(td->xsize*j) + xpos_startdata];
											col->blue=(unsigned char)((3*col->blue)/16);
											col->green=(unsigned char)((3*col->green)/16);
											col->red=(unsigned char)((3*col->red)/4);
										}
										else
										{
											col=(s_col *)&td->framebuffer[(td->xsize*j) + xpos_startdata];
											col->blue=(unsigned char)((3*col->blue)/16);
											col->green=(unsigned char)((3*col->green)/4);
											col->red=(unsigned char)((3*col->red)/16);
										}
									}
								}
							}

							sprintf(tempstr,"---- %.3d Bytes",sc->sectorsize);

							switch(sc->trackencoding)
							{
								case ISOFORMAT_SD:
									if(sc->startdataindex != sc->endsectorindex)
										sprintf(tempstr,"FM   %.3dB DM:%.2Xh",sc->sectorsize,sc->alternate_datamark);
									else
										sprintf(tempstr,"FM   %.3dB DM: ??",sc->sectorsize);
									break;
								case ISOFORMAT_DD:
									if(sc->startdataindex != sc->endsectorindex)
										sprintf(tempstr,"MFM  %.3dB DM:%.2Xh",sc->sectorsize,sc->alternate_datamark);
									else
										sprintf(tempstr,"MFM  %.3dB DM: ??",sc->sectorsize);
									break;
								case AMIGAFORMAT_DD:
									sprintf(tempstr,"AMFM %.3dB ",sc->sectorsize);
									break;
								case TYCOMFORMAT_SD:
									sprintf(tempstr,"TYFM %.3dB ",sc->sectorsize);
								break;
								case MEMBRAINFORMAT_DD:
									sprintf(tempstr,"MEMBRAIN %.3dB DM:%.2Xh",sc->sectorsize,sc->alternate_datamark);
								break;
								case EMUFORMAT_SD:
									sprintf(tempstr,"E-mu %.3dB ",sc->sectorsize);
								break;
								case APPLE2_GCR5A3:
									sprintf(tempstr,"Apple II 5A3 %.3dB ",sc->sectorsize);
								break;
								case APPLE2_GCR6A2:
									sprintf(tempstr,"Apple II 6A2 %.3dB ",sc->sectorsize);
								break;
								case ARBURG_DAT:
									sprintf(tempstr,"Arburg DATA %.3dB ",sc->sectorsize);
								break;
								case ARBURG_SYS:
									sprintf(tempstr,"Arburg SYSTEM %.3dB ",sc->sectorsize);
								break;
								case AED6200P_DD:
									if(sc->startdataindex != sc->endsectorindex)
										sprintf(tempstr,"AED 6200P %.3dB DM:%.2Xh",sc->sectorsize,sc->alternate_datamark);
									else
										sprintf(tempstr,"AED 6200P %.3dB DM: ??",sc->sectorsize);
									break;
								break;

							}

							if(sc->fill_byte_used)
								sprintf(tempstr2," F:%.2Xh",sc->fill_byte);
							else
								strcpy(tempstr2,"");

							strcat(tempstr,tempstr2);

							putstring8x8(td,xpos_startheader,225,tempstr,0x000,0x000,1,1);

							if(sc->startdataindex != sc->endsectorindex)
								sprintf(tempstr,"T:%.2d H:%d S:%.3d CRC:%.4X",sc->cylinder,sc->head,sc->sector,sc->data_crc);
							else
								sprintf(tempstr,"T:%.2d H:%d S:%.3d NO DATA?",sc->cylinder,sc->head,sc->sector);
							putstring8x8(td,xpos_startheader+8,225,tempstr,0x000,0x000,1,1);
						}
						else
						{
							sprintf(tempstr,"----");
							switch(sc->trackencoding)
							{
								case ISOFORMAT_SD:
									sprintf(tempstr,"FM   DATA ? DM:%.2Xh",sc->alternate_datamark);
									break;
								case ISOFORMAT_DD:
									sprintf(tempstr,"MFM  DATA ? DM:%.2Xh",sc->alternate_datamark);
									break;
								case AMIGAFORMAT_DD:
									sprintf(tempstr,"AMFM DATA ?");
									break;

								case TYCOMFORMAT_SD:
									sprintf(tempstr,"TYFM DATA ? DM:%.2Xh",sc->alternate_datamark);
								break;

								case MEMBRAINFORMAT_DD:
									sprintf(tempstr,"MEMBRAIN DATA ? DM:%.2Xh",sc->alternate_datamark);
								break;
								case EMUFORMAT_SD:
									sprintf(tempstr,"E-mu Data ?");
								break;
								case APPLE2_GCR5A3:
									sprintf(tempstr,"Apple II 5A3 Data ?");
								break;
								case APPLE2_GCR6A2:
									sprintf(tempstr,"Apple II 6A2 Data ?");
								break;
								case ARBURG_DAT:
									sprintf(tempstr,"Arburg DATA Data ?");
								break;
								case ARBURG_SYS:
									sprintf(tempstr,"Arburg SYSTEM Data ?");
								break;
								case AED6200P_DD:
									sprintf(tempstr,"AED 6200P Data ?");
								break;
							}
							putstring8x8(td,xpos_startheader,225,tempstr,0x000,0x000,1,1);
						}

						// Right Line
						for(j=50;j<(td->ysize-10);j++)
						{
							if( ((xpos_endsector-1)<td->xsize) && xpos_endsector>=0 )
							{
								if(!(j&8))
								{
									if(sc->use_alternate_data_crc)
									{
										col=(s_col *)&td->framebuffer[(td->xsize*j) + (xpos_endsector-1)];
										col->blue=(unsigned char)((3*col->blue)/8);
										col->green=(unsigned char)((3*col->green)/8);
									}
									else
									{
										col=(s_col *)&td->framebuffer[(td->xsize*j) + (xpos_endsector-1)];
										col->blue = (unsigned char)((3*col->blue)/8);
										col->red = (unsigned char)((3*col->red)/8);
									}
								}
							}
						}
					}
				}while(sc);

				hxcfe_deinitSectorAccess(ss);

				loop++;
			}
		}
		else
			endfill=1;

		if(loop>100)
			endfill=1;
	}while(!endfill);

	td->sl=sl;

	return sl;
}

void hxcfe_td_activate_analyzer( HXCFE_TD *td, int32_t TRACKTYPE, int32_t enable )
{
	if(td && TRACKTYPE<32)
	{
		if(enable)
			td->enabledtrackmode = td->enabledtrackmode | (0x00000001 << TRACKTYPE);
		else
			td->enabledtrackmode = td->enabledtrackmode & ( ~(0x00000001 << TRACKTYPE) );
	}
}

void hxcfe_td_draw_track( HXCFE_TD *td, HXCFE_FLOPPY * floppydisk, int32_t track, int32_t side )
{
	int tracksize;
	int i,j,old_i;

	double timingoffset_offset;
	int	   buffer_offset;

	double timingoffset;
	double timingoffset2;
	int interbit;
	int bitrate;
	int xpos,ypos,old_xpos;
	int endfill,loopcnt;
	double xresstep;
	s_sectorlist * sl,*oldsl;
	s_pulseslist * pl,*oldpl;
	HXCFE_SIDE * currentside;
	s_col * col;

	char tmp_str[32];

	sl=td->sl;
	while(sl)
	{

		oldsl = sl->next_element;
		//sl = sl->next_element;

		hxcfe_freeSectorConfig( 0, sl->sectorconfig );

		free(sl);

		sl = oldsl;
	}
	td->sl=0;

	pl=td->pl;
	while(pl)
	{

		oldpl = pl->next_element;

		free(pl);

		pl = oldpl;
	}
	td->pl=0;

	if(track>=floppydisk->floppyNumberOfTrack) track = floppydisk->floppyNumberOfTrack - 1;
	if(track<0) track = 0;

	if(side>=floppydisk->floppyNumberOfSide) side = floppydisk->floppyNumberOfSide - 1;
	if(side<0) side = 0;


	if(!floppydisk->floppyNumberOfTrack || !floppydisk->floppyNumberOfSide)
	{
		memset(td->framebuffer,0xCC,td->xsize*td->ysize*sizeof(unsigned int));
		return;
	}

	currentside=floppydisk->tracks[track]->sides[side];

	tracksize=currentside->tracklen;

	old_i=0;
	i=0;

	memset(td->framebuffer,0,td->xsize*td->ysize*sizeof(unsigned int));

	timingoffset = ( getOffsetTiming(currentside,tracksize,0,0));

	timingoffset2=( timingoffset * td->x_start_us ) / (100 * 1000);
	timingoffset=0;
	while((i<tracksize) && timingoffset2>timingoffset)
	{
		timingoffset=getOffsetTiming(currentside,i,timingoffset,old_i);
		old_i=i;
		i++;
	};

	buffer_offset=i;
	timingoffset_offset=timingoffset;

	bitrate = currentside->bitrate;

	tracksize=currentside->tracklen;
	timingoffset=0;
	interbit=0;
	i=buffer_offset;

	xresstep = 0;
	if(td->xsize)
		xresstep = (double)td->x_us/(double)td->xsize;
	endfill=0;

	if(!xresstep)
		return;


	//////////////////////////////////////////
	// Scatter drawing
	do
	{
		do
		{

			if( currentside->databuffer[i>>3] & (0x80>>(i&7)) )
			{
				if(currentside->bitrate == VARIABLEBITRATE)
					bitrate = currentside->timingbuffer[i>>3];

				xpos= (int)( timingoffset / (xresstep) );
				ypos= td->ysize - (int)( ( (double) ((interbit+1) * 500000) / (double) bitrate ) / (double)((double)td->y_us/(double)td->ysize));

				if(td->x_us>250)
				{
					// Scather gatter display mode
					if( (xpos<td->xsize) && (ypos<td->ysize) && ypos>=0 )
					{
						td->framebuffer[(td->xsize*ypos) + xpos]++;

						ypos--;
						if( (ypos<td->ysize) && ypos>=0 )
						{
							td->framebuffer[(td->xsize*ypos) + xpos]++;
						}
						ypos=ypos+2;
						if( (ypos<td->ysize) && ypos>=0 )
						{
							td->framebuffer[(td->xsize*ypos) + xpos]++;
						}
					}
				}
				else
				{
					// Pulses display mode
					if( (xpos<td->xsize) )
					{

						for(ypos= td->ysize - 40 ; ypos > (td->ysize - 250) ; ypos--)
						{
							td->framebuffer[(td->xsize*ypos) + xpos]=255;
						}
					}
				}

				if(xpos>td->xsize)
					endfill=1;

				interbit=0;
			}
			else
			{
				interbit++;
			}

			if(currentside->bitrate==VARIABLEBITRATE)
				bitrate = currentside->timingbuffer[i>>3];

			timingoffset=timingoffset + ((double)(500000)/(double)bitrate);

			i++;

		}while(i<tracksize && !endfill);

		xpos= (int)( timingoffset / xresstep );
		if(xpos>td->xsize)
			endfill=1;

		i=0;

	}while(!endfill);

	//////////////////////////////////////////
	// Color inverting
	for(i=0;i<td->xsize*td->ysize;i++)
	{
		col=(s_col *)&td->framebuffer[i];

		col->spare=0;
		if(!col->red && !col->green && !col->blue)
		{
			col->red=255;
			col->green=255;
			col->blue=255;
		}
		else
		{
			col->green = (unsigned char)( 255 - col->red );
			col->blue = (unsigned char)( 255 - col->red );
			col->red = (unsigned char)( 255 - col->red );

			col->green = (unsigned char)(col->green / 2);
			col->blue = (unsigned char)(col->blue / 2);
			col->red = (unsigned char)(col->red / 2);
		}
	}

	//////////////////////////////////////////
	// Flakey bits drawing
	if(currentside->flakybitsbuffer)
	{
		tracksize = currentside->tracklen;;
		timingoffset = 0;
		i = buffer_offset;
		old_i = buffer_offset;

		loopcnt = 0;
		endfill = 0;
		do
		{
			do
			{

				timingoffset = getOffsetTiming(currentside,i,timingoffset,old_i);
				old_i = i;
				xpos = (int) ( timingoffset / xresstep );
				if( ( xpos >= td->xsize ) )
				{
					endfill = 1;
				}

				if( currentside->flakybitsbuffer[i>>3] & (0x80>>(i&7)) )
				{
					if( (xpos<td->xsize) )
					{
						for(j=30;j<50;j++)
						{
							col=(s_col *)&td->framebuffer[(td->xsize*j) + xpos];
							col->blue=0;
							col->green=0;
							col->red=255;
						}
					}
				}

				i++;

			}while(i<tracksize && !endfill);

			loopcnt++;

			if( i == tracksize && loopcnt>1000)
				endfill = 1;

			i=0;
			old_i=0;
		}while(!endfill);

	}

	//////////////////////////////////////////
	// Index drawing
	tracksize=currentside->tracklen;
	old_i=buffer_offset;
	i=buffer_offset;
	timingoffset=0;
	loopcnt = 0;
	endfill=0;
	do
	{
		do
		{
			timingoffset=getOffsetTiming(currentside,i,timingoffset,old_i);
			old_i=i;
			xpos= (int)( timingoffset / xresstep );

			if( (xpos<td->xsize) )
			{
				if( currentside->indexbuffer[i>>3] )
				{
					for(j=10;j<12;j++)
					{
						col=(s_col *)&td->framebuffer[(td->xsize*j) + xpos];
						col->blue=255;
						col->green=0;
						col->red=0;
					}
				}
				else
				{
					for(j=25;j<27;j++)
					{
						col=(s_col *)&td->framebuffer[(td->xsize*j) + xpos];
						col->blue=255;
						col->green=0;
						col->red=0;
					}
				}
			}
			else
			{
				endfill=1;
			};
			i++;
		}while(i<tracksize && !endfill);

		loopcnt++;

		if( i == tracksize && loopcnt>1000)
			endfill = 1;

		old_i=0;
		i=0;

	}while(!endfill);

	//////////////////////////////////////////
	// cell drawing

	if(!(td->x_us>250))
	{
		// Only possible in pulses display mode
		tracksize=currentside->tracklen;
		old_i=buffer_offset;
		i=buffer_offset;
		timingoffset=0;
		endfill=0;
		do
		{
			do
			{
				timingoffset=getOffsetTiming(currentside,i,timingoffset,old_i);
				timingoffset2=getOffsetTiming(currentside,i+1,timingoffset,i);

				old_i=i;
				xpos = (int)( timingoffset / xresstep ) - (int)(( timingoffset2 - timingoffset) / (xresstep *2) );

				if(xpos>=0 && (xpos<td->xsize) )
				{
					for(ypos= td->ysize - 40 ; ypos > (td->ysize - 50) ; ypos--)
					{
						col=(s_col *)&td->framebuffer[(td->xsize*ypos) + xpos];
						col->blue=0;
						col->green=155;
						col->red=0;
					}
					// Add the pulse to the pulses list.
					oldpl = pl;
					pl = malloc(sizeof(s_pulseslist));
					if(pl)
					{
						memset(pl,0,sizeof(s_pulseslist));
						pl->track = track;
						pl->side = side;
						pl->pulse_number = i;
						pl->x_pos1 = xpos;
						pl->x_pos2 = xpos + (int)(( timingoffset2 - timingoffset) / (xresstep) );
						pl->next_element = oldpl;
					}
				}
				else
				{
					if(xpos >= 0 )
						endfill = 1;
				};
				i++;
			}while(i<tracksize && !endfill);

			old_i=0;
			i=0;

		}while(!endfill);

		td->pl = pl;

		ypos = td->ysize - 40;
		for(xpos = 0;xpos < td->xsize ; xpos++)
		{
			col=(s_col *)&td->framebuffer[(td->xsize*ypos) + xpos];
			col->blue=0;
			col->green=155;
			col->red=0;
		}
	}
	else
	{
		// Pulse list generation
		tracksize = currentside->tracklen;
		old_i = buffer_offset;
		i = buffer_offset;
		timingoffset=0;

		loopcnt = 0;
		endfill=0;
		old_xpos = -1;
		do
		{
			do
			{
				timingoffset=getOffsetTiming(currentside,i,timingoffset,old_i);
				timingoffset2=getOffsetTiming(currentside,i+1,timingoffset,i);

				old_i=i;
				xpos = (int)( timingoffset / xresstep ) - (int)(( timingoffset2 - timingoffset) / (xresstep *2) );

				if(xpos>=0 && (xpos<td->xsize) && ( old_xpos != xpos ) )
				{
					// Add the pulse to the pulses list.
					oldpl = pl;
					pl = malloc(sizeof(s_pulseslist));
					if(pl)
					{
						memset(pl,0,sizeof(s_pulseslist));
						pl->track = track;
						pl->side = side;
						pl->pulse_number = i;
						pl->x_pos1 = xpos;
						pl->x_pos2 = xpos + 1;//(int)(( timingoffset2 - timingoffset) / (xresstep) );
						pl->next_element = oldpl;
						old_xpos = xpos;
					}
				}
				else
				{
					if(xpos >= 0 && ( old_xpos != xpos ) )
						endfill = 1;
				};
				i++;
			}while(i<tracksize && !endfill);

			loopcnt++;

			if( i == tracksize && loopcnt>1000)
				endfill = 1;

			old_i=0;
			i=0;


		}while(!endfill);

		td->pl = pl;
	}

	//////////////////////////////////////////
	// Sector drawing
	for(i=0;i<32;i++)
	{
		if(td->enabledtrackmode & (0x00000001<<i) )
		{
			display_sectors(td,floppydisk,track,side,timingoffset_offset,i);
		}
	}

	sprintf(tmp_str,"T:%.3d S:%.1d",track,side);
	putstring8x8(td,1,1,tmp_str,0x000,0x000,0,1);

}

void hxcfe_td_draw_stream_track( HXCFE_TD *td, HXCFE_TRKSTREAM* track_stream )
{
	int i,j;
	int xpos,ypos;
	char tmp_str[64];
	uint32_t total_offset,cur_ticks,curcol;

	memset(td->framebuffer,0,td->xsize*td->ysize);
	//////////////////////////////////////////
	// Scatter drawing
	total_offset = 0;
	for (i = 0; i < (int)track_stream->nb_of_pulses; i++)
	{
		cur_ticks = track_stream->track_dump[i];
		total_offset += cur_ticks;

		xpos = (int)( ((double)total_offset / (double) (TICKFREQ / 1000000)) / ((double)((double)td->x_us/(double)td->xsize)) );

		ypos = td->ysize - (int)( (double)( (double)cur_ticks / (double) (TICKFREQ / 1000000) )/ (double)((double)td->y_us/(double)td->ysize));

		if ( ( xpos < td->xsize ) && ( ( ypos < td->ysize ) && ( ypos >= 0 ) ) )
		{
			td->framebuffer[(td->xsize*ypos) + xpos]++;
		}
	}

	for(ypos = 0; ypos < td->ysize; ypos++)
	{
		for(xpos = 0; xpos < td->xsize; xpos++)
		{
			curcol = td->framebuffer[(td->xsize*ypos) + xpos];

			curcol = curcol * 32;

			if(curcol>255)
			{
				curcol = 255;
			}

			curcol = curcol<<8 | curcol << 16 | curcol;

			td->framebuffer[(td->xsize*ypos) + xpos] = curcol;
		}
	}

	//////////////////////////////////////////
	// Draw indexes
	for(i=0;i < (int)track_stream->nb_of_index;i++)
	{
		j = 0;
		total_offset = 0;
		while( ( j < (int)track_stream->index_evt_tab[i].dump_offset ) && ( j < (int)track_stream->nb_of_pulses ) )
		{
			j++;
			total_offset += track_stream->track_dump[j];
		}

		xpos = (int)( ((double)total_offset / (double) (TICKFREQ / 1000000)) / ((double)((double)td->x_us/(double)td->xsize)) );
		ypos = 0;

		if ( ( xpos < td->xsize ) && ( ypos < td->ysize ) )
		{
			for(ypos=0;ypos<td->ysize;ypos++)
			{
				if(!td->framebuffer[(td->xsize*(ypos)) + xpos])
				{
					td->framebuffer[(td->xsize*(ypos)) + xpos] = 0x00FF00;
				}
			}
		}
	}

	// Print pixel density
	sprintf(tmp_str,"xres: %f us/pix",((double)((double)td->x_us/(double)td->xsize)));
	putstring8x8(td,1,1,tmp_str,0xFFFFFF,0x000000,0,0);
	sprintf(tmp_str,"yres: %f us/pix",(double)((double)td->y_us/(double)td->ysize));
	putstring8x8(td,1,1+8,tmp_str,0xFFFFFF,0x000000,0,0);
}

int32_t hxcfe_td_setName( HXCFE_TD *td, char * name )
{
	if( td )
	{
		if(name)
		{
			if(td->name)
				free(td->name);

			td->name = malloc( strlen(name) + 1);
			if(td->name)
			{
				strcpy(td->name,name);
				return HXCFE_NOERROR;
			}

			return HXCFE_INTERNALERROR;
		}
	}

	return HXCFE_NOERROR;
}
s_sectorlist * hxcfe_td_getlastsectorlist(HXCFE_TD *td)
{
	return td->sl;
}

s_pulseslist * hxcfe_td_getlastpulselist(HXCFE_TD *td)
{
	return td->pl;
}

void plot(HXCFE_TD *td,int x,int y,uint32_t color,int op)
{
	unsigned char color_r,color_v,color_b;
	uint32_t rdcolor;

	if(x>=0 && x<td->xsize)
	{
		if(y>=0 && y<td->ysize)
		{
			switch(op)
			{
				case 0:
				default:
					td->framebuffer[(td->xsize*y)+x]=color;
				break;
				case 1:
					rdcolor = td->framebuffer[(td->xsize*y)+x];

					color_r = (unsigned char)(rdcolor & 0xFF);
					color_v = (unsigned char)((rdcolor>>8) & 0xFF);
					color_b = (unsigned char)((rdcolor>>16) & 0xFF);

					color_r = (unsigned char)(color_r * (float)((float)(color&0xFF)/(float)255));
					color_v = (unsigned char)(color_v * (float)((float)((color>>8)&0xFF)/(float)255));
					color_b = (unsigned char)(color_b * (float)((float)((color>>16)&0xFF)/(float)255));

					td->framebuffer[(td->xsize*y)+x] = (color_b<<16) | (color_v<<8) | color_r;

				break;
			}
		}
	}
}

void circle(HXCFE_TD *td,int x_centre,int y_centre,int r,unsigned int color)
{
	int x;
	int y;
	int d;

	x=0;
	y=r;
	d=r-1;

//    8  1
//  7     2
//  6     3
//    5 4

	while(y>=x)
	{
		plot(td, x+x_centre, -y+y_centre  , color,0);   // 1 -
		plot(td, y+x_centre, -x+y_centre  , color,0);   // 2 +
		plot(td, x_centre + y, x+y_centre , color,0);   // 3 -
		plot(td, x_centre + x, y+y_centre , color,0);   // 4 +
		plot(td, -x+x_centre, y+y_centre  , color,0);   // 5 -
		plot(td, -y+x_centre, x+y_centre  , color,0);   // 6 +
		plot(td, -y+x_centre, -x+y_centre  , color,0);  // 7 -
		plot(td, -x+x_centre, -y+y_centre  , color,0);  // 8 +


		if (d >= 2*x)
		{
			d = d - ( 2 * x ) -1;
			x = x+1;

		}
		else
		{
			if(d <= 2*(r-y))
			{
				d = d+2*y-1;
				y = y-1;
			}
			else
			{
				d = d+2*(y-x-1);
				y = y-1;
				x = x+1;
			}
		}
	}
}

void draw_circle (HXCFE_TD *td,uint32_t col,float start_angle,float stop_angle,int xpos,int ypos,int diametre,int op,int thickness)
{
    int x, y,i;
    int length;
    float angle = 0.0;
    float angle_stepsize = (float)0.001;

	length = diametre;

	if(op!=1) thickness++;

	length += thickness;

	i = 0;
	do
	{
		angle = start_angle;
		// go through all angles from 0 to 2 * PI radians
		do //2 * 3.14159)
		{
			// calculate x, y from a vector with known length and angle
			x = (int)((length) * cos (angle));
			y = (int)((length) * sin (angle));

			plot(td, x+xpos, -y+ypos  , col, op);

			angle += angle_stepsize;
		}while (angle < stop_angle );

		length--;
		i++;
	}while(i<(thickness));
}

void draw_density_circle (HXCFE_TD *td,uint32_t col,float start_angle,float stop_angle,int xpos,int ypos,int diametre,int op,int thickness,HXCFE_SIDE * side)
{
	int x, y,i, x_old,y_old;
	int length;
	int old_j,j,k;

	double timingoffset;
	float track_timing,timingoffset2;
	float angle = 0.0;
	float angle_stepsize = (float)0.001;
	int tracksize;
	int prev_offset;
	int bitcount;
	int totalcount;
	uint8_t lum,mask;

	length = diametre;

	if(op!=1) thickness++;

	length += thickness;

	track_timing = (float)getOffsetTiming(side,side->tracklen,0,0);

	tracksize = side->tracklen;

	i = 0;
	do
	{
		timingoffset = 0;
		old_j = 0;
		j = 0;
		prev_offset = 0;

		//bitcount
		angle = start_angle;
		x_old = (int)((length) * cos (angle));
		y_old = (int)((length) * sin (angle));
		// go through all angles from 0 to 2 * PI radians
		do
		{
			// calculate x, y from a vector with known length and angle
			x = (int)((length) * cos (angle));
			y = (int)((length) * sin (angle));

			if( (x_old != x) || (y_old != y) )
			{
				timingoffset2=( track_timing * ( angle / ( stop_angle - start_angle ) ) );

				while((j<tracksize) && timingoffset2>timingoffset)
				{
					timingoffset = getOffsetTiming(side,j,timingoffset,old_j);
					old_j=j;
					j++;
				};

				bitcount = 0;
				totalcount = 0;
				if(j - prev_offset)
				{

					mask = 0xFF >> (prev_offset&7);

					for(k=0;k<(j - prev_offset);k+=8)
					{
						bitcount = bitcount + bitcount_array[(side->databuffer[(prev_offset+k)>>3])&mask];
						totalcount = totalcount + bitcount_array[mask];
						if( (j - prev_offset) - k >= 8)
							mask = 0xFF;
						else
							mask = 0xFF << ( 8 - ( (j - prev_offset) - k ) ) ;
					}
				}

				lum = (uint8_t)((float)col * (float) ((float)bitcount/(float)totalcount) );

				plot(td, x+xpos, -y+ypos  , (uint32_t)( (lum<<16) | ( (lum>>1) <<8) | (lum>>1)), op);

				prev_offset = old_j;
			}

			angle += angle_stepsize;

			x_old = x;
			y_old = y;

		}while (angle < stop_angle );

		length--;
		i++;
	}while(i<(thickness));
}

s_sectorlist * display_sectors_disk(HXCFE_TD *td,HXCFE_FLOPPY * floppydisk,int track,int side,double timingoffset_offset, int TRACKTYPE,int xpos,int ypos,int diam,int thickness,uint32_t * crc32)
{
	int tracksize;
	int old_i;
	float track_timing;
	float startsector_timingoffset;
	float datasector_timingoffset;
	float endsector_timingoffset;
	float timingoffset;
	int color;
	int loop;
	HXCFE_SECTORACCESS* ss;
	HXCFE_SECTCFG* sc;
	HXCFE_SIDE * currentside;
	s_sectorlist * sl,*oldsl;

	old_i=0;
	loop=0;

	sl=td->sl;
	oldsl=0;

	currentside=floppydisk->tracks[track]->sides[side];
	tracksize=currentside->tracklen;

	startsector_timingoffset = 0;

	old_i=0;
	timingoffset = 0;

	track_timing = (float)getOffsetTiming(currentside,tracksize,timingoffset,old_i);

	ss=hxcfe_initSectorAccess(td->hxcfe,floppydisk);
	if(ss)
	{
		do
		{
			sc = hxcfe_getNextSector(ss,track,side,TRACKTYPE);
			if(sc)
			{
				oldsl = sl;
				sl=malloc(sizeof(s_sectorlist));
				if(sl)
				{
					memset(sl,0,sizeof(s_sectorlist));

					sl->track = track;
					sl->side = side;

					sl->next_element = oldsl;

					sl->sectorconfig = sc;

					startsector_timingoffset = (float)getOffsetTiming(currentside,sc->startsectorindex,timingoffset,old_i);

					if(sc->endsectorindex<sc->startsectorindex)
					{
						endsector_timingoffset = (float)getOffsetTiming(currentside,sc->startsectorindex+ ((tracksize - sc->startsectorindex) + sc->endsectorindex),timingoffset,old_i);
					}
					else
					{
						endsector_timingoffset = (float)getOffsetTiming(currentside,sc->endsectorindex,timingoffset,old_i);
					}

					if(sc->startdataindex<sc->startsectorindex)
					{
						datasector_timingoffset = (float)getOffsetTiming(currentside,sc->startsectorindex+ ((tracksize - sc->startsectorindex) + sc->startdataindex),timingoffset,old_i);
					}
					else
					{
						datasector_timingoffset = (float)getOffsetTiming(currentside,sc->startdataindex,timingoffset,old_i);
					}

					///////////////////////////////////////
					// Header Block
					///////////////////////////////////////
					if( sc->use_alternate_header_crc )
					{
 						color = 0xFF0000;
					}
					else
					{
						color = 0x99FF99;
					}

					draw_circle (td,color,(float)((float)2 * PI)*(startsector_timingoffset/track_timing),(float)((float)2 * PI)*(datasector_timingoffset/track_timing),xpos,ypos,diam,0,thickness);

					///////////////////////////////////////
					// Data Block
					///////////////////////////////////////
					if(sc->startdataindex == sc->startsectorindex)
					{
						// CRC error -> Blue
						color = 0xFF0000;
					}
					else
					{
						if(sc->use_alternate_data_crc)
						{
							// CRC error -> Red
							color = 0x008FFA;
						}
						else
						{
							if(!sc->fill_byte_used)
							{
								// Sector ok -> Green
								color = 0x00FF00;
							}
							else
							{
								color = 0x66EE00;
							}

							if(crc32)
							{
								if(sc->input_data && !sc->use_alternate_data_crc)
								{
									*crc32 = std_crc32(*crc32, sc->input_data, sc->sectorsize);
								}
							}
						}
					}

					draw_circle (td,color,(float)((float)2 * PI)*(datasector_timingoffset/track_timing),(float)((float)2 * PI)*(endsector_timingoffset/track_timing),xpos,ypos,diam,0,thickness);

					// Left Line
					draw_circle (td,0xFF6666,(float)((float)2 * PI)*(startsector_timingoffset/track_timing),(float)((float)2 * PI)*(startsector_timingoffset/track_timing),xpos,ypos,diam,0,thickness);

					// Data Line
					draw_circle (td,0x7F6666,(float)((float)2 * PI)*(datasector_timingoffset/track_timing),(float)((float)2 * PI)*(datasector_timingoffset/track_timing),xpos,ypos,diam,0,thickness);

					// Right Line
					draw_circle (td,0xFF6666,(float)((float)2 * PI)*(endsector_timingoffset/track_timing),(float)((float)2 * PI)*(endsector_timingoffset/track_timing),xpos,ypos,diam,0,thickness);

					sl->start_angle = (float)((float)2 * PI)*(startsector_timingoffset/track_timing);
					sl->end_angle = (float)((float)2 * PI)*(endsector_timingoffset/track_timing);

					sl->diameter = diam;
					sl->thickness = thickness;
				}
			}
		}while(sc);

		hxcfe_deinitSectorAccess(ss);
		loop++;
	}


	td->sl=sl;

	return sl;
}

int countSector(s_sectorlist * sl,int side)
{
	int cnt;

	cnt = 0;

	while(sl)
	{
		if(sl->side == side)
		{
			if(sl->sectorconfig)
			{
				cnt++;
			}
		}
		sl = sl->next_element;
	}

	return cnt;
}

int countSize(s_sectorlist * sl,int side)
{
	int size;

	size = 0;

	while(sl)
	{
		if(sl->side == side)
		{
			if(sl->sectorconfig)
			{
				size = size + sl->sectorconfig->sectorsize;
			}
		}
		sl = sl->next_element;
	}

	return size;
}

int countBadSectors(s_sectorlist * sl,int side)
{
	int cnt;

	cnt = 0;

	while(sl)
	{
		if(sl->side == side)
		{
			if(sl->sectorconfig)
			{
				if(!sl->sectorconfig->trackencoding || sl->sectorconfig->use_alternate_data_crc || !sl->sectorconfig->input_data)
				{
					cnt++;
				}
			}
		}
		sl = sl->next_element;
	}

	return cnt;
}

int countTrackType(s_sectorlist * sl,int side,int type)
{
	int cnt;

	cnt = 0;

	while(sl)
	{
		if(sl->side == side)
		{
			if(sl->sectorconfig)
			{
				if(sl->sectorconfig->trackencoding == type)
				{
					cnt++;
				}
			}
		}
		sl = sl->next_element;
	}

	return cnt;
}

typedef struct type_list_
{
	int track_type;
	char *name;
}type_list;


const static type_list track_type_list[]=
{
	{ISOFORMAT_SD,      "ISO FM"},
	{ISOFORMAT_DD,      "ISO MFM"},
	{AMIGAFORMAT_DD,    "Amiga MFM"},
	{TYCOMFORMAT_SD,    "TYFM"},
	{MEMBRAINFORMAT_DD, "MEMBRAIN"},
	{EMUFORMAT_SD,      "E-mu"},
	{APPLE2_GCR5A3,     "Apple II 5A3"},
	{APPLE2_GCR6A2,     "Apple II 6A2"},
	{ARBURG_DAT,        "Arburg DATA"},
	{ARBURG_SYS,        "Arburg SYSTEM"},
	{AED6200P_DD,       "AED 6200P"},
	{0,0}
};

void hxcfe_td_draw_disk(HXCFE_TD *td,HXCFE_FLOPPY * floppydisk)
{
	int tracksize;
	int i;
	int track,side;
	uint32_t crc32;
	HXCFE_SIDE * currentside;
	unsigned int color;
	int y_pos,x_pos_1,x_pos_2,ytypepos;
	float track_ep;
	int xpos;
	char tempstr[512];
	s_sectorlist * sl,*oldsl;

	track_ep = 0;

	crc32 = 0x00000000;
	sl=td->sl;
	while(sl)
	{
		oldsl = sl->next_element;

		hxcfe_freeSectorConfig( 0, sl->sectorconfig );

		free(sl);

		sl = oldsl;
	}
	td->sl=0;

	memset(td->framebuffer,0,td->xsize*td->ysize*sizeof(uint32_t));

	y_pos = td->ysize/2;
	x_pos_1 = td->xsize/4;
	x_pos_2 = td->xsize - (td->xsize/4);

	sprintf(tempstr,"libhxcfe v%s",STR_FILE_VERSION2);
	putstring8x8(td,1,td->ysize - (8 + 1),tempstr,0xAAAAAA,0x000000,0,0);

	if(td->name)
	{
		putstring8x8(td,1,td->ysize - (8*4),td->name,0xAAAAAA,0x000000,1,0);
		putstring8x8(td,(td->xsize/2) + 1,td->ysize - (8*4),td->name,0xAAAAAA,0x000000,1,0);
	}

	color = 0x131313;
	for(i=25;i<(td->ysize-(y_pos));i++)
	{
		circle(td,x_pos_1,y_pos,i,color);
		circle(td,x_pos_2,y_pos,i,color);
	}

	track_ep = (float)( (td->ysize-(y_pos)) - 60 ) /((float) floppydisk->floppyNumberOfTrack+1);
	for(track=0;track<floppydisk->floppyNumberOfTrack;track++)
	{
		td->hxc_setprogress(track*floppydisk->floppyNumberOfSide,floppydisk->floppyNumberOfTrack*floppydisk->floppyNumberOfSide*2,td,td->progress_userdata);

		currentside = floppydisk->tracks[track]->sides[0];
		draw_density_circle (td,0x0000FF,0,(float)((float)((float)2 * PI)),x_pos_1,y_pos,60 + (int)(((floppydisk->floppyNumberOfTrack-track) * track_ep)),0,(int)track_ep,currentside);

		sprintf(tempstr,"Side %d, %d Tracks",0,track);
		putstring8x8(td,1,1,tempstr,0xAAAAAA,0x000000,0,0);

		if(floppydisk->tracks[track]->number_of_side == 2)
		{
			sprintf(tempstr,"Side %d, %d Tracks",1,track);
			putstring8x8(td,td->xsize/2,1,tempstr,0xAAAAAA,0x000000,0,0);

			currentside = floppydisk->tracks[track]->sides[1];
			draw_density_circle (td,0x0000FF,0,(float)((float)((float)2 * PI)),x_pos_2,y_pos,60 + (int)(((floppydisk->floppyNumberOfTrack-track) * track_ep)),0,(int)track_ep,currentside);
		}
	}

	sprintf(tempstr,"Side %d, %d Tracks",0,floppydisk->floppyNumberOfTrack);
	putstring8x8(td,1,1,tempstr,0xAAAAAA,0x000000,0,0);

	if(floppydisk->floppyNumberOfSide == 2)
	{
		sprintf(tempstr,"Side %d, %d Tracks",1,floppydisk->floppyNumberOfTrack);
		putstring8x8(td,td->xsize/2,1,tempstr,0xAAAAAA,0x000000,0,0);
	}

	for(track=0;track<floppydisk->floppyNumberOfTrack;track++)
	{
		for(side=0;side<floppydisk->floppyNumberOfSide;side++)
		{
			td->hxc_setprogress((floppydisk->floppyNumberOfTrack*floppydisk->floppyNumberOfSide) + track * floppydisk->floppyNumberOfSide ,floppydisk->floppyNumberOfTrack*floppydisk->floppyNumberOfSide*2,td,td->progress_userdata);

			currentside = floppydisk->tracks[track]->sides[side];

			tracksize = currentside->tracklen;

			track_ep = (float)( (td->ysize-(y_pos)) - 60 ) /((float) floppydisk->floppyNumberOfTrack+1);

			//////////////////////////////////////////
			// Sector drawing
			for(i=0;i<32;i++)
			{
				if(td->enabledtrackmode & (0x00000001<<i) )
				{
					if(!side)
					{
						display_sectors_disk(td,floppydisk,track,side,0,i,x_pos_1,y_pos,60 + (int)((floppydisk->floppyNumberOfTrack-track) * track_ep),(int)track_ep,&crc32);
					}
					else
					{
						display_sectors_disk(td,floppydisk,track,side,0,i,x_pos_2,y_pos,60 + (int)((floppydisk->floppyNumberOfTrack-track) * track_ep),(int)track_ep,&crc32);
					}
				}
			}

			if(currentside->flakybitsbuffer)
			{
				i=0;
				do
				{
					if( currentside->flakybitsbuffer[i>>3] & (0x80>>(i&7)) )
					{
						xpos= (int)((float)2048 * ((float)i/(float)tracksize));
						if( (xpos<2048))
						{
							if(!side)
								draw_circle (td,0x0000FF,(float)((float)((float)2 * PI)*((float)xpos/(float)2048)),(float)((float)((float)2 * PI)*((float)xpos/(float)2048)),x_pos_1,y_pos,60 + (int)(((floppydisk->floppyNumberOfTrack-track) * track_ep)),0,(int)track_ep);
							else
								draw_circle (td,0x0000FF,(float)((float)((float)2 * PI)*((float)xpos/(float)2048)),(float)((float)((float)2 * PI)*((float)xpos/(float)2048)),x_pos_2,y_pos,60 + (int)(((floppydisk->floppyNumberOfTrack-track) * track_ep)),0,(int)track_ep);
						}
					}

					i++;
				}while(i<tracksize);
			}

			sprintf(tempstr,"%d Sectors,%d bad", countSector(td->sl,0),countBadSectors(td->sl,0));
			putstring8x8(td,1,11,tempstr,0xAAAAAA,0x000000,0,0);

			sprintf(tempstr,"%d Bytes", countSize(td->sl,0));
			putstring8x8(td,1,21,tempstr,0xAAAAAA,0x000000,0,0);

			sprintf(tempstr,"%d Sectors,%d bad", countSector(td->sl,1),countBadSectors(td->sl,1));
			putstring8x8(td,(td->xsize/2)+1,11,tempstr,0xAAAAAA,0x000000,0,0);

			sprintf(tempstr,"%d Bytes", countSize(td->sl,1));
			putstring8x8(td,(td->xsize/2)+1,21,tempstr,0xAAAAAA,0x000000,0,0);

			sprintf(tempstr,"CRC32: 0x%.8X",crc32);
			putstring8x8(td,td->xsize/2 - (8*18),td->ysize - 9,tempstr,0xAAAAAA,0x000000,0,0);

			ytypepos = 31;
			i = 0;
			while(track_type_list[i].name)
			{
				if(countTrackType(td->sl,0,track_type_list[i].track_type))
				{
					putstring8x8(td,1,ytypepos,"             ",0xAAAAAA,0x000000,0,0);
					putstring8x8(td,1,ytypepos,track_type_list[i].name,0xAAAAAA,0x000000,0,0);
					ytypepos += 10;
				}
				i++;
			}

			ytypepos = 31;
			i = 0;
			while(track_type_list[i].name)
			{
				if(countTrackType(td->sl,1,track_type_list[i].track_type))
				{
					putstring8x8(td,(td->xsize/2)+1,ytypepos,"             ",0xAAAAAA,0x000000,0,0);
					putstring8x8(td,(td->xsize/2)+1,ytypepos,track_type_list[i].name,0xAAAAAA,0x000000,0,0);
					ytypepos += 10;
				}
				i++;
			}

		}
	}

	putstring8x8(td,x_pos_1 - 24,y_pos + 32,"Side 0",0x666666,0x000000,0,1);
	putstring8x8(td,x_pos_2 - 24,y_pos + 32,"Side 1",0x666666,0x000000,0,1);

	putstring8x8(td,x_pos_1 - 4,y_pos - 44,"->",0x666666,0x000000,0,1);
	putstring8x8(td,x_pos_2 - 4,y_pos - 44,"->",0x666666,0x000000,0,1);

	putstring8x8(td,x_pos_1 + 40,y_pos - 3,"---",0xCCCCCC,0x000000,0,1);
	putstring8x8(td,x_pos_2 + 40,y_pos - 3,"---",0xCCCCCC,0x000000,0,1);

	for(track=0;track<floppydisk->floppyNumberOfTrack+1;track++)
	{
		track_ep=(float)( (td->ysize-(y_pos)) - 60 ) /((float) floppydisk->floppyNumberOfTrack+1);

		draw_circle (td,0xF8F8F8,0,(float)((float)((float)2 * PI)),x_pos_1,y_pos,60 + (int)(((floppydisk->floppyNumberOfTrack-track) * track_ep)) + 1,1,(int)0);
		draw_circle (td,0xF8F8F8,0,(float)((float)((float)2 * PI)),x_pos_2,y_pos,60 + (int)(((floppydisk->floppyNumberOfTrack-track) * track_ep)) + 1,1,(int)0);
	}

	draw_circle (td,0xEFEFEF,0,(float)((float)((float)2 * PI)),x_pos_1,y_pos,60 + (int)(((floppydisk->floppyNumberOfTrack+1) * track_ep)) + 1,1,(int)0);
	draw_circle (td,0xEFEFEF,0,(float)((float)((float)2 * PI)),x_pos_2,y_pos,60 + (int)(((floppydisk->floppyNumberOfTrack+1) * track_ep)) + 1,1,(int)0);

	td->hxc_setprogress(floppydisk->floppyNumberOfTrack*floppydisk->floppyNumberOfSide,floppydisk->floppyNumberOfTrack*floppydisk->floppyNumberOfSide,td,td->progress_userdata);
}

void * hxcfe_td_getframebuffer(HXCFE_TD *td)
{
	if(td)
	{
		return (void*)td->framebuffer;
	}

	return 0;
}

int32_t hxcfe_td_getframebuffer_xres( HXCFE_TD *td )
{
	if(td)
	{
		return td->xsize;
	}

	return 0;

}

int32_t hxcfe_td_getframebuffer_yres( HXCFE_TD *td )
{
	if(td)
	{
		return td->ysize;
	}

	return 0;
}

int32_t hxcfe_td_exportToBMP( HXCFE_TD *td, char * filename )
{
	int i,j,k;
	uint32_t * ptr;
	unsigned char * ptrchar;
	bitmap_data bdata;

	uint32_t pal[256];
	int nbcol;

	ptrchar = 0;

	ptr = malloc((td->xsize*td->ysize*4));
	if(ptr)
	{
		memset(ptr,0,(td->xsize*td->ysize*4));
	}
	else
		goto alloc_error;

	ptrchar = malloc((td->xsize*td->ysize));

	if(ptrchar)
	{
		td->hxcfe->hxc_printf(MSG_INFO_1,"Converting image...");
		nbcol = 0;

		k=0;
		for(i=0;i< ( td->ysize );i++)
		{
			for(j=0;j< (td->xsize );j++)
			{
				ptrchar[k] = getPixelCode(td->framebuffer[k],(uint32_t*)&pal,&nbcol);
				k++;
			}
		}

		if(nbcol>=256)
		{
			k = 0;
			for(i=0;i< ( td->ysize );i++)
			{
				for(j=0;j< ( td->xsize );j++)
				{
					ptrchar[k] = ptrchar[k] & 0xF8F8F8;
					k++;
				}
			}

			for(i=0;i<256;i++)
			{
				pal[i]=i|(i<<8)|(i<<16);
			}

			nbcol = 0;
			k=0;
			for(i=0;i< ( td->ysize );i++)
			{
				for(j=0;j< ( td->xsize );j++)
				{
					ptrchar[k] = getPixelCode(td->framebuffer[k],(uint32_t*)&pal,&nbcol);
					k++;
				}
			}
		}

		td->hxcfe->hxc_printf(MSG_INFO_1,"Writing %s...",filename);

		if(nbcol>=256)
		{
			bdata.nb_color = 16;
			bdata.xsize = td->xsize;
			bdata.ysize = td->ysize;
			bdata.data = (uint32_t*)ptr;
			bdata.palette = 0;
			bmp16b_write(filename,&bdata);
		}
		else
		{
			bdata.nb_color = 8;
			bdata.xsize = td->xsize;
			bdata.ysize = td->ysize;
			bdata.data = (uint32_t*)ptrchar;
			bdata.palette = (unsigned char*)&pal;

			bmpRLE8b_write(filename,&bdata);
		}

		free(ptrchar);
	}
	else
	{
		goto alloc_error;
	}

	return 0;

alloc_error:

	if(ptr)
		free(ptr);

	if(ptrchar)
		free(ptrchar);

	return -1;
}

void hxcfe_td_deinit(HXCFE_TD *td)
{
	free(td->framebuffer);

	if(td->name)
		free(td->name);

	free(td);
}
