contract C {
    function f() pure public {
        assembly {
            mload(0)
        }
    }
}
// ----
// Warning: (75-83): Top-level expressions are not supposed to return values (this expression returns 1 value). Use ``pop()`` or assign them.
// DeclarationError: (61-93): Unbalanced stack at the end of a block: 1 surplus item(s).
