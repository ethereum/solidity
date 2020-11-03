contract C {
    function f(int8 a, uint8 b) public returns (int256) {
        return a >> b;
    }
}

// ====
// compileViaYul: also
// ----
// f(int8,uint8): -66, 0 -> -66
// f(int8,uint8): -66, 1 -> -33
// f(int8,uint8): -66, 4 -> -5
// f(int8,uint8): -66, 8 -> -1
// f(int8,uint8): -66, 16 -> -1
// f(int8,uint8): -66, 17 -> -1
// f(int8,uint8): -67, 0 -> -67
// f(int8,uint8): -67, 1 -> -34
// f(int8,uint8): -67, 4 -> -5
// f(int8,uint8): -67, 8 -> -1
// f(int8,uint8): -67, 16 -> -1
// f(int8,uint8): -67, 17 -> -1
