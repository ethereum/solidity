pragma experimental "v0.5.0";
contract C {
    function f() pure public {
        assembly {
            1
        }
    }
}
// ----
// SyntaxError: (105-106): Top-level expressions are not supposed to return values (this expression returns 1 value). Use ``pop()`` or assign them.
// DeclarationError: (91-116): Unbalanced stack at the end of a block: 1 surplus item(s).
