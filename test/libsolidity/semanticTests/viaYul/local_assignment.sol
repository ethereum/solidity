contract C {
    function f(uint a) public pure returns (uint x) {
        uint b = a;
        x = b;
    }
}
// ====
// compileViaYul: true
// ----
// f(uint256): 6 -> 6
