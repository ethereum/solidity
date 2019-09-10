contract C {
    function f() public pure {
        assembly {
            let x := _slot
        }
    }
}
// ----
// DeclarationError: (84-89): In variable names _slot and _offset can only be used as a suffix.
