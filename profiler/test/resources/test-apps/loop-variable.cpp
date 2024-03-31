// TODO: why result stack allocations are 9 like int single-main.cpp?
int main() {
    int a = 1;
    for (int i = 0; i < 10; ++i) {
        int b = 1;
        a += b;
    }
    return a;
}