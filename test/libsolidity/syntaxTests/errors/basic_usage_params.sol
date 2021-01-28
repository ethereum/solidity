error E(uint,string);
function f() pure {
    revert(E(2, "abc"));
}
function g(string storage s) pure {
    bool x;
    require(x, E(7, s));
}
// ----
