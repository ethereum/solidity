// Tests that private contract function does not shadow a free function,
// but that internal contract function does.
function foo(uint256 value) pure returns (uint256) { return 1; }
function bar(uint256 value) pure returns (uint256) { return 2; }
contract A {
    function foo(uint256 value) private pure returns (uint256) { return 3; }
    function bar(uint256 value) internal pure returns (uint256) { return 4; }
}
contract B is A {
    using {foo} for uint256;
    function test1(uint256 value) public pure returns (uint256) { return value.foo(); }
    function test2(uint256 value) public pure returns (uint256) { return bar(value); }
}
// ----
// test1(uint256): 0 -> 1
// test2(uint256): 0 -> 4
