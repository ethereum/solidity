library Lib { function m(uint x, uint y) returns (uint) { return x * y; } }
contract Test {
    function f(uint x) returns (uint) {
        return Lib.m(x, 9);
    }
}
// ----
// DEPLOYLIB: Lib
// f(uint256): 33
// -> 297
