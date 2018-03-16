// This caused a segfault in an earlier version
contract C {
    function C() public {}
}
contract D is C {
    function D() C(5) public {}
}
// ----
// TypeError: Wrong argument count for modifier invocation: 1 arguments given but expected 0.
