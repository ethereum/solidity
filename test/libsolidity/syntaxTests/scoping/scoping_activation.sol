contract test {
    function f() pure public {
        x = 3;
        uint x;
    }
}
// ----
// DeclarationError 7576: (55-56): Undeclared identifier. "x" is not (or not yet) visible at this point.
