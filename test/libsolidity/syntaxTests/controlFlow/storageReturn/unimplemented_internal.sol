contract C {
    function f() internal returns(uint[] storage);
    function g() internal returns(uint[] storage s);
}
// ----
// TypeError: (0-118): Contract "C" should be marked as abstract.
