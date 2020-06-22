contract A {
    function f(uint[] calldata) external pure {}
}
contract B {
    function f(uint[] memory) internal pure {}
}
contract C is A, B {}
// ----
// TypeError 6480: (126-147): Derived contract must override function "f". Two or more base classes define function with same name and parameter types.
