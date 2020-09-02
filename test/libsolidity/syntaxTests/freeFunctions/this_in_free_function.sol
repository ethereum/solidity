contract C {}
function f() {
    this;
}
// ----
// DeclarationError 7576: (33-37): Undeclared identifier. "this" is not (or not yet) visible at this point.
