/* This file is part of Netsukuku
 * (c) Copyright 2004 Andrea Lo Pumo aka AlpT <alpt@freaknet.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
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
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <gmp.h>

#include "gmap.h"
#include "xmalloc.h"
#include "log.h"

void maxgroupnode_level_init(void)
{
	mpz_t base;
	int i, e;
	mpz_init(base);	
	
	mpz_set_ui(base, MAXGROUPNODE);
	for(i=0, e=1; i<MAX_LEVELS; i++, e++) {
		mpz_init(maxgroupnode_levels[i]);
		mpz_pow_ui(maxgroupnode_levels[i], base, e);
	}
	mpz_clear(base);
}

void maxgroupnode_level_free(void)
{
	int i;
	for(i=0; i<MAX_LEVELS; i++) 
		mpz_clear(maxgroupnode_levels[i]);
}

/* iptogid: ip to gnode id, of the specified `level', conversion function.
 * Note: this function cannot fail! So be sure to pass a valid `level'.*/
u_short iptogid(inet_prefix ip, u_char level)
{
	u_char upper_level=1;
	__u32 h_ip[4];

	memcpy(h_ip, ip.data, 4);
	if(ip.family==AF_INET) {	
		if(level == (IPV4_LEVELS-1))
			upper_level=0;
	} else {
		if(level == (IPV6_LEVELS-1))
			upper_level=0;
	}

	if(!upper_level) {
		/* gid=ip/MAXGROUPNODE^level */
		div_mpz(h_ip, maxgroupnode_levels[level]);
	} else {
		mpz_t xx, yy, zz;
		int count;
		upper_level=level+1;

		/* gid=(ip/MAXGROUPNODE^level) - (ip/MAXGROUPNODE^(level+1) * MAXGROUPNODE); */
		mpz_init(xx);
		mpz_init(yy);
		mpz_init(zz);

		mpz_import(yy, 4, ORDER, sizeof(h_ip[0]), NATIVE_ENDIAN, 0, h_ip);

		mpz_tdiv_q(xx, yy, maxgroupnode_levels[level]);
		mpz_tdiv_q(zz, yy, maxgroupnode_levels[upper_level]);
		mpz_mul_ui(zz, zz, MAXGROUPNODE);
		mpz_sub(yy, xx, zz);

		memset(h_ip, '\0', sizeof(h_ip[0])*4);
		mpz_export(h_ip, &count, ORDER, sizeof(h_ip[0]), NATIVE_ENDIAN, 0, yy);

		mpz_clear(xx);
		mpz_clear(yy);
		mpz_clear(zz);
	}

	return  (u_short) h_ip[0];
}

/* gidtoipstart: It `*ip' to the ipstart of the gnode with `gid' in `level'.
 * `family' is used to fill the inet_prefix of ipstart.*/
void gidtoipstart(u_short gid, u_char level, int family, inet_prefix *ip)
{
	mpz_t xx;
	int count;
	inet_prefix *ip;
	__u32 h_ip[4];

	memset(h_ip, '\0', sizeof(h_ip[0])*4);
	if(!level) {
		/* ip = MAXGROUPNODE*gid; */
		h_ip[0]=MAXGROUPNODE*gid;
	} else {
		mpz_init(xx);
		/* ip = MAXGROUPNODE^level + MAXGROUPNODE*gid; */
		mpz_add_ui (xx, maxgroupnode_levels[level], MAXGROUPNODE*gid);
		mpz_export(h_ip, &count, ORDER, sizeof(h_ip[0]), NATIVE_ENDIAN, 0, xx);
		mpz_clear(xx);
	}

	ip=xmalloc(sizeof(inet_prefix));
	memcpy(ip->data, h_ip, sizeof(int)*4);
	ip->family=family;
	ip->len=family == AF_INET ? 4 : 16;
	return ip;
}

