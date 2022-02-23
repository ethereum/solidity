contract C {
    struct S { uint y; }
    function f() public pure {
        S memory x = x.y;
    }
}
// ----
// DeclarationError 7576: (90-91): Undeclared identifier. "x" is not (or not yet) visible at this point.
