contract C {
    function f(uint x) public returns(uint a) {
        assembly {
            a := byte(x, 31)
        }
    }

    function g(uint x) public returns(uint a) {
        assembly {
            a := byte(31, x)
        }
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 2 -> 0
// g(uint256): 2 -> 2
