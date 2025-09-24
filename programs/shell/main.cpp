#include "../ministl/iostream.h"

struct Node {

    Node(int val) : val(val) {}

    Node* next;
    int val;
};


void func(Node& n) {
    
}

int main() {


    Node* head = new Node(0);
    head->next = new Node(1);
    head->next->next = new Node(2);
    head->next->next->next = new Node(3);
    head->next->next->next->next = nullptr;
    

    for (Node* it = head; it; it = it->next) {
        std::cout << "Node value: " << it->val << std::endl;
    }



    return 0;
}