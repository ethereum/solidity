contract test {
    function f() public {
        assembly {
            pop
        }
    }
}
// ----
// Warning: (73-76): The use of non-functional instructions is deprecated. Please use functional notation instead.
// DeclarationError: (59-86): Unbalanced stack at the end of a block: 1 missing item(s).
