#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include "list.h"

struct customer_info_obj* create_list(struct list_object_struct* obj, int val)
{
    printf("\n creating list with headnode as [%d]\n",val);
    struct customer_info_obj *ptr = (struct customer_info_obj*)malloc(sizeof(struct customer_info_obj));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->val = val;
    ptr->next = NULL;

    obj->head = obj->curr = ptr;
    return ptr;
}

struct customer_info_obj* add_to_list(struct list_object_struct* obj, int val)
{
    if(NULL == obj->head)
    {
        return (create_list(obj, val));
    }

    struct customer_info_obj *ptr = (struct customer_info_obj*)malloc(sizeof(struct customer_info_obj));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->val = val;
    ptr->remain_time = 50; //50*100ms = 5s
    ptr->next = NULL;

    {
        obj->curr->next = ptr;
        obj->curr = ptr;
    }

    return ptr;
}

struct customer_info_obj* search_in_list(struct list_object_struct* obj, int val, struct customer_info_obj **prev)
{
    struct customer_info_obj *ptr = obj->head;
    struct customer_info_obj *tmp = NULL;
    bool found = false;

    printf("\n Searching the list for value [%d] \n",val);

    while(ptr != NULL)
    {
        if(ptr->val == val)
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

int delete_from_list(struct list_object_struct* obj, int val)
{
    struct customer_info_obj *prev = NULL;
    struct customer_info_obj *del = NULL;

    printf("\n Deleting value [%d] from list\n",val);

    del = search_in_list(obj, val,&prev);
    if(del == NULL)
    {
        return -1;
    }
    else
    {
        if(prev != NULL)
            prev->next = del->next;

        if(del == obj->curr)
        {
            obj->curr = prev;
        }
        else if(del == obj->head)
        {
            obj->head = del->next;
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
        printf("\n [%d] \n",ptr->val);
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
            return ptr->val; //return customer id
        }
        ptr = ptr->next;
    }
    
    return -1;
}