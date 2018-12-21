contract test {
    function f(uint a) public returns(uint d) { return 2 ** a; }
}
// f(uint256): 1
// -> 2
