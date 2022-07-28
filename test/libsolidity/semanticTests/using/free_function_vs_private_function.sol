function foo(uint256 value) pure returns (uint256) {
    return value + 1;
}
contract A {
    function foo(uint256 value) private pure returns (uint256) {
        return value - 1;
    }
}
contract B is A {
    using {foo} for uint256;
    function bar(uint256 value) public pure returns (uint256) {
        return value.foo();
    }
}
// ----
// bar(uint256): 1 -> 2
