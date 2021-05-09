library Lib {
    function foo(uint256 value) internal pure returns (uint256) {
        return value + 42;
    }
}

contract A {
    using Lib for uint256;
}

contract B is A {
    using Lib for uint256;
    function bar(uint256 value) public pure returns (uint256) {
        return value.foo(); // Usage of Lib
    }
}
// ----
