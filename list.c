#include "list.h"

void list_init(struct list_head *list) {
  list->next = list->prev = list;
}

int list_is_empty(struct list_head *list) {
  return list->next == list;
}

void list_insert_before(struct list_head *new, struct list_head *old) {
  new->next = old;
  new->prev = old->prev;
  new->prev->next = new;
  old->prev = new;
}

void list_remove_item(struct list_head *item) {
  struct list_head *next = item->next;
  next->prev = item->prev;
  next->prev->next = next;
}
