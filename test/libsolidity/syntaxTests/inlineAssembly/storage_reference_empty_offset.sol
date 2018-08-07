contract C {
    function f() public pure {
        assembly {
            _offset
        }
    }
}
// ----
// DeclarationError: (75-82): In variable names _slot and _offset can only be used as a suffix.
