contract C {
    function f() pure public {
        assembly {
            jump(2)
        }
    }
}
// ----
// DeclarationError 4619: (75-79): Function not found.
