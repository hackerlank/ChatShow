/*
 * encry_book.c
 *
 *  Created on: 2014-10-1
 *      Author: happy_ant
 * Email: zhaolixiang99@163.com
 */

#include "encry_book.h"
#include "alloc.h"

//change carefully
uint8_t _g_initbook[256] = {
			0xe3,0xa4,0xe5,0x7c,0x0b,0x9e,0xbc,0xfd,0x67,0x14,0xb0,0x96,0x44,0x9a,0xc2,0x77,
			0x4f,0xe9,0xe1,0xd6,0x8b,0xb4,0x86,0x1f,0x25,0xdb,0x93,0xd8,0x62,0xc9,0xca,0x53,
			0xa8,0xa2,0xad,0x8c,0x28,0x64,0x5a,0x9b,0xcd,0xb7,0x33,0x3a,0xb6,0xf7,0x46,0x5d,
			0x32,0x0e,0xd9,0x90,0x36,0xf6,0x11,0xaa,0x54,0x98,0x66,0x47,0xcf,0xc0,0x17,0x8e,
			0xb2,0x07,0x50,0x72,0xa3,0xe2,0x10,0x34,0xed,0x4d,0xf9,0x20,0x27,0xb8,0xc4,0xbe,
			0x61,0x37,0x91,0x9c,0xcb,0xaf,0x09,0x5e,0x6e,0xef,0xb9,0x55,0x0c,0xfc,0x9d,0x26,
			0x95,0x22,0x1d,0x2e,0x7e,0xd4,0xc7,0x29,0xa6,0x42,0x74,0xa5,0xc1,0x65,0x35,0x3c,
			0x0f,0xc3,0xa1,0x2a,0x13,0x8f,0x2b,0x92,0x06,0x2c,0x00,0xfb,0x68,0x7f,0xbb,0xee,
			0xac,0x43,0x01,0x51,0x2f,0x4c,0x73,0xce,0x60,0x85,0x52,0x0d,0xae,0xe8,0x79,0xe4,
			0x6f,0x69,0x19,0x6c,0x1b,0x5c,0xd0,0xff,0xbf,0x56,0x38,0xba,0x24,0x04,0x39,0xf8,
			0x3f,0xb3,0x9f,0x16,0xf1,0xe6,0x6d,0x49,0x12,0xd1,0x81,0x03,0x57,0xd7,0xcc,0x18,
			0x7b,0x6b,0x63,0x5f,0xa0,0x87,0xeb,0x5b,0xf4,0x76,0xf2,0x45,0x41,0x02,0xc6,0x78,
			0x84,0x3e,0x83,0x23,0xec,0xb5,0x15,0xc8,0x05,0x6a,0x7d,0x08,0x75,0x21,0x58,0xb1,
			0xda,0x99,0xdf,0xf0,0x89,0xd3,0x0a,0xab,0xdc,0x3b,0x30,0x1a,0x7a,0x88,0xdd,0x59,
			0x4a,0xc5,0xa9,0xf5,0x2d,0x4b,0x71,0xbd,0xf3,0x70,0x8a,0xd2,0x4e,0x1c,0xde,0x80,
			0xe7,0x94,0xfa,0xe0,0x8d,0x97,0x48,0xea,0x1e,0x82,0x3d,0x40,0x31,0xa7,0xd5,0xfe};
/////////////////

#define EXCHANGE_ENCRYBOOK(book,index1,index2) \
	do{if(index1!=index2){\
		uint8_t temp = book->_enbook[index1];\
		book->_enbook[index1]=book->_enbook[index2];\
		book->_enbook[index2] = temp;\
	}}while(0)

inline void update_encrybook(struct encry_book* book,int times)
{
	uint64_t key = book->_key;
	uint8_t* keys = (uint8_t*)(&key);
	int n = times;
	while(times-- > 0)
	{
		EXCHANGE_ENCRYBOOK(book,(keys[0]),(keys[4]));
		++ keys[0]; ++ keys[4];
		EXCHANGE_ENCRYBOOK(book,(keys[1]),(keys[5]));
		++ keys[1]; ++ keys[5];
		EXCHANGE_ENCRYBOOK(book,(keys[2]),(keys[6]));
		++ keys[2]; ++ keys[6];
		EXCHANGE_ENCRYBOOK(book,(keys[3]),(keys[7]));
		++ keys[3]; ++ keys[7];
	}
	for(n = 0; n < 256;++n)
	{
		book->_debook[(book->_enbook[n])] = (uint8_t)n;
	}
}


uint64_t create_encrybook_key()
{
	return (0x1010101008080808|((uint64_t)rand()<< 32)|rand());
}


void get_encrybook(struct encry_book* book,uint64_t key)
{
	srand((uint32_t)time(0));
	memcpy(book->_enbook,_g_initbook,256);
	book->_key = key;
	update_encrybook(book,8);
}


void create_encrybooks(struct encry_book* list,int n)
{
	srand((uint32_t)time(0));
	int i;
	for(i = 0; i < n;i++)
	{
		memcpy(list[i]._enbook,_g_initbook,256);
		list[i]._key = create_encrybook_key();
		update_encrybook(&list[i],8);
	}
}
