pragma experimental "v0.5.0";
contract test {
    function f() pure public {
        uint a = a;
    }
}
// ----
// DeclarationError: (94-95): Undeclared identifier. Did you mean "a"?
