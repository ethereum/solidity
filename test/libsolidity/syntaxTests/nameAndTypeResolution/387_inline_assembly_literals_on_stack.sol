contract C {
    function f() pure public {
        assembly {
            1
        }
    }
}
// ----
// Warning: (75-76): Top-level expressions are not supposed to return values (this expression returns 1 value). Use ``pop()`` or assign them.
// DeclarationError: (61-86): Unbalanced stack at the end of a block: 1 surplus item(s).