/* iptoquadg: Using the given `ip' it fills the `qg' quadro_group struct.*/
void iptoquadg(inet_prefix ip, map_gnode *ext_map, quadro_group *qg)
{
	int i;
	u_char levels;
	
	if(ip.family==AF_INET) 
		levels=IPV4_LEVELS;
	else
		levels=IPV6_LEVELS;

	qg->levels=levels;
	for(i=0; i<levels; i++) {
		qg->gid[i]=iptogid(ip, i);
		gq->gnode[i]=GI2GMAP(ext_map[i], qg->gid[i]);
		gidtoipstart(qg->gid[i], i, ip.family, &gq->ipstart[i]);
	}
}

map_gnode *init_gmap(u_short groups)
{
	map_gnode *gmap;
	size_t len;
	
	if(!groups)
		groups=MAXGROUPNODE;
	len=sizeof(map_gnode)*groups;
	gmap=(map_gnode *)xmalloc(len);
	memset(gmap[i], '\0', len);
	
	return gmap;
}

void free_gmap(map_gnode *gmap, u_short groups)
{
	int i;
	size_t len;

	if(!groups)
		groups=MAXGROUPNODE;
	len=sizeof(map_gnode)*groups;
	
	for(i=0; i<groups; i++)
		rnode_destroy(gmap[i]);
}

/* init_gmap: Initialize an ext_map with `levels' gmap. Each gmap
 * has `groups' elements*/
map_gnode *init_extmap(u_char levels, u_short groups)
{
	map_gnode **ext_map;
	int i;

	if(!levels)
		levels=MAX_LEVELS;
	if(!groups)
		groups=MAXGROUPNODE;

	ext_map=(map_gnode *)xmalloc(sizeof(map_node *) * levels);
	for(i=0; i<levels; i++)
		ext_map[i]=init_gmap(groups);
	return map;
}

/* free_gmap: Destroy the ext_map*/
void free_extmap(map_gnode **ext_map, u_char levels, u_short groups)
{
	int e;

	if(!levels)
		levels=MAX_LEVELS;
	if(!groups)
		groups=MAXGROUPNODE;
	
	for(e=0; e<levels; e++) {
		free_gmap(ext_map[e], groups);
		xfree(ext_map[e]);
	}
	xfree(ext_map);
}

/* gmap_get_rblock: It uses get_rnode_block to pack all the ext_map's rnodes
 * `maxgroupnode' is the number of nodes present in the map.
 * It returns a pointer to the start of the rnode block and stores in "count" 
 * the number of rnode structs packed*/
map_rnode *gmap_get_rblock(map_gnode *map, int maxgroupnode, int *count)
{
	int i, c=0, tot=0;
 	map_rnode *rblock;
	*count=0;
	
	for(i=0; i<maxgroupnode; i++)
		tot+=map[i].g.links;
	if(!tot)
		return 0;
	rblock=(map_rnode *)xmalloc(sizeof(map_rnode)*tot);

	for(i=0; i<maxgroupnode; i++)
		c+=get_rnode_block((int *)map, &map[i].g, rblock, c);

	*count=c;
	return rblock;
}

/* gmap_store_rblock: Given a correct ext_map with `maxgroupnode' elements it
 * restores all the r_node structs in the map from the rnode_block using 
 * store_rnode_block.*/
int gmap_store_rblock(map_gnode *map, int maxgroupnode, map_rnode *rblock)
{
	int i, c=0;

	for(i=0; i<maxgroupnode; i++)
		c+=store_rnode_block((int *)map, &map[i], rblock, c);
	return c;
}

/* extmap_get_rblock: It packs the rnode_block for each map present in the `ext_map'.
 * There are a total of `levels' maps in the ext_map. Each map has `maxgroupnodes' 
 * nodes. In `*ret_count' is stored an array of map's rnodes count, so each element
 * of the array represents the number of rnodes in the rblock of the relative map.
 * It returns an array of rblock's pointer. Each array's element points to the rblock
 * for the map in that level.
 * Ex: ret_count[n] is the number of rnodes placed in rblock[n], and n is also the
 * level of the map which has those rnodes. I hope I was clear ^_-
 * PS: You'll have to xfree ret_count, rblock[0-levels], and rblock;
 */ 
