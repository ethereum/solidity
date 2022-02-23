contract C {
    modifier m() { _; }
}
contract D {
    function f() C.m public {
    }
}
// ----
// TypeError 9428: (69-72): Can only use modifiers defined in the current contract or in base contracts.
