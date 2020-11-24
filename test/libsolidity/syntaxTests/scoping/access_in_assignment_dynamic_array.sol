contract C {
    function f() public pure {
        uint[] memory x = x[0];
    }
}
// ----
// DeclarationError 7576: (70-71): Undeclared identifier. "x" is not (or not yet) visible at this point.
