contract C {
}
function f() pure {
    super;
}
// ----
// DeclarationError 7576: (39-44): Undeclared identifier. "super" is not (or not yet) visible at this point.
