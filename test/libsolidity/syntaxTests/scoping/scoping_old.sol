contract test {
    function f() pure public {
        x = 4;
        uint256 x = 2;
    }
}
// ----
// DeclarationError: (55-56): Undeclared identifier. "x" is not (or not yet) visible at this point.
