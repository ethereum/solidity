contract C {
    function f() pure public {
        assembly {
            jump(2)
        }
    }
}
// ----
// DeclarationError: (75-79): Function not found.
