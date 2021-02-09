contract C {
    function f() pure public {
        assembly {
            jumpi(2, 1)
        }
    }
}
// ----
// DeclarationError 4619: (75-80): Function "jumpi" not found.
