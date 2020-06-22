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
// DeclarationError 3859: (115-118): The prefix of this declaration conflicts with a declaration outside the inline assembly block.
// DeclarationError 3859: (140-143): The prefix of this declaration conflicts with a declaration outside the inline assembly block.
