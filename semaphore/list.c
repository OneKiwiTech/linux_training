#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

struct test_struct
{
    int val;
    int remain_time;
    struct test_struct *next;
};

struct list_object_struct
{
    struct test_struct *head;
    struct test_struct *curr;
};

struct test_struct* create_list(struct list_object_struct* obj, int val)
{
    printf("\n creating list with headnode as [%d]\n",val);
    struct test_struct *ptr = (struct test_struct*)malloc(sizeof(struct test_struct));
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

struct test_struct* add_to_list(struct list_object_struct* obj, int val, bool add_to_end)
{
    if(NULL == obj->head)
    {
        return (create_list(obj, val));
    }

    if(add_to_end)
        printf("\n Adding node to end of list with value [%d]\n",val);
    else
        printf("\n Adding node to beginning of list with value [%d]\n",val);

    struct test_struct *ptr = (struct test_struct*)malloc(sizeof(struct test_struct));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->val = val;
    ptr->remain_time = 50; //50*100ms = 5s
    ptr->next = NULL;

    if(add_to_end)
    {
        obj->curr->next = ptr;
        obj->curr = ptr;
    }
    else
    {
        ptr->next = obj->head;
        obj->head = ptr;
    }
    return ptr;
}

struct test_struct* search_in_list(struct list_object_struct* obj, int val, struct test_struct **prev)
{
    struct test_struct *ptr = obj->head;
    struct test_struct *tmp = NULL;
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
    struct test_struct *prev = NULL;
    struct test_struct *del = NULL;

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
    struct test_struct *ptr = obj->head;

    printf("\n -------Printing list Start------- \n");
    while(ptr != NULL)
    {
        printf("\n [%d] \n",ptr->val);
        ptr = ptr->next;
    }
    printf("\n -------Printing list End------- \n");

    return;
}


int list_count_down(struct list_object_struct* obj)
{
    struct test_struct *ptr = obj->head;

    while(ptr != NULL)
    {
        ptr->remain_time--;
        if (ptr->remain_time == 0)
        {
            return ptr->val; //return customer id
        }
        ptr = ptr->next;
    }
    
    return -1;
}

/* 
int main(void)
{
    int i = 0, ret = 0;
    struct test_struct *ptr = NULL;

    print_list();

    for(i = 5; i<10; i++)
        add_to_list(i,true);

    print_list();

    for(i = 4; i>0; i--)
        add_to_list(i,false);

    print_list();

    for(i = 1; i<10; i += 4)
    {
        ptr = search_in_list(i, NULL);
        if(NULL == ptr)
        {
            printf("\n Search [val = %d] failed, no such element found\n",i);
        }
        else
        {
            printf("\n Search passed [val = %d]\n",ptr->val);
        }

        print_list();

        ret = delete_from_list(i);
        if(ret != 0)
        {
            printf("\n delete [val = %d] failed, no such element found\n",i);
        }
        else
        {
            printf("\n delete [val = %d]  passed \n",i);
        }

        print_list();
    }

    return 0;
}
*/