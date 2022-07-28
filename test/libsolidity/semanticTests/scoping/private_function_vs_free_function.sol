function foo(uint256 value) pure returns (uint256) { return 1; }
contract A {
    function foo(uint256 value) private pure returns (uint256) { return 2; }
}
contract B is A {
    using {foo} for uint256;
    function test(uint256 value) public pure returns (uint256) { return value.foo(); }
}
// ----
// test(uint256): 0 -> 1
