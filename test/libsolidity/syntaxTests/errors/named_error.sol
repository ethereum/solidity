error E(uint a);
function f() pure {
    revert(E({a: 2}));
}
// ----
