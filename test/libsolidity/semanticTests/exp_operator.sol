contract test {
    function f(uint a) public returns(uint d) { return 2 ** a; }
    function g(uint a, uint b) public returns(uint c) { return a ** b; }
}
// ----
// f(uint256): 1 
// -> 2
// g(uint256, uint256): 1, 1
// REVERT # Argument encoding is not properly implemented, yet.
