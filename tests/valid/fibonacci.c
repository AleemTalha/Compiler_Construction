/* Nested blocks and complex expressions */
int fibonacci(int n) {
    int a = 0;
    int b = 1;
    int temp;
    while (n > 0) {
        temp = a + b;
        a = b;
        b = temp;
        n = n - 1;
    }
    return a;
}

int main() {
    int x;
    input(x);
    int result = fibonacci(x);
    output(result);
    return 0;
}
