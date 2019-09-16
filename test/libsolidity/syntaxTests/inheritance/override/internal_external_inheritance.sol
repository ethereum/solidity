contract A {
    function f(uint[] calldata) external pure {}
}
contract B {
    function f(uint[] memory) internal pure {}
}
contract C is A, B {}
// ----
// TypeError: (126-147): Functions of the same name f and parameter types defined in two or more base contracts must be overridden in the derived contract.
