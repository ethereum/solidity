abstract contract C {
    function f() internal virtual returns(uint[] storage);
    function g() internal virtual returns(uint[] storage s);
}
// ----
