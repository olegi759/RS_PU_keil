#ifndef NET_UTIL_H
#define NET_UTIL_H

#ifndef HTONS
#define HTONS(a)			((((a)>>8)&0xff)|(((a)<<8)&0xff00))
#endif

#ifndef NTOHS
#define NTOHS(a)			HTONS(a)
#endif

#ifndef HTONL
#define HTONL(a)			( (((a)>>24)&0xff) | (((a)>>8)&0xff00) |\
							(uint32_t)((((uint64_t)a)<<8) & 0xff0000) |\
							(uint32_t)((((uint64_t)a)<<24) & 0xff000000) )
#endif

#ifndef NTOHL
#define NTOHL(a)			HTONL(a)
#endif


#ifndef HTON24
#define HTON24(a)			( (((a) & 0x0000ff) << 16) | ((a) & 0x00ff00) | (((a) & 0xff0000) >> 16) )
#endif

#ifndef NTOH24
#define NTOH24(a)			HTON24(a)
#endif



#ifndef IPV4ADDR
#define IPV4ADDR(a,b,c,d)	        ( ((uint32_t)a) | ((uint32_t)b << 8) |\
									((uint32_t)c << 16) | ((uint32_t)d << 24) )
#endif


//Big-endian
#define BE_TWOBYTE2US(first_byte, second_byte)										(((uint16_t)first_byte << 8) | (uint16_t)second_byte)
#define BE_US2TWOBYTE(us, first_byte, second_byte)									{ first_byte = (us >> 8) & 0xFF; second_byte = us & 0xFF; }
#define BE_FOURBYTE2UL(first_byte, second_byte, third_byte, fourth_byte)			( ((uint32_t)first_byte << 24) | ((uint32_t)second_byte << 16) | ((uint32_t)third_byte << 8) | (uint32_t)fourth_byte )
#define BE_UL2FOURBYTE(ul, first_byte, second_byte, third_byte, fourth_byte)		{ first_byte = (ul >> 24) & 0xFF; second_byte = (ul >> 16) & 0xFF; third_byte = (ul >> 8) & 0xFF;  fourth_byte = ul & 0xFF; }



//Little-endian
#define LE_TWOBYTE2US(first_byte, second_byte)										BE_TWOBYTE2US(second_byte, first_byte)
#define LE_US2TWOBYTE(us, first_byte, second_byte)									BE_US2TWOBYTE(us, second_byteб first_byte)
#define LE_FOURBYTE2UL(first_byte, second_byte, third_byte, fourth_byte)			BE_FOURBYTE2UL(fourth_byte, third_byte, second_byte, first_byte)
#define LE_UL2FOURBYTE(ul, first_byte, second_byte, third_byte, fourth_byte)		BE_UL2FOURBYTE(ul, fourth_byte, third_byte, second_byte, first_byte)

#define LE_THREEBYTE2U24(first_byte, second_byte, third_byte)						( ((uint32_t)third_byte << 16) | ((uint32_t)second_byte << 8) | (uint32_t)first_byte )
#define LE_U242THREEBYTE(ul, first_byte, second_byte, third_byte)					{ third_byte = (ul >> 16) & 0xFF; second_byte = (ul >> 8) & 0xFF; first_byte = ul  & 0xFF; }


#ifdef __cplusplus
extern "C" {
#endif

//ip to string
char * ip2str(unsigned long ip);
unsigned long str2ip(char * str);
//mac to string
char * mac2str(unsigned char * mac);
unsigned char * str2mac(char * str);

#ifdef __cplusplus
}
#endif


#endif
