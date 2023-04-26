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
  // 5-byte output array
  char out[5];

  // check if count left over from prev file
  if (instances > 0 && *start != prev) {
    // if left over count is different, add to output
    w_u32to4(out, instances);
    out[5] = prev;
    for (char i = 0; i < 5; i++, o_len++) output[o_len] = out[i];
    prev = *start;
    instances = 1;
  }
  
  // process map
  while (start != end) {
    if (prev == 0) {
      prev = *start;
      instances = 1;
    } else if (*start == prev) {
      instances++;
    } else {
      w_u32to4(out, instances);
      out[4] = prev;
      for (char i = 0; i < 5; i++, o_len++) output[o_len] = out[i];
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
  // 5-byte output array
  char out[5];

  // check if count left over from prev file
  if (instances > 0 && *start != prev) {
    // if left over count is different, add to output
    w_u32to4(out, instances);
    out[5] = prev;
    for (char i = 0; i < 5; i++, o_len++) output[o_len] = out[i];
    prev = *start;
    instances = 1;
  }
  
  // process map
  while (start != end) {
    if (prev == 0) {
      prev = *start;
      instances = 1;
    } else if (*start == prev) {
      instances++;
    } else {
      w_u32to4(out, instances);
      out[4] = prev;
      for (char i = 0; i < 5; i++, o_len++) output[o_len] = out[i];
      prev = *start;
      instances = 1;
    }
    start++;
  }

  // write remaining count to output
  w_u32to4(out, instances);
  out[4] = prev;
  for (char i = 0; i < 5; i++, o_len++) output[o_len] = out[i];
  
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
  // 5-byte output array
  char out[5];
  
  // process map
  while (start != end) {
    if (prev == 0) {
      prev = *start;
      instances = 1;
    } else if (*start == prev) {
      instances++;
    } else {
      w_u32to4(out, instances);
      out[4] = prev;
      for (char i = 0; i < 5; i++, o_len++) output[o_len] = out[i];
      prev = *start;
      instances = 1;
    }
    start++;
  }

  // write remaining count to output
  w_u32to4(out, instances);
  out[4] = prev;
  for (char i = 0; i < 5; i++, o_len++) output[o_len] = out[i];
  
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
  // 5-byte output array
  char out[5];
  
  // process map
  while (start != end) {
    if (prev == 0) {
      prev = *start;
      instances = 1;
    } else if (*start == prev) {
      instances++;
    } else {
      w_u32to4(out, instances);
      out[4] = prev;
      for (char i = 0; i < 5; i++, o_len++) output[o_len] = out[i];
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

// convert u_int32_t to chars
void w_u32to4(char out[5], u_int32_t instances) {
  for (char i = 0; i < 4; i++) out[3 - i] = (instances >> (24 - 8 * i)) & 0xFF;
}

// send output package to output buffer
void w_out5(char out[5], char *buf, size_t *ind) {
  for (char i = 0; i < 5; i++, *ind++) buf[*ind] = out[i];
}

// write every output to stdout
void w_write(struct clist_t *clist) {
  for (char i = 0; i < 3 && clist->output_lens[i] != 0; i++) {
    // fprintf(stderr, "DEBUG> writing chunk %d to stdout (%lu bytes)\n", i, clist->output_lens[i]);
    char *o = clist->outputs[i];
    for (size_t j = 0 ; j < clist->output_lens[i]; j++) {
      printf("%c", o[j]);
    }
  }
}

// write remaining count to stdout
void w_remain(struct clist_t *clist) {
  if (clist->instances > 0) {
    fwrite(&(clist->instances), 4, 1, stdout);
    printf("%c", clist->prev);
  }
}

