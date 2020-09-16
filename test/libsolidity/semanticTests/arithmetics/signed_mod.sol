contract C {
    function f(int a, int b) public pure returns (int) {
        return a % b;
    }
}

// ====
// compileViaYul: also
// ----
// f(int256,int256): 7, 5 -> 2
// f(int256,int256): 7, -5 -> 2
// f(int256,int256): -7, 5 -> -2
// f(int256,int256): -7, 5 -> -2
// f(int256,int256): -5, -5 -> 0
