pragma experimental "v0.5.0";
contract C {
    function f() pure public {
        assembly {
            mload(0)
        }
    }
}
// ----
// SyntaxError: (105-113): Top-level expressions are not supposed to return values (this expression returns 1 value). Use ``pop()`` or assign them.
// DeclarationError: (91-123): Unbalanced stack at the end of a block: 1 surplus item(s).
