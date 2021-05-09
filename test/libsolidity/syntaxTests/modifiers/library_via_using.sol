library L {
    modifier m() { _; }
}
contract C {
    using L for *;
    function f() L.m public {
    }
}
// ----
// TypeError 9428: (87-90): Can only use modifiers defined in the current contract or in base contracts.
