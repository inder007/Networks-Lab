//utility file for data structures and file I/O

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int ll_id;

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
    int ll_id;
} list;

//function to create node
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

//bool to check if list is empty
int is_empty(struct list *l){
    if(l->head == NULL){
        return 1;
    }
    return 0;
}

//function to clear file
void clear_files(){
    char file_name[50];
    for(int i=1; i<=25; i++){
        memset(file_name, 0, sizeof(file_name));
        sprintf(file_name, "./database/ll_%d", i);
        FILE *fp = fopen(file_name, "w");
        fclose(fp);
    }
}

//function to load linked list in file
void load_file(list* temp){
    node* temp_node = temp->head;
    FILE *fp;
    char file_name[50];
    memset(file_name, 0, sizeof(file_name));
    sprintf(file_name, "./database/ll_%d", temp->ll_id);
    // printf("%s", file_name);
    fp = fopen(file_name, "w");
    while(temp_node!=NULL){
        fprintf(fp, "%d %d %d %d %c\n", (temp_node->quant), (temp_node->t_id), (temp_node->i_id), (temp_node->price), (temp_node->bors));
        temp_node = temp_node->next;
    }
    fclose(fp);
}

void insert_last(struct list* l, int quant, int t_id, int i_id, int price, char bors){
    struct node* temp = create_node(quant, t_id, i_id, price, bors);
    if(l->head==NULL){
        l->head = temp;
    } else {
        struct node* temp_head = l->head;
        while(temp_head->next!=NULL){
            temp_head = temp_head->next;
        }
        temp_head->next = temp;
    }
}

//function to insert in linked list
void insert(struct list* l, int quant, int t_id, int i_id, int price, char bors){
    struct node* temp = create_node(quant, t_id, i_id, price, bors);
    if(l->head==NULL){
        l->head = temp;
    } else {
        temp->next = l->head;
        l->head = temp;
    }
    load_file(l);
}

//function to delete node
void delete(struct list* l, node* x){
    if(is_empty(l)){
        // printf("list is empty\n");
        return;
    }
    struct node* temp = l->head;
    if(temp == x){
        l->head = temp->next;
        free(temp);
        load_file(l);
        return;
    }
    struct node* prev = l->head;
    temp = temp->next;
    while(temp!=NULL){
        if(temp == x){
            prev->next = temp->next;
            free(temp);
            load_file(l);
            return;
        }
        temp = temp->next;
        prev = prev->next;
    }
    load_file(l);
}

//function to populate file in linked list
void load_list(list* temp){
    node* temp_node = create_node(0, 0, 0, 0, 0);
    FILE *fp;
    char file_name[50];
    memset(file_name, 0, sizeof(file_name));
    sprintf(file_name, "./database/ll_%d", temp->ll_id);
    // printf("%s\n", file_name);
    fp = fopen(file_name, "r");
    while(fscanf(fp, "%d %d %d %d %c", &(temp_node->quant), &(temp_node->t_id), &(temp_node->i_id), &(temp_node->price), &(temp_node->bors))!=EOF){
        insert_last(temp, (temp_node->quant), (temp_node->t_id), (temp_node->i_id), (temp_node->price), (temp_node->bors));
	}
    fclose(fp);
}

//function to create list
struct list* create_list(){
    struct list* temp = malloc(sizeof(struct list));
    temp->head = NULL;
    temp->ll_id = ll_id;
    load_list(temp);
    ll_id++;
    return temp;
}

//function to find minimum element in the list
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

//function to find maximum element in the list
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

//function to search element in linked list
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

//function to print list
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
