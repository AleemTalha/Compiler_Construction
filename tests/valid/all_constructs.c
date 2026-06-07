int main() {
    int a = 5;
    int b = 3;
    int c = a + b * 2;
    bool flag = (c > 10) && (a < b);
    if (flag) {
        output(a);
    } else {
        output(b);
    }
    return 0;
}
