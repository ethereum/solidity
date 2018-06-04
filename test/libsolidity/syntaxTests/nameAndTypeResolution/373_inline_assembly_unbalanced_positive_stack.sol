contract test {
    function f() public {
        assembly {
            1
        }
    }
}
// ----
// Warning: (73-74): Top-level expressions are not supposed to return values (this expression returns 1 value). Use ``pop()`` or assign them.
// DeclarationError: (59-84): Unbalanced stack at the end of a block: 1 surplus item(s).
