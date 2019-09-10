contract C {
    function f() public pure {
        assembly {
            let x := _offset
        }
    }
}
// ----
// DeclarationError: (84-91): In variable names _slot and _offset can only be used as a suffix.
