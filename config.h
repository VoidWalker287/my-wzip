#ifndef CONFIG
#define CONFIG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_SIZE 4096

struct clist_t {
     char *starts[3];
     char *ends[3];
     char *outputs[3];
     size_t output_lens[3];
     u_int32_t instances;
     char prev;
};

void w_compress(struct clist_t *clist);
void *w_compress0(void *p);
void *w_compress1(void *p);
void *w_compress2(void *p);
void w_write(struct clist_t *clist);
void w_remain(struct clist_t *clist);

#endif
