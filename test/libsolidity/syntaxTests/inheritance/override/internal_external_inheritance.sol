contract A {
    function f(uint[] calldata) external pure {}
}
contract B {
    function f(uint[] memory) internal pure {}
}
contract C is A, B {}
// ----
// TypeError: (126-147): Derived contract must override function "f". Function with the same name and parameter types defined in two or more base classes.
