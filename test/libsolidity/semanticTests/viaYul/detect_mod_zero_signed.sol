contract C {
    function f(int a, int b) public pure returns (int x) {
        x = a % b;
    }
    function g(int8 a, int8 b) public pure returns (int8 x) {
        x = a % b;
    }
}
// ====
// compileViaYul: only
// ----
// f(int256,int256): 10, 3 -> 1
// f(int256,int256): 10, 2 -> 0
// f(int256,int256): 11, 2 -> 1
// f(int256,int256): -10, 3 -> -1
// f(int256,int256): 10, -3 -> 1
// f(int256,int256): -10, -3 -> -1
// f(int256,int256): 2, 2 -> 0
// f(int256,int256): 1, 0 -> FAILURE
// f(int256,int256): -1, 0 -> FAILURE
// f(int256,int256): 0, 0 -> FAILURE
// f(int256,int256): 0, 1 -> 0
// f(int256,int256): 0, -1 -> 0
// g(int8,int8): 10, 3 -> 1
// g(int8,int8): 10, 2 -> 0
// g(int8,int8): 11, 2 -> 1
// g(int8,int8): -10, 3 -> -1
// g(int8,int8): 10, -3 -> 1
// g(int8,int8): -10, -3 -> -1
// g(int8,int8): 2, 2 -> 0
// g(int8,int8): 1, 0 -> FAILURE
// g(int8,int8): -1, 0 -> FAILURE
// g(int8,int8): 0, 0 -> FAILURE
// g(int8,int8): 0, 1 -> 0
// g(int8,int8): 0, -1 -> 0
