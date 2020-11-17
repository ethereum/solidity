library L {
    modifier m() { _; }
}
contract C {
    function f() L.m public {
    }
}
// ----
// TypeError 9428: (68-71): Can only use modifiers defined in the current contract or in base contracts.
