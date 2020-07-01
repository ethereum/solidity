library Lib {
    function foo(uint256 value) internal returns (uint256) {
        return value + 42;
    }
}

contract A {
    using Lib for uint256;
}

contract B is A {
    function bar(uint256 value) public returns (uint256) {
        return value.foo(); // Usage of Lib
    }
}
// ----
// TypeError 9582: (246-255): Member "foo" not found or not visible after argument-dependent lookup in uint256.
