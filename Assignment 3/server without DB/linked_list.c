#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct node{
    struct node* next;
    int quant;
    int t_id;
    int i_id;
    int price;
    char bors;
} node;

typedef struct list{
    struct node *head;
} list;

struct list* create_list(){
    struct list* temp = malloc(sizeof(struct list));
    temp->head = NULL;
    return temp;
}

struct node* create_node(int quant, int t_id, int i_id, int price, char bors){
    struct node* temp = malloc(sizeof(struct node));
    temp->next = NULL;
    temp->quant = quant;
    temp->t_id = t_id;
    temp->i_id = i_id;
    temp->price = price;
    temp->bors = bors;
    return temp;
}

void insert(struct list* l, int quant, int t_id, int i_id, int price, char bors){
    struct node* temp = create_node(quant, t_id, i_id, price, bors);
    if(l->head==NULL){
        l->head = temp;
    } else {
        temp->next = l->head;
        l->head = temp;
    }
}


int is_empty(struct list *l){
    if(l->head == NULL){
        return 1;
    }
    return 0;
}

struct node* min_search(struct list* l){
    if(is_empty(l)){
        // printf("list is empty\n");
        return NULL;
    }
    struct node* temp = l->head;
    struct node* min_temp = l->head; 
    int min_price = INT_MAX;
    while(temp!=NULL){
        if(temp->price <= min_price){
            min_temp = temp;
            min_price = temp->price;
        }
        temp = temp->next;
    }
    return min_temp;
}

struct node* max_search(struct list* l){
    if(is_empty(l)){
        // printf("list is empty\n");
        return NULL;
    }
    struct node* temp = l->head;
    struct node* max_temp = l->head; 
    int max_price = INT_MIN;
    while(temp!=NULL){
        if(temp->price >= max_price){
            max_temp = temp;
            max_price = temp->price;
        }
        temp = temp->next;
    }
    return max_temp;
}

struct node* search(struct list* l, int x){
    if(is_empty(l)){
        // printf("list is empty\n");
        return NULL;
    }
    struct node* temp = l->head;
    while(temp!=NULL){
        if(temp->quant == x){
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}


void delete(struct list* l, node* x){
    if(is_empty(l)){
        // printf("list is empty\n");
        return;
    }
    struct node* temp = l->head;
    if(temp == x){
        l->head = temp->next;
        free(temp);
        return;
    }
    struct node* prev = l->head;
    temp = temp->next;
    while(temp!=NULL){
        if(temp == x){
            prev->next = temp->next;
            free(temp);
            return;
        }
        temp = temp->next;
        prev = prev->next;
    }
}

void print_list(struct list* l){
    if(is_empty(l)){
        printf("list is empty\n");
        return;
    }
    struct node* temp = l->head;
    while(temp!=NULL){
        printf("%d ", temp->price);
        temp = temp->next;
    }
    printf("\n");
}


// int main(){
//     struct list* l = create_list();
//     char ch;
//     while(1){
//         scanf("%c", &ch);
//         if(ch=='i'){
//             int x;
//             scanf("%d", &x);
//             insert(l, 0, 0, 0, x, 0);
//         } else if(ch=='d'){
//             int x;
//             scanf("%d", &x);
//             // delete(l, x);
//         } else if(ch=='s'){
//             int x;
//             scanf("%d", &x);
//             // search(l, x);
//         } else if(ch=='p'){
//             print_list(l);
//         } else if(ch=='e'){
//             return 0;
//         } else if(ch=='l'){
//             printf("%d\n", min_search(l)->price);
//         } else if(ch=='h'){
//             printf("%d\n", max_search(l)->price);
//         }
//     }
// }
