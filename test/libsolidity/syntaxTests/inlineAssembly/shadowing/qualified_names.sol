contract D {
    uint constant a;
}
contract C {
    function f() public pure {
        assembly {
            let D.a := 1
            let D.b := 1 // shadowing the prefix only is also an error
        }
    }
}
// ----
// DeclarationError 3927: (115-118): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (140-143): User-defined identifiers in inline assembly cannot contain '.'.
