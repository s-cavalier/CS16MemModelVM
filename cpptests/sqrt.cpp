int main(void) {
    register unsigned int n = 0;

    if (n <= 0) return 0;
    if (n == 1) return 1;
    register unsigned int a = 0, b = 1, result;
    for (unsigned i = 2; i < n; ++i) {
        result = a + b;
        a = b;
        b = result;
    }
}