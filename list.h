#ifndef _LIST_H_
#define _LIST_H_

#include <stddef.h>

struct list_head {
  struct list_head *prev;
  struct list_head *next;
};

#define container_of(ptr, type, member) ({                      \
      const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
      (type *)( (char *)__mptr - offsetof(type,member) );})

void list_init(struct list_head *list);
int list_is_empty(struct list_head *list);
void list_insert_before(struct list_head *new, struct list_head *old);
void list_remove_item(struct list_head *item);

#endif  // _LIST_H_
