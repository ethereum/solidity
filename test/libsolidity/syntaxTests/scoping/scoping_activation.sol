pragma experimental "v0.5.0";
contract test {
    function f() pure public {
        x = 3;
        uint x;
    }
}
// ----
// DeclarationError: (85-86): Undeclared identifier. Did you mean "x"?
