contract A {
    error E();
}
contract C {
    function f() public pure {
        revert A.E();
    }
}
// ----
