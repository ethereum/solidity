contract A {
    function f(uint[] calldata) external pure {}
    function f(uint[] memory) internal pure {}
}
// ----
// DeclarationError: (17-61): Function with same name and arguments defined twice.
