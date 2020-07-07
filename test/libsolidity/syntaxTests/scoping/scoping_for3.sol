contract test {
    function f() pure public {
        for (uint x = 0; x < 10; x ++){
            x = 2;
        }
        x = 4;
    }
}
// ----
// DeclarationError 7576: (124-125): Undeclared identifier.
