library Lib {
    function m(uint x, uint y) public returns(uint) {
        return x * y;
    }
}
contract Test {
    function f(uint x) public returns(uint) {
        Lib;
        Lib.m;
        return x + 9;
    }
}

// ----

library Lib {
    function m(uint x, uint y) public returns(uint) {
        return x * y;
    }
}
contract Test {
    function f(uint x) public returns(uint) {
        Lib;
        Lib.m;
        return x + 9;
    }
}

// ----
// f(uint256): 33) -> 42
// f(uint256):"33" -> "42"
