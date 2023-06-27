// Tests that private contract functions are not overridden or visible by inheriting contracts,
// and that that functions functions do.
contract A {
    function foo() private pure returns (uint256) { return 1; }
    function bar() internal pure returns (uint256) { return foo(); }
}
contract B is A {
    function foo() private pure returns (uint256) { return 2; }
    function test1() public pure returns (uint256) { return bar(); }
    function test2() public pure returns (uint256) { return foo(); }
}
contract C is B {
    function foo() public pure returns (uint256) { return 3; }
}
// ----
// test1() -> 1
// test2() -> 2
// foo() -> 3
