contract C {
    function f() pure public {
        assembly {
            jumpdest()
        }
    }
}
// ----
// DeclarationError 4619: (75-83): Function "jumpdest" not found.
