

int main() {
    struct Node {
        int data;
        Node* next;
    };

    Node n1 = {0, 0};
    Node n2 = {1, &n1};
    Node n3 = {2, &n2};

    for (Node* it = &n1; it; it = it->next) ++it->data;

    __asm__(
        "li $v0, 10\n"
        "syscall"
    );
}