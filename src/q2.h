/* This file is part of Netsukuku
 * (c) Copyright 2007 Andrea Lo Pumo aka AlpT <alpt@freaknet.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef QSPN_H
#define QSPN_H


/* 
 * NOTE: this file will substitute qspn.h
 */

/* 
 * Maximum number of recorded interesting routes.
 */
#define MAX_QCACHE_ROUTES	1

/*
 * Map:
 * 	new route
 *
 * qmap:
 */
typedef struct
{
	
} qcache;

/*
 * QSPN Events
 */
ev_t QSPN_ADD_NODE,
     QSPN_ADD_GNODE,
     QSPN_DEL_NODE,
     QSPN_DEL_GNODE;

#endif /*QSPN_H*/
