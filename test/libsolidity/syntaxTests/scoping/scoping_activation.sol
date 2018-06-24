contract test {
    function f() pure public {
        x = 3;
        uint x;
    }
}
// ----
// DeclarationError: (55-56): Undeclared identifier. "x" is not (or not yet) visible at this point.
