#include "config.h"

// compression function for small files
void w_compress(struct clist_t *clist) {
     // unpack clist
     char *start = clist->starts[0];
     char *end = clist->ends[0];
     char *output = clist->outputs[0];
     size_t o_len = clist->output_lens[0];
     u_int32_t instances = clist->instances;
     char prev = clist->prev;
     // check if count left over from prev file
     if (instances > 0 && *start != prev) {
          // if left over count is different, write to output
          for (char i = 0; i < 4; i++, o_len++) output[o_len] = (instances >> (8 * i)) & 0xFF;
	  output[o_len++] = prev;
          prev = *start;
          instances = 0;
     }
     // process map
     while (start != end) {
          if (prev == 0) {
               prev = *start;
               instances = 1;
          } else if (*start == prev) {
               instances++;
          } else {
               for (char i = 0; i < 4; i++, o_len++) output[o_len] = (instances >> (8 * i)) & 0xFF;
	       output[o_len++] = prev;
               prev = *start;
               instances = 1;
          }
          start++;
     }
     // save uncounted final char for next file or output
     clist->prev = prev;
     clist->instances = instances;
     clist->output_lens[0] = o_len;
}
// compression functions for threads
void *w_compress0(void *p) {
     // unpack clist
     struct clist_t *clist = (struct clist_t *)p;
     char *start = clist->starts[0];
     char *end = clist->ends[0];
     char *output = clist->outputs[0];
     size_t o_len = clist->output_lens[0];
     u_int32_t instances = clist->instances;
     char prev = clist->prev;
     // check if count left over from prev file
     if (instances > 0 && *start != prev) {
          // if left over count is different, add to output
          for (char i = 0; i < 4; i++, o_len++) output[o_len] = (instances >> (8 * i)) & 0xFF;
	  output[o_len++] = prev;
          prev = *start;
          instances = 0;
     }
     // process map
     while (start != end) {
          if (prev == 0) {
               prev = *start;
               instances = 1;
          } else if (*start == prev) {
               instances++;
          } else {
               for (char i = 0; i < 4; i++, o_len++) output[o_len] = (instances >> (8 * i)) & 0xFF;
	       output[o_len++] = prev;
               prev = *start;
               instances = 1;
          }
          start++;
     }
     // write remaining count to output
     for (char i = 0; i < 4; i++, o_len++) output[o_len] = (instances >> (8 * i)) & 0xFF;
     output[o_len++] = prev;
     clist->output_lens[0] = o_len;
     return NULL;
}

void *w_compress1(void *p) {
     // unpack clist
     struct clist_t *clist = (struct clist_t *)p;
     char *start = clist->starts[1];
     char *end = clist->ends[1];
     char *output = clist->outputs[1];
     size_t o_len = clist->output_lens[1];
     u_int32_t instances = 0;
     char prev = *start;
     // process map
     while (start != end) {
          if (prev == 0) {
               prev = *start;
               instances = 1;
          } else if (*start == prev) {
               instances++;
          } else {
               for (char i = 0; i < 4; i++, o_len++) output[o_len] = (instances >> (8 * i)) & 0xFF;
	       output[o_len++] = prev;
               prev = *start;
               instances = 1;
          }
          start++;
     }
     // write remaining count to output
     for (char i = 0; i < 4; i++, o_len++) output[o_len] = (instances >> (8 * i)) & 0xFF;
     output[o_len++] = prev;
     clist->output_lens[1] = o_len;
     return NULL;
}

void *w_compress2(void *p) {
     // unpack clist
     struct clist_t *clist = (struct clist_t *)p;
     char *start = clist->starts[2];
     char *end = clist->ends[2];
     char *output = clist->outputs[2];
     size_t o_len = clist->output_lens[2];
     char prev = *start;
     u_int32_t instances = 0;
     // process map
     while (start != end) {
          if (prev == 0) {
               prev = *start;
               instances = 1;
          } else if (*start == prev) {
               instances++;
          } else {
               for (char i = 0; i < 4; i++, o_len++) output[o_len] = (instances >> (8 * i)) & 0xFF;
	       output[o_len++] = prev;
               prev = *start;
               instances = 1;
          }
          start++;
     }
     // save uncounted final char for next file or output
     clist->prev = prev;
     clist->instances = instances;
     clist->output_lens[2] = o_len;
     return NULL;
}

// write every output to stdout
void w_write(struct clist_t *clist) {
     for (char i = 0; i < 3 && clist->output_lens[i] != 0; i++) {
          fwrite(clist->outputs[i], 1, clist->output_lens[i], stdout);
     }
}

// write remaining count to stdout
void w_remain(struct clist_t *clist) {
     if (clist->instances > 0) {
          fwrite(&(clist->instances), 5, 1, stdout);
     }
}

