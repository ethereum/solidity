pragma experimental "v0.5.0";
contract test {
    function f() pure public {
        uint a = a;
    }
}
// ----
// DeclarationError: Undeclared identifier. Did you mean "a"?
