#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include "list.h"
#include "my_list.h"

list_object_struct_t*  create_list(struct list_head* head, int id, int metadata)
{
    list_object_struct_t* ptr = NULL;

    ptr = malloc(sizeof(*ptr));

    ptr->id = id;
    ptr->meta_data = metadata;

    INIT_LIST_HEAD(&ptr->list);
    INIT_LIST_HEAD(&ptr->customer_list);
    list_add_tail(&ptr->list, head);

    return ptr;
}

void  add_to_list(list_object_struct_t* list_head, int id, bool has_scanner)
{
    customer_info_obj_t *eptr = NULL;
    
    eptr = malloc(sizeof(*eptr));
    eptr->id = id;
    INIT_LIST_HEAD(&eptr->list);
    list_add_tail(&eptr->list, &(list_head->customer_list) );
}

void delete_from_list(struct list_head* head, list_object_struct_t *list_ptr, customer_info_obj_t* ptr)
{
	list_object_struct_t *next;
	list_for_each_entry_safe(ptr, next, head, list) {
		printf("Removing – pid: %d\n", ptr->id);

		customer_info_obj_t *enext, *eptr;
		list_for_each_entry_safe(eptr, enext, &(list_ptr->customer_list), list) {
			printf("Removing – epid: %d\n", eptr->id);
			list_del(&eptr->list);
			free(eptr);
		}
		
		list_del(&ptr->list);
		free(ptr);
	}
}

void print_list(struct list_head* head)
{
    list_object_struct_t* ptr;

    list_for_each_entry(ptr, head, list){
        // printf("customer id: %d\n", ptr->id);
        customer_info_obj_t *eptr;
        list_for_each_entry(eptr, &ptr->customer_list, list) {
            printf("epid: %d\n", eptr->id);
        }
    }
}


customer_info_obj_t* list_count_down(struct list_head* head, list_object_struct_t* ptr)
{
    list_for_each_entry(ptr, head, list){
        // printf("customer id: %d\n", ptr->id);
        customer_info_obj_t *eptr;
        list_for_each_entry(eptr, &ptr->customer_list, list) {
            // printf("epid: %d\n", eptr->id);
            eptr->remain_time = eptr->remain_time - 1;
            if (eptr->remain_time == 0){
                return eptr;
            }
        }
    }

    return NULL;
}