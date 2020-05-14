contract C {
    function f(int256 a, int256 b) public returns (int256) {
        a <<= b;
        return a;
    }

    function g(int256 a, int256 b) public returns (int256) {
        a >>= b;
        return a;
    }
}

// ====
// compileViaYul: also
// ----
// f(int256,int256): 1, -1 -> FAILURE
// g(int256,int256): 1, -1 -> FAILURE
