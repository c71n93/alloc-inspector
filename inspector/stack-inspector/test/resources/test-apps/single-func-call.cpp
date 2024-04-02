// 8 + 3 = 11 total stack allocations (gcc -O0)

// 1
int foo() {
    return 1;
}

// 1[init] + (1 + 1)[call foo] = 3
int main() {
    return foo();
}
