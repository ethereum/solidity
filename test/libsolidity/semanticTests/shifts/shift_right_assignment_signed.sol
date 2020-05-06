contract C {
    function f(int256 a, int256 b) public returns (int256) {
        a >>= b;
        return a;
    }
}

// ====
// compileViaYul: also
// ----
// f(int256,int256): 0x4266, 0x0 -> 0x4266
// f(int256,int256): 0x4266, 0x8 -> 0x42
// f(int256,int256): 0x4266, 0x10 -> 0
// f(int256,int256): 0x4266, 0x11 -> 0
