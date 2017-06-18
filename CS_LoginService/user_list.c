/*
 * user_list.c
 *
 *  Created on: May 18, 2017
 *      Author: root
 */

#include "user_list.h"
#include "net_protocol.h"

struct user_account_group
{
	uint32_t _handlerid;
	uint32_t _last_update_time;
	struct str_hash _users;
};

struct user_account_list
{
	uint8_t _name_word[256];
	struct squeue _caches;
	int _max_cache;
	struct user_account_group _list[USERLIST_NUM];
};

/////////////////////////////////////////////////////////////////////////////

struct user_account_list* _g_user_list = NULL;

void user_account_domsg(handler_msg* msg,void* g,uint32_t hid)
{
	switch(msg->_msgid)
	{
	case USERMSG_LOGIN:
		handle_usermsg_login(msg,g);
		break;
	case USERMSG_REGISTER:
		handle_usermsg_register(msg,g);
		break;
	case USERMSG_PASSWORD:
		handle_usermsg_password(msg,g);
		break;
	}
}

void _free_timeout_user(void* u,void* t)
{
	free_user_cache((struct user_account*)u);
}

int _is_user_timeout(void* u ,void* t)
{
	return ((struct user_account*)u)->_lastused < *((uint32_t*)t) ? -1 : 0;
}

void user_account_doproc(void* hdata,uint32_t hid)
{
	uint32_t cur_time = get_mytime();
	struct user_account_group* g = (struct user_account_group*)hdata;
	if(g->_last_update_time < cur_time)
	{
		ASSERT(cur_time > 3600);
		cur_time -= 3600;
		doall_strhash(&g->_users,_is_user_timeout,_free_timeout_user,&cur_time);
		g->_last_update_time = get_mytime()+3600;
	}
}

/////////////////////////////////////////////////////////////////////////////

void userlist_init(int handler_group,int user_cache_max)
{
	ASSERT(_g_user_list == NULL && user_cache_max > 0 && user_cache_max < 0x7FFFFFFF);
	int size = sizeof(struct user_account_list);
	_g_user_list = (struct user_account_list*)my_malloc(size);
	memset(_g_user_list,0,size);
	_g_user_list->_max_cache = user_cache_max;
	init_squeue(&_g_user_list->_caches);
	int i;
	for(i = 0; i < 256; ++i)
	{
		_g_user_list->_name_word[i] = 255;
	}
	int index = 0;
	for(i = 'a'; i <= 'z'; ++i)
	{
		_g_user_list->_name_word[i] = index++;
	}
	index = 0;
	for(i = 'A'; i <= 'Z'; ++i)
	{
		_g_user_list->_name_word[i] = index++;
	}
	for(i = '0'; i <= '9'; ++i)
	{
		_g_user_list->_name_word[i] = index++;
	}
	_g_user_list->_name_word['.'] = index++;
	_g_user_list->_name_word['@'] = index++;
	_g_user_list->_name_word['_'] = index++;
	ASSERT(index == USERLIST_NUM);
	char userlist_name[32];
	for(i = 0; i < USERLIST_NUM; i++)
	{
		struct user_account_group* g = &_g_user_list->_list[i];
		g->_last_update_time = 3600;
		sprintf(userlist_name,"user_%d",i);
		g->_handlerid = register_handler(handler_group,userlist_name,user_account_domsg,user_account_doproc,NULL,g);
		ASSERT(g->_handlerid > 0);
	}
}

void userlist_uninit()
{
	if(_g_user_list == NULL)
		return;
	struct user_account_list* list = _g_user_list;
	_g_user_list = NULL;
	void *chache = pop_squeue(&list->_caches);
	while(chache)
	{
		my_free(chache);
		chache = pop_squeue(&list->_caches);
	}
	my_free(list);
}

inline int _check_username(const char* name)
{
	if(NULL == name || name[0] == 0)
		return -1;
	while(*name)
	{
		if(_g_user_list->_name_word[(uint8_t)(*name)] == 255)
			return -1;
		++name;
	}
	return 0;
}

void pushmsg_byname(const char* name,handler_msg* msg)
{
	if(-1 == _check_username(name))
	{
		free_handler_msg(msg);
		return;
	}
	int index = _g_user_list->_name_word[(uint8_t)(*name)];
	ASSERT(index >= 0 && index < USERLIST_NUM);
	send_handler_msg(_g_user_list->_list[index]._handlerid,msg);
}

/////////////////////////////////////////////////////////////////////////////

struct user_account* get_user_cache(void* hdata)
{
	struct user_account* cache = (struct user_account*)pop_squeue(&_g_user_list->_caches);
	if(NULL== cache)
	{
		cache = my_new(struct user_account);
	}
	memset(cache,0,sizeof(*cache));
	return cache;
}

void free_user_cache(struct user_account* ua)
{
	if(_g_user_list->_caches._count > _g_user_list->_max_cache)
	{
		my_free(ua);
	}
	else
	{
		push_squeue(&_g_user_list->_caches,ua);
	}
}

struct user_account* get_user_byname(char* name,void* hdata)
{
	struct user_account_group* g = (struct user_account_group*)hdata;
	struct user_account* u = get_str_hash(&g->_users,(const char*)(name));
	if(u){
		uint32_t cur = get_mytime();
		if(u->_time < cur)
		{
			if((u->_lastused&0xFFFFFF00) == (cur&0xFFFFFF00))
			{
				if(++u->_count > 32){u->_time = cur+3600;}
			}
			else
			{
				u->_time = 0;
				u->_count = 1;
			}
		}
		u->_lastused = get_mytime();
	}
	return u;
}


uint64_t pop_user_netid_byname(char* name,uint64_t* charid)
{
	uint64_t netid = 0;
	if(-1 == _check_username(name))
	{
		return -1;
	}
	int index = _g_user_list->_name_word[(uint8_t)(*name)];
	ASSERT(index >= 0 && index < USERLIST_NUM);
	struct user_account* g = get_user_byname(name,(void*)&(_g_user_list->_list[index]));
	if(g)
	{
		netid = g->_netid;
		*charid = g->_id;
		del_str_hash(&(_g_user_list->_list[index]._users),name);
	}

	return netid;
}

void add_user(struct user_account* u,void* hdata)
{
	u->_lastused = get_mytime();
	struct user_account_group* g = (struct user_account_group*)hdata;
	int res = set_str_hash(&g->_users,u->_name,u);
	ASSERT(res == 0);
}
