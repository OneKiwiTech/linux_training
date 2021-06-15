#ifndef LINK_LIST_H
#define LINK_LIST_H

struct test_struct* create_list(int val);
struct test_struct* add_to_list(int val, bool add_to_end);
struct test_struct* search_in_list(int val, struct test_struct **prev);
int delete_from_list(int val);
void print_list(void);
int list_count_down(void);

#endif 