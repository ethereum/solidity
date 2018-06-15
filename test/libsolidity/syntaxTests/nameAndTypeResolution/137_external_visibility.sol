contract c {
    function f() external {}
    function g() public { f(); }
}
// ----
// DeclarationError: (68-69): Undeclared identifier. "f" is not (or not yet) visible at this point.
