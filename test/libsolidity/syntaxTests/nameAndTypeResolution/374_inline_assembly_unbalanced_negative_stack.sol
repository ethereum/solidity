contract test {
    function f() public {
        assembly {
            pop
        }
    }
}
// ----
// SyntaxError: (73-76): The use of non-functional instructions is disallowed. Please use functional notation instead.
// DeclarationError: (59-86): Unbalanced stack at the end of a block: 1 missing item(s).
