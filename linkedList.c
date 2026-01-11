#include <stdio.h>
#include <stdlib.h>

typedef struct list {
    int value;
    struct list* next;
} List;

List* createLL(int data) {
    List* newNode = malloc(sizeof(List));
    if (!newNode) {
        printf("error");
        exit(1);
    }
    newNode->value = data;
    newNode->next = NULL;
    return newNode;
}

List* insertLL(List* list, int n){
    List* newNode = createLL(n);
    if (list == NULL) return newNode; //first node in the list

    List* aux = list;
    while(aux->next != NULL) {
        aux = aux->next;
    }
    aux->next = newNode;
    return list;
}

void deleteLL(List* list) {
    List* aux;
    while (list != NULL) {
        aux = list;
        list = list->next;
        free(aux);
    }
}

void displayLL(List* list, int n) {
    List* current = list;
    while(current != NULL) {
        printf("%d -> ", current->value);
        current = current->next;
    }
    printf("NULL\n");
}

int main() {
    List* list = NULL;
    int size = 0;
    printf("insert the size: ");
    scanf("%d", &size);
    getchar();
    for (int i = 0; i < size; i++) { //each node is allocated automatically
        list = insertLL(list, 0);
    }
    displayLL(list, size);
    deleteLL(list);
    return 0;
}