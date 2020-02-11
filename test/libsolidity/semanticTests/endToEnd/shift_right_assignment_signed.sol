contract C {
    function f(int a, int b) public returns(int) {
        a >>= b;
        return a;
    }
}

// ----
// f(int256,int256): 0x4266, 0 -> 0x4266
// f(int256,int256): 0x4266, 8 -> 0x42
// f(int256,int256): 0x4266, 16 -> 0
// f(int256,int256): 0x4266, 17 -> 0
