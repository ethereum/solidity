library L {
    function foo(uint256 value) private pure returns (uint256) { return 1; }
    function bar(uint256 value) public pure returns (uint256) { return foo(value); }
}
contract A {
    function foo(uint256 value) private pure returns (uint256) { return 2; }
}
contract B is A {
    using L for uint256;
    function foo(uint256 value) private pure returns (uint256) { return 3; }
    function test(uint256 value) public pure returns (uint256) { return value.bar(); }
}
// ====
// compileToEwasm: false
// ----
// library: L
// test(uint256): 0 -> 1
