pragma experimental "v0.5.0";
contract test {
    function f() pure public {
        for (uint x = 0; x < 10; x ++){
            x = 2;
        }
        x = 4;
    }
}
// ----
// DeclarationError: Undeclared identifier.
