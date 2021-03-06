/*
 * user_list.h
 *
 *  Created on: May 18, 2017
 *      Author: root
 */

#ifndef USER_LIST_H_
#define USER_LIST_H_

#include "user_msg.h"

#define USERLIST_NUM 39

struct user_account
{
	uint64_t _netid;
	uint64_t _id;
	uint32_t _time;
	uint32_t _lastused;
	uint8_t _count;
	char _state;
	char _name[33];
	char _password[33];
};


//a~z0~9_@.
void userlist_init(int handler_group,int user_cache_max);
void userlist_uninit();

void pushmsg_byname(const char* name,handler_msg* msg);

uint64_t pop_user_netid_byname(char* name,uint64_t *charid);
struct user_account* get_user_cache(void*);
void free_user_cache(struct user_account* ua);
struct user_account* get_user_byname(char* name,void*);
void add_user(struct user_account* ua,void*);


#endif /* USER_LIST_H_ */
