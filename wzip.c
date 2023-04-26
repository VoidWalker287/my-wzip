#include "config.h"

struct stat fs;
char event_horizon;

int main(int argc, char *argv[]) {
  // usage error
  if (argc < 2) {
    printf("wzip: file1 [file2 ...]\n");
    exit(1);
  }
  
  // initialize chunk list structure
  struct clist_t *clist = malloc(sizeof(struct clist_t));
  
  // file loop
  for (char arg = 1; arg < argc; arg++) {
    // special case: repeated large files
    if (fs.st_size > MAX_SIZE && arg > 1 && strcmp(argv[arg], argv[arg - 1]) == 0) {
      event_horizon++;
      if (event_horizon == 1) w_remain(clist);
      w_write(clist);
      w_remain(clist);
      goto END;
    } else {
      event_horizon = 0;
    }
    
    // free previous outputs if not null
    for (char i = 0; i < 3 && clist->outputs[i] != NULL; i++) free(clist->outputs[i]);
    
    // open current file
    FILE *f = fopen(argv[arg], "r");
    if (f == NULL) exit(1);
    
    // get file size
    fstat(fileno(f), &fs);
    size_t size = fs.st_size;

    // map file contents, close file
    clist->starts[0] = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fileno(f), 0);
    fclose(f);
    // check if file is big or small
    if (fs.st_size > MAX_SIZE) {
      // allocate max memory needed for chunks
      for (char i = 0; i < 3; i++) {
	clist->outputs[i] = malloc(fs.st_size * 2 / 5);
	clist->output_lens[i] = 0;
      }
      // set end for chunk 0
      clist->ends[0] = clist->starts[0] + fs.st_size / 3;
      while (*(clist->ends[0]) == *(clist->ends[0] + 1)) clist->ends[0]++;
      clist->ends[0]++;
      // set start & end for chunk 1
      clist->starts[1] = clist->ends[0];
      clist->ends[1] = clist->starts[1] + fs.st_size / 3;
      while (*(clist->ends[1]) == *(clist->ends[1] + 1)) clist->ends[1]++;
      clist->ends[1]++;
      // set start & end for chunk 2
      clist->starts[2] = clist->ends[1];
      clist->ends[2] = clist->starts[0] + fs.st_size;

      // create threads & process chunks
      pthread_t threads[3];
      pthread_create(&threads[0], NULL, &w_compress0, (void *)clist);
      pthread_create(&threads[1], NULL, &w_compress1, (void *)clist);
      pthread_create(&threads[2], NULL, &w_compress2, (void *)clist);

      // wait for threads to finish
      for (char i = 0; i < 3; i++) pthread_join(threads[i], NULL);
    } else {
      // use one chunk for small files
      clist->ends[0] = clist->starts[0] + fs.st_size;
      clist->outputs[0] = malloc(5 * size);
      clist->output_lens[0] = 0;
      w_compress(clist);
    }
    // write outputs, free memory
    w_write(clist);
  END:
    NULL;
  } // end of file loop
  
  // write remaining output
  if (event_horizon == 0) w_remain(clist);
  free(clist);
  return 0;
}
