contract C {
    type T is uint;
}
contract D {
    function f(C.T x) public pure returns(uint) {
        return C.T.unwrap(x);
    }
    function g(uint x) public pure returns(C.T) {
        return C.T.wrap(x);
    }
    function h(uint x) public pure returns(uint) {
        return f(g(x));
    }
    function i(C.T x) public pure returns(C.T) {
        return g(f(x));
    }
}
// ----
// f(uint256): 0x42 -> 0x42
// g(uint256): 0x42 -> 0x42
// h(uint256): 0x42 -> 0x42
// i(uint256): 0x42 -> 0x42
