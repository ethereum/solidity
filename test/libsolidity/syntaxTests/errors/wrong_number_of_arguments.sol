error E();
function f() pure {
    require();
    require(true, E(), 8);
    revert(E(), 3);
}
// ----
// TypeError 7445: (35-44): Function "require" needs 1 or 2 arguments, but provided 0.
// TypeError 7445: (50-71): Function "require" needs 1 or 2 arguments, but provided 3.
// TypeError 7445: (77-91): Function "revert" needs 0 or 1 arguments, but provided 2.
