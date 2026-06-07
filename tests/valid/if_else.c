int max(int a, int b) {
    int result;
    if (a > b) {
        result = a;
    } else {
        result = b;
    }
    return result;
}

int main() {
    int x = 10;
    int y = 20;
    int m = max(x, y);
    output(m);
    return 0;
}
