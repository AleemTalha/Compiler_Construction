/* Factorial program */
int factorial(int n) {
    int result = 1;
    while (n > 0) {
        result = result * n;
        n = n - 1;
    }
    return result;
}

int main() {
    int x;
    input(x);
    int fact = factorial(x);
    output(fact);
    return 0;
}
