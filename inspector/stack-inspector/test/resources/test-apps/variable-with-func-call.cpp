// 8 + 4 = 12 total stack allocations (gcc -O0)

// 1
int foo() {
    return 1;
}

// 1[init] + 1[variables] + (1 + 1)[call foo] = 4
int main() {
    int a = foo();
    return a;
}
