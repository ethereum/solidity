contract C {
    function f(uint256 a, int8 b) public returns (uint256) {
        assembly { b := 0xff }
        return a << b;
    }
    function g(uint256 a, int8 b) public returns (uint256) {
        assembly { b := 0xff }
        return a >> b;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256,int8): 0x1234, 0x0 -> FAILURE
// g(uint256,int8): 0x1234, 0x0 -> FAILURE
