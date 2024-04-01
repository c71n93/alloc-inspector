// 8 + 4 = 12 total stack allocations (gcc -O0)

struct S {
    int a = 1;
    int b = 0;
};

// 1
int foo() {
    return 1;
}

// 1[init] + 1[variables] + (1 + 1)[call foo] = 4
int main() {
    int a = foo();
    struct S s = {a, a};
    return s.a + s.b;
}
