contract C {
    modifier m() { _; }
}
contract D is C {
    function f() C.m public {
    }
}
// ----
