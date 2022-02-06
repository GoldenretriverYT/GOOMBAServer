#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif
typedef unsigned char *POINTER;
typedef unsigned short int UINT2;
typedef unsigned long int UINT4;
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif
