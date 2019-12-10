contract C {
    function f() public pure {
        assembly {
            let x_offset := 1
            let x_slot := 1
            let _offset := 1
            let _slot := 1
        }
    }
}
// ----
// DeclarationError: (79-87): In variable declarations _slot and _offset can not be used as a suffix.
// DeclarationError: (109-115): In variable declarations _slot and _offset can not be used as a suffix.
// DeclarationError: (137-144): In variable declarations _slot and _offset can not be used as a suffix.
// DeclarationError: (166-171): In variable declarations _slot and _offset can not be used as a suffix.
