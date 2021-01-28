error E();
function f() pure {
    revert(E());
}
function g() pure {
    bool x;
    require(x, E());
}
// ----
