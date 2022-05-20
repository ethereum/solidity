contract C {
    function f(int a, int b) public pure returns (int) {
        return a % b;
    }
    function g(bool _check) public pure returns (int) {
        int x = type(int).min;
        if (_check) {
            return x / -1;
        } else {
            unchecked { return x / -1; }
        }
    }
}

// ====
// compileToEwasm: also
// ----
// f(int256,int256): 7, 5 -> 2
// f(int256,int256): 7, -5 -> 2
// f(int256,int256): -7, 5 -> -2
// f(int256,int256): -7, 5 -> -2
// f(int256,int256): -5, -5 -> 0
// g(bool): true -> FAILURE, hex"4e487b71", 0x11
// g(bool): false -> -57896044618658097711785492504343953926634992332820282019728792003956564819968
