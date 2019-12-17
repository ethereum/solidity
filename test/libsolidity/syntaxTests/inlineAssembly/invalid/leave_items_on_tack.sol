contract C {
    function f() pure public {
        assembly {
            mload(0)
        }
    }
}
// ----
// TypeError: (75-83): Top-level expressions are not supposed to return values (this expression returns 1 value). Use ``pop()`` or assign them.
