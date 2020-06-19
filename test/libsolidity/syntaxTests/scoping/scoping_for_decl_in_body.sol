contract test {
    function f() pure public {
        for (;; y++){
            uint y = 3;
        }
    }
}
// ----
// DeclarationError 7576: (63-64): Undeclared identifier.
