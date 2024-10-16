// Tests that private functions are not overridden by inheriting contract's functions.
contract A {
    function foo() private pure returns (uint256) { return 1; }
    function foo(uint256 value) private pure returns (uint256) { return 2; }
    function test1() public pure returns (uint256) { return foo(); }
    function test2() public pure returns (uint256) { return foo(0); }
}
contract B is A {
    function foo() private pure returns (uint256) { return 3; }
    function foo(uint256 value) private pure returns (uint256) { return 4; }
    function test3() public pure returns (uint256) { return foo(); }
    function test4() public pure returns (uint256) { return foo(0); }
}
// ----
// test1() -> 1
// test2() -> 2
// test3() -> 3
// test4() -> 4
