library Lib {
    function m(uint x, uint y) public returns(uint) {
        return x * y;
    }
}
contract Test {
    function f(uint x) public returns(uint) {
        return Lib.m(x, 9);
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
        return Lib.m(x, 9);
    }
}

// ----
// f(uint256): 33) -> 33 * 9
// f(uint256):"33" -> "297"
