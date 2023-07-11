library L {
    function externalFunction(uint) external pure {}
    function f() public pure {
        uint x;
        externalFunction(x);
    }
}
// ----
// DeclarationError 7576: (120-136): Undeclared identifier. "externalFunction" is not (or not yet) visible at this point.
