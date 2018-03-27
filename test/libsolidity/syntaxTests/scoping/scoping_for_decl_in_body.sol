pragma experimental "v0.5.0";
contract test {
    function f() pure public {
        for (;; y++){
            uint y = 3;
        }
    }
}
// ----
// DeclarationError: Undeclared identifier.
