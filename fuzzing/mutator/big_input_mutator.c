#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// opaque state for our mutator
typedef struct my_mutator {
  int first_run;
} my_mutator_t;

// AFL++ will call this once when loading the mutator
void *afl_custom_init(void *afl, unsigned int seed) {
  my_mutator_t *data = calloc(1, sizeof(my_mutator_t));
  fprintf(stderr, "[mutator] BigBomb mutator loaded (seed=%u)\n", seed);
  data->first_run = 1;
  return data;
}

// AFL++ will call this for each fuzzing attempt
size_t afl_custom_fuzz(void *data,
                       uint8_t *buf,
                       size_t buf_size,
                       uint8_t **out_buf,
                       uint8_t *add_buf,
                       size_t add_buf_size,
                       size_t max_size) {
  my_mutator_t *d = (my_mutator_t *)data;

  if (d->first_run) {
    d->first_run = 0;
    size_t big_size =1024; 
    uint8_t *big = malloc(big_size);
    memset(big, 'A', big_size);
    *out_buf = big;
    return big_size;
  }

  // fallback: just return original buffer unchanged
  *out_buf = buf;
  return buf_size;
}

// AFL++ will call this when shutting down
void afl_custom_deinit(void *data) {
  free(data);
}