map_rnode *extmap_get_rblock(map_gnode **ext_map, u_char levels, int maxgroupnodes, int *ret_count)
{
	int i;
 	map_rnode **rblock;
	int *count;
	
	if(!levels)
		return 0;
	rblock=xmalloc(sizeof(map_rnode *) * levels);
	count=xmalloc(sizeof(int) * levels);
	
	for(i=0; i<levels; i++) 
		rblock[i]=gmap_get_rblock(ext_map[i], maxgroupnodes, &count[i]);
	
	*ret_count=count;
	return rblock;
}

/* extmap_store_rblock: It [re]stores all the rnodes in all the maps of the `ext_map'.
 * The maps have `maxgroupnode' nodes, while the `ext_map' has `levels' levels.
 * The rnodes are taken from the `rblock'.
 * The number of map restored is returned and it shall be equal to the number of `levels'.
 */
int extmap_store_rblock(map_gnode **ext_map, u_char levels, int maxgroupnode, map_rnode **rblock)
{
	int i;
	for(i=0; i<levels; i++)
		gmap_store_rblock(ext_map[i], maxgroupnode, rblock[i]);
	return i;
}

/* verify_ext_map_hdr: What to say? It verifies the validity of an ext_map_hdr*/
int verify_ext_map_hdr(struct ext_map_hdr *emap_hdr)
{
	u_char levels;
	int maxgroupnode;
	
	levels=emap_hdr->quadg.levels;
	maxgroupnode=emap_hdr->ext_map_sz/(sizeof(map_node)*levels);
	if(maxgroupnode > MAXGROUPNODE || emap_hdr->total_rblock_sz > MAXRNODEBLOCK*levels 
			|| emap_hdr->ext_map_sz > maxgroupnode*sizeof(map_node)*levels
			|| emap_hdr->quadg.gid[0] > maxgroupnode)
		return 1;

	return 0;
}

void free_extmap_rblock(map_rnode **rblock, u_char levels)
{
	int i;
	for(i=0; i<levels; i++)
		if(rblock[i])
			xfree(rblock[i]);
	xfree(rblock[i]);
}

/* pack_extmap: It returns the packed `ext_map', ready to be saved or sent. It stores 
 * in `pack_sz' the size of the package. Each gmaps, present in the `ext_map', has 
 * `maxgroupnode' nodes. `quadg' must be a valid quadro_group struct filled with 
 * valid values. */
char *pack_extmap(map_gnode **ext_map, int maxgroupnode, quadro_group *quadg, size_t *pack_sz)
{
	struct ext_map_hdr emap_hdr;
	map_rnode *rblock;
	int *count, i;
	char *package, *p=0;
	u_char levels=quadg->levels;

	/*Packing the rblocks*/
	rblock=extmap_get_rblock(ext_map, levels, maxgroupnode, &count);

	/*Building the hdr...*/
	memset(&emap_hdr, 0, sizeof(struct ext_map_hdr));
	emap_hdr.ext_map_sz=maxgroupnode*sizeof(map_node)*levels;
	for(i=0; i<levels; i++) {
		emap_hdr.rblock_sz[i]=count[i]*sizeof(map_rnode);
		emap_hdr.total_rblock_sz+=emap_hdr.rblock_sz;
		emap_hdr.gmap_sz[i]=maxgroupnode*sizeof(map_node);
	}
	memcpy(&emap_hdr.quadg, quadg, sizeof(quadro_group));
	memset(emap_hdr.gnode, 0, sizeof(map_gnode *) * MAX_LEVELS); /*emap_hdr.gnode loses its meaning*/

	/*Let's fuse all in one*/
	*pack_sz=EXT_MAP_BLOCK_SZ(emap_hdr.ext_map_sz, emap_hdr.total_rblock_sz);
	package=(char *)xmalloc(*pack_sz);
	memcpy(package, &emap_hdr, sizeof(struct ext_map_hdr));
	p=sizeof(struct ext_map_hdr);
	memcpy(package+p, ext_map, emap_hdr.ext_map_sz);
	if(rblock) {
		p+=emap_hdr.ext_map_sz;
		memcpy(package+p, rblock, emap_hdr.total_rblock_sz);
		free_extmap_rblock(rblock, levels);
	}

	xfree(count);
	return package;
}


