contract A {
    function f(uint[] calldata) external pure {}
}
contract B {
    function f(uint[] memory) internal pure {}
}
contract C is A, B {}
// ----
// TypeError: (81-123): Overriding function visibility differs.
