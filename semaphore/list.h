#ifndef LINK_LIST_H
#define LINK_LIST_H

struct test_struct* create_list(struct list_object_struct* obj, int val);
struct test_struct* add_to_list(struct list_object_struct* obj, int val, bool add_to_end);
struct test_struct* search_in_list(struct list_object_struct* obj, int val, struct test_struct **prev);
int delete_from_list(struct list_object_struct* obj, int val);
void print_list(struct list_object_struct* obj);
int list_count_down(struct list_object_struct* obj);


#endif 