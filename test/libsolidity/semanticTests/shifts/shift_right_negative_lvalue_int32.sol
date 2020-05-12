contract C {
    function f(int32 a, int32 b) public returns (int256) {
        return a >> b;
    }
}

// ====
// compileViaYul: also
// ----
// f(int32,int32): -4266, 0 -> -4266
// f(int32,int32): -4266, 1 -> -2133
// f(int32,int32): -4266, 4 -> -267
// f(int32,int32): -4266, 8 -> -17
// f(int32,int32): -4266, 16 -> -1
// f(int32,int32): -4266, 17 -> -1
// f(int32,int32): -4267, 0 -> -4267
// f(int32,int32): -4267, 1 -> -2134
// f(int32,int32): -4267, 4 -> -267
// f(int32,int32): -4267, 8 -> -17
// f(int32,int32): -4267, 16 -> -1
// f(int32,int32): -4267, 17 -> -1
