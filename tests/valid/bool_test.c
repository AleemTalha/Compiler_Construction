int main() {
    bool flag = true;
    bool other = false;
    bool result;

    if (flag && other) {
        result = true;
    } else {
        result = false;
    }
    output(result);

    while (flag) {
        flag = false;
    }

    if (!flag) {
        output(flag);
    }

    bool a = true;
    bool b = false;
    bool c = a || b;
    output(c);

    return 0;
}
