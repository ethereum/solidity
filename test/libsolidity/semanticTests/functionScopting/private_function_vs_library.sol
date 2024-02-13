// Tests that private library functions are not overridden or visible contracts,
// and that that internal library functions do.
library L {
    function foo(uint256 value) private pure returns (uint256) { return 1; }
    function bar(uint256 value) internal pure returns (uint256) { return foo(value); }
}
contract A {
    function foo(uint256 value) private pure returns (uint256) { return 2; }
}
contract B is A {
    using L for uint256;
    function foo(uint256 value) private pure returns (uint256) { return 3; }
    function test1(uint256 value) public pure returns (uint256) { return value.bar(); }
    function test2(uint256 value) public pure returns (uint256) { return foo(0); }
}
// ----
// library: L
// test1(uint256): 0 -> 1
