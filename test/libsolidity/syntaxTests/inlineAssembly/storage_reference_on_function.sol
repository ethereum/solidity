contract C {
    function f() pure public {
        assembly {
            let x := f_slot
        }
    }
}
// ----
// DeclarationError: (84-90): Identifier not found.
