struct S { uint x; }
contract C {
    function f() public {
        revert S(10);
    }
}
// ----
// TypeError 1885: (75-76='S'): Expression has to be an error.
