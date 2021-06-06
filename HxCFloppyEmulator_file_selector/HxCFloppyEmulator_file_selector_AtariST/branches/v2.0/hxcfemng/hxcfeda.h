#ifndef __HXCFEDA_H__
#define __HXCFEDA_H__

/*
//
// Copyright (C) 2009, 2010, 2011 Jean-Francois DEL NERO
//
// This file is part of the HxCFloppyEmulator file selector.
//
// HxCFloppyEmulator file selector may be used and distributed without restriction
// provided that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator file selector is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator file selector is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator file selector; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

#include "conf.h"

int hxc_media_read(unsigned long sector, unsigned char *buffer);
int hxc_media_write(unsigned long sector, unsigned char *buffer);
void hxc_enterModule(unsigned char bootdev);


typedef struct DirectoryEntry_ {
	unsigned char name[12];
	unsigned char attributes;
/*	unsigned long firstCluster;*/
	unsigned char firstCluster_b1;
	unsigned char firstCluster_b2;
	unsigned char firstCluster_b3;
	unsigned char firstCluster_b4;
/*	unsigned long size;*/
	unsigned char size_b1;
	unsigned char size_b2;
	unsigned char size_b3;
	unsigned char size_b4;
	unsigned char longName[LFN_MAX_SIZE];	/* boolean */
}  __attribute__ ((__packed__)) DirectoryEntry;

#endif
