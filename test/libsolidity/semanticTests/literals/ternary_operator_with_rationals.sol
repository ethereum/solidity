contract C {
    function f(int a, int b) public returns (int) {
        return a < b ? -1 : 1;
    }

    function g(int a) public returns (int) {
        int8 b = -1;
        return a < 0 ? b : 1;
    }

    function h(int a) public returns (int) {
        int8 b = -1;
        return a < 0 ? 1 : b;
    }
}
// ----
// f(int256,int256): 0, 2 -> -1
// f(int256,int256): 2, 0 -> 1
// g(int256): -2 -> -1
// g(int256): 2 -> 1
// h(int256): -2 -> 1
// h(int256): 2 -> -1
