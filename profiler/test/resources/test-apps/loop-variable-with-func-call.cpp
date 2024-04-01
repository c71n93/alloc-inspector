// 8 + 22 = 30 total stack allocations (gcc -O0)

// 1
int foo() {
    return 1;
}

// 1[init] + 1[variables] + 10 * (1 + 1)[call foo] = 22
int main() {
    int a = 1;
    for (int i = 0; i < 10; ++i) {
        a += foo();
    }
    return a;
}
