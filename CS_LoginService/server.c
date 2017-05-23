/*
 * server.c
 *
 *  Created on: May 18, 2017
 *      Author: root
 */


#include "server.h"
#include "server_config.h"
#include "server_db.h"
#include "user_list.h"
#include "user_msg.h"
#include "packet.h"
#include <signal.h>


int* _g_server_running = 0;

void signal_proc(int sg) {
	*_g_server_running = 0;
}

//////////////////////////////////////////////////////////////////

int init_server(int* running)
{
	_g_server_running = running;
	signal(SIGHUP,SIG_IGN);
	signal(SIGPIPE,SIG_IGN);
	signal(SIGUSR1,signal_proc);
	signal(SIGUSR2,signal_proc);
	srand((unsigned int)time(0));
	//
	void* config = read_server_config("config.ini");
	if(NULL == config) {
		return -1;
	}
	//8K
	create_handler_msg_cachepool(8142,config_server_cache_poolsize(config));

	//handler
	init_handlerlib(config_server_id(config),config_handlers_count(config));
	create_handlers_group(1,config_tcp_threads_count(config));
	create_handlers_group(2,config_work_threads_count(config));
	//log
	init_loglib(2,config_log_level(config),config_log_prev(config));
	//net
	init_tcplib(1,config_tcp_handlers_count(config));
	//protocol
	init_protocollib();

	char str[10240];
	const char* ip = "0.0.0.0";
	uint32_t addr = INADDR_ANY;
	if (ip&&ip[0]) {addr=inet_addr(ip);}

	tcp_packet* packet = create_packet(0x101);
	push_packet_int32(packet,3305);
	push_packet_end(packet);

	memcpy(str,packet->_head,PACKET_BY_HEADER_SIZE);
	memcpy(str + PACKET_BY_HEADER_SIZE,packet->_buf, packet->_off);
	conn_tcpprotocol(1,addr,7000,str,packet->_off + PACKET_BY_HEADER_SIZE,0,1,1);

	destory_packet(packet);
//	if(is_config_cmd(config) == 0)
//	{
//		const char* ip = config_cmd_ip(config);
//		int port = config_cmd_port(config);
//		int protocol = config_cmd_protocol(config);
//		if(0 != bind_tcpprotocol(ip,port,protocol))
//		{
//			return -1;
//		}
//	}
//	int i;
//	for(i = 0; i < 128; i++)
//	{
//		if(0 != is_config_user(config,i))
//		{
//			continue;
//		}
//		const char* ip = config_user_ip(config,i);
//		int port = config_user_port(config,i);
//		int protocol = config_user_protocol(config,i);
//		if(0 != bind_tcpprotocol(ip,port,protocol))
//		{
//			return -1;
//		}
//	}

//	if(is_config_db(config,0) == 0)
//	{
//		init_mydb(config,0);
//	}
//	else
//	{
//		return -1;
//	}

//	userlist_init(2,config_server_user_poolsize(config));
//	if(set_serverkey(config_server_userkey(config),config_server_usertable(config),
//			config_server_userserverid(config)) == -1)
//	{
//		return -1;;
//	}

	close_server_config(config);
	return 0;
}

void uninit_server()
{
	userlist_uninit();
	uninit_tcplib();
	uninit_protocollib();
	uninit_loglib();
	uninit_handlerlib();
	destory_handler_msg_cachepool();
}


/////////////////////////////////////////////////////////////////

void start_server()
{
	start_all_handlers();
}
void stop_server()
{
	stop_all_handlers();
}
void server_timer()
{
	do_timer();
}
