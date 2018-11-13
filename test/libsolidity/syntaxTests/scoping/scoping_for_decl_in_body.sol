contract test {
    function f() pure public {
        for (;; y++){
            uint y = 3;
        }
    }
}
// ----
// DeclarationError: (63-64): Undeclared identifier.
