contract C {
    function f(uint256 x) public returns (uint256) {
        return eval(g, x);
    }

    function eval(function(uint) internal returns (uint) x, uint a) internal returns (uint) {
        return x(a);
    }

    function g(uint256 x) public pure returns (uint256) {
        return x + 1;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint256): 7 -> 8
