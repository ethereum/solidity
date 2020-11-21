library L {
    function add(uint256 a, uint256 b) internal pure returns (uint256) {
        return a + b;
    }
}

contract C {
    using L for uint256;

    function foo(uint256 a, uint256 b) public returns (uint256) {
        return a.add(b);
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// foo(uint256,uint256): 8, 42 -> 50
