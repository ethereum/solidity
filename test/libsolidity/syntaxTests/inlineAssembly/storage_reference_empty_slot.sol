contract C {
    function f() public pure {
        assembly {
            _slot
        }
    }
}
// ----
// DeclarationError: (75-80): In variable names _slot and _offset can only be used as a suffix.
