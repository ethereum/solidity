contract C {
    function f() public pure {
        assembly {
            let x.offset := 1
            let x.slot := 1
        }
    }
}
// ----
// DeclarationError 3927: (79-87='x.offset'): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (109-115='x.slot'): User-defined identifiers in inline assembly cannot contain '.'.