/* unpack_extmap: Given a valid ext_map package (packed with pack_extmap), it 
 * allocates a brand new ext_map and restores in it the gmaps and the rnodes.
 * In `quadg' is stored the quadro_group referring to this ext_map.
 * On success the a pointer to the new ext_map is retuned, otherwise 0 will be
 * the fatal value.*/
map_gnode *unpack_extmap(char *package, size_t pack_sz, quadro_group *quadg)
{
	map_gnode **ext_map;
	struct ext_map_hdr *emap_hdr=(struct ext_map_hdr *)package;
	map_rnode **rblock;
	u_char levels;
	int err, i, maxgroupnode;
	char *p=0;

	levels=emap_hdr->quadg.levels;
	maxgroupnode=emap_hdr->ext_map_sz/(sizeof(map_node)*levels);
	if(verify_ext_map_hdr(emap_hdr)) {
		error("Malformed ext_map_hdr. Aborting unpack_map().");
		return 0;
	}
	
	/*Unpacking the ext_map*/
	p=sizeof(struct ext_map_hdr);
	ext_map=init_extmap(levels, maxgroupnode);
	memcpy(ext_map, package+p, emap_hdr.ext_map_sz);
	
	/*We restore the quadro_group struct*/
	memcpy(quadg, &emap_hdr->quadg, sizeof(quadro_group));
	for(i=0; i<levels; i++)
		quadg->gnode[i]=GI2GMAP(ext_map[i], quadg->gid[i]);

	/*Let's store in it the lost rnodes.*/
	p+=emap_hdr.ext_map_sz;
	if(emap_hdr.total_rblock_sz) {
		rblock=package+p;
		err=extmap_store_rblock(ext_map, levels, maxgroupnode, rblock);
		if(err!=levels) {
			error("unpack_extmap(): It was not possible to restore all the rnodes in the ext_map");
			free_extmap(ext_map, levels, maxgroupnode);
			return 0;
		}
	}
	/*Let's mark our gnodes ;)*/
	for(i=0; i<levels; i++)
		ext_map[i][quadg->gid[i]]|=GMAP_ME;
	
	return ext_map;
}

/* * * save/load ext_map * * */
int save_extmap(map_gnode **ext_map, int maxgroupnode, quadro_group *quadg, char *file)
{
	FILE *fd;
	size_t pack_sz;
	char *pack;
	
	if((fd=fopen(file, "w"))==NULL) {
		error("Cannot save the map in %s: %s", file, strerror(errno));
		return -1;
	}

	/*Pack!*/
	pack=pack_extmap(ext_map, maxgroupnode, quadg, &pack_sz);
	/*Write!*/
	fwrite(pack, pack_sz, 1, fd);
	xfree(pack);

	fclose(fd);
	return 0;
}

map_gnode *load_extmap(char *file, quadro_group *quadg)
{
	map_gnode **ext_map;
	FILE *fd;
	struct int_map_hdr imap_hdr;
	size_t pack_sz;
	int err;
	char *pack;
	
	if((fd=fopen(file, "r"))==NULL) {
		error("Cannot load the map from %s: %s", file, strerror(errno));
		return 0;
	}

	fread(&emap_hdr, sizeof(struct ext_map_hdr), 1, fd);
	if(verify_ext_map_hdr(&emap_hdr)) {
		error("Malformed ext_map_hdr. Aborting load_extmap().");
		return 0;
	}

	rewind(fd);
	pack_sz=EXT_MAP_BLOCK_SZ(emap_hdr.ext_map_sz, emap_hdr.total_rblock_sz);
	pack=xmalloc(pack_sz);
	fread(pack, pack_sz, 1, fd);
	ext_map=unpack_extmap(pack, pack_sz, quadg);
	if(!ext_map)
		error("Cannot unpack the ext_map!");

	xfree(pack);
	fclose(fd);
	return ext_map;
}
