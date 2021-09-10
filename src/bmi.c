#include "bm.h"

int main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "Usage: ./bmi <input.bm>");
    fprintf(stderr, "ERROR: expected input");
    exit(1);
  }
  
  bm_load_program_from_file(&bm, argv[1]);
  for (int i = 0; i < 69 && !bm.halt; ++i) {
    Err err = bm_execute_inst(&bm);
    bm_dump_stack(stdout, &bm);
    if (err != ERR_OK) {
      fprintf(stderr, "ERROR: %s\n", err_as_cstr(err));
      exit(1);
    }
  }
  bm_dump_stack(stdout, &bm);
   
  return 0;
}
