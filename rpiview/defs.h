#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define PROG_VERSION        "1.0.0"

// These are the ONLY global variables, for verbose and running
extern bool g_running;
extern bool g_verbose;

#define USEC_PER_SEC	1000000

// Helper macros
#if 0
#define DEBUG(...)      printf(__VA_ARGS__)
#else
#define DEBUG(...)      do{ if(g_verbose) printf(__VA_ARGS__); }while(0)
#endif

#define strset(B,S) do{strncpy(&(B),S,sizeof(S)-1);}while(0)
#define pbuf(B,N)	do{int x=0;for(x=0;x<(N);x++) printf("0x%02X,",(B)[x]); printf("\n");}while(0)
#define cmp_const(B,S)	(strncmp((B),(S),sizeof(S)-1)==0)

#endif
