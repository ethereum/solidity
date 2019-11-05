abstract contract C {
    function f() internal returns(uint[] storage);
    function g() internal returns(uint[] storage s);
}
// ----
