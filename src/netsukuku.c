#include <string.h>
#include <phtread.h>
#include "netsukuku.h"
#include "xmalloc.h"
#include "log.h"

int init_load_maps(void)
{
	if(!(me.int_map=load_map(server_opt.int_map_file, &me.cur_node)))
		me.int_map=init_map(0);
	me.bnode_map=load_bmap(server_opt.bnode_map_file, &me.bmap_nodes);
	me.ext_map=load_gmap(server_opt.ext_map_file, &me.cur_gnode);
}

int save_maps(void)
{
	save_map(me.int_map, me.cur_node, server_opt.int_map_file);
	save_bmap(me.bnode_map, me.bmap_nodes, server_opt.bnode_map_file);
	save_gmap(me.ext_map, me.cur_gnode, server_opt.ext_map_file);
	
}

int main(int argc, char **argv)
{
	/*This shall be the main flow:*/

#ifdef QSPN_EMPIRIC
	error("QSPN_EMPIRIC is activated!!!!");
	exit(1);
#endif
	/*Init stuff
	void maxgroupnode_level_init(void);
	char *if_init(char *dev, &me.cur_dev_idx)
	join_ipv6_multicast();
	map_init();
	*/
	memset(&me, 0, sizeof(struct current));
	/*curme_init();*/

	init_radar();
	close_radar();

	/*Now we hook in the Netsukuku network
	netsukuku_hook();
	*/

	/*These are the main threads that keeps Netsukuku up & running.
	thread(connection_wrapper());
	thread(radar());
	thread(daemon_udp());
	daemon_tcp(); <<-- here we use this self process for the tcp_daemon
	*/

	/*destroy stuff
	  maxgroupnode_level_free();
	  */
}
