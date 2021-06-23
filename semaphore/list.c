#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include "list.h"

// https://www.thegeekstuff.com/2012/08/c-linked-list-example/
struct customer_info_obj* create_list(struct list_object_struct* obj, int id)
{
    printf("\n creating list with headnode as [%d]\n",id);
    struct customer_info_obj *ptr = (struct customer_info_obj*)malloc(sizeof(struct customer_info_obj));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->id = id;
    ptr->next = NULL;

    obj->head = obj->curr = ptr;

    return ptr;
}

struct customer_info_obj* add_to_list(struct list_object_struct* obj, int id, bool has_scanner)
{
    printf("add_to_list");
    if(NULL == obj->head)
    {
        printf("Create list\n");
        return create_list(obj, id);
    }

    printf("call malloc\n");
    
    struct customer_info_obj *ptr = (struct customer_info_obj*)malloc(sizeof(struct customer_info_obj));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        exit(0);
    }


    ptr->id = id;
    ptr->remain_time = obj->meta_data; //50*100ms = 5s
    ptr->next = NULL;

     printf("Add to end of list\n");
    // Add to the end of list
    {
        obj->curr->next = ptr;
        obj->curr = ptr;
    }

    return ptr;
}

struct customer_info_obj* search_in_list(struct list_object_struct* obj, int id, struct customer_info_obj **prev)
{
    struct customer_info_obj *ptr = obj->head;
    struct customer_info_obj *tmp = NULL;
    bool found = false;

    printf("\n Searching the list for idue [%d] \n",id);

    while(ptr != NULL)
    {
        if(ptr->id == id)
        {
            found = true;
            break;
        }
        else
        {
            tmp = ptr;
            ptr = ptr->next;
        }
    }

    if(true == found)
    {
        if(prev)
            *prev = tmp;
        return ptr;
    }
    else
    {
        return NULL;
    }
}

int delete_from_list(struct list_object_struct* list_obj, struct customer_info_obj* cust_obj)
{
    struct customer_info_obj *prev = NULL;
    struct customer_info_obj *del = NULL;

    printf("\n Deleting id [%d] from list\n", cust_obj->id);

    del = search_in_list(list_obj, cust_obj->id, &prev);
    if(del == NULL)
    {
        return -1;
    }
    else
    {
        if(prev != NULL)
            prev->next = del->next;

        if(del == list_obj->curr)
        {
            list_obj->curr = prev;
        }
        else if(del == list_obj->head)
        {
            list_obj->head = del->next;
        }
    }

    free(del);
    del = NULL;

    return 0;
}

void print_list(struct list_object_struct* obj)
{
    struct customer_info_obj *ptr = obj->head;

    printf("\n -------Printing list Start------- \n");
    while(ptr != NULL)
    {
        printf("\n [%d] \n",ptr->id);
        ptr = ptr->next;
    }
    printf("\n -------Printing list End------- \n");

    return;
}


int list_count_down(struct list_object_struct* obj, struct customer_info_obj *curr_cust_obj )
{
    struct customer_info_obj *ptr = obj->head;

    while(ptr != NULL)
    {
        ptr->remain_time--;
        if (ptr->remain_time == 0)
        {
            curr_cust_obj = ptr;
            return 1; 
        }
        ptr = ptr->next;
    }
    
    return 0;
}