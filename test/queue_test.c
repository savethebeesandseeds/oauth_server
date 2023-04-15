#include "queue_piaabo.h"

typedef struct item_type {
  char *key;
  char *value;
} item_type_t;
typedef __queue_t items_queue_t;


#define COUNT_ITEMS 10
int main(int argc, char const *argv[])
{
  clock_t start;
  clock_t end;
  double time_taken;
  
  /* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
  /* queue intiailization */
  start = clock();
  items_queue_t *items = queue_fabric();
  end = clock();
  time_taken = (double)(end - start) / CLOCKS_PER_SEC; // Calculate the time taken
  printf("[queue_fabric]: %f seconds\n", time_taken);
  /* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

  /* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
  /* queue population */
  start = clock();
  for(int i = 0; i < COUNT_ITEMS; i++) {
    item_type_t *ditem = (item_type_t *)malloc(sizeof(item_type_t));
    ditem->key="key";
    ditem->value="value";
    queue_insert_item_on_top(items, ditem, sizeof(item_type_t), free);
  }
  end = clock();
  time_taken = (double)(end - start) / CLOCKS_PER_SEC; // Calculate the time taken
  printf("[queue_insert_items_on_top<%d>]: %f seconds\n", COUNT_ITEMS, time_taken);
  /* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

  /* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
  /* queue movements */
  start = clock();
  queue_to_base(items);
  end = clock();
  time_taken = (double)(end - start) / CLOCKS_PER_SEC; // Calculate the time taken
  printf("[queue_to_base]: %f seconds\n", time_taken);
  /* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

  /* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
  /* queue movements */
  start = clock();
  queue_to_top(items);
  end = clock();
  time_taken = (double)(end - start) / CLOCKS_PER_SEC; // Calculate the time taken
  printf("[queue_to_top]: %f seconds\n", time_taken);
  /* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

  /* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
  /* queue destruction */
  start = clock();
  queue_destructor(items);
  end = clock();
  time_taken = (double)(end - start) / CLOCKS_PER_SEC; // Calculate the time taken
  printf("[queue_destructor]: %f seconds\n", time_taken);
  /* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */


  return 0;
}