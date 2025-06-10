#include "../ministl/stdio.h"

struct Node {
    int data;
    Node* next;
};

void printForwards(Node* head) {
    if (!head) return;
    std::printInteger(head->data);
    std::printString("\n");
    printForwards(head->next);
}

void printBackwards(Node* head) {
    if (!head) return;
    printBackwards(head->next);
    std::printInteger(head->data);
    std::printString("\n");
}

int main() {
    Node n1{1, nullptr};
    Node n2{2, &n1};
    Node n3{3, &n2};
    Node n4{4, &n3};
    Node n5{5, &n4};
    Node n6{6, &n5};
    Node n7{7, &n6};
    Node n8{8, &n7};
    Node n9{9, &n8};
    
    printBackwards(&n9);
    printForwards(&n9);

    std::exit();
}