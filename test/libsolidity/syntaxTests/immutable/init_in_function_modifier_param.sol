contract D {
    uint immutable t;
    modifier m(uint) { _; }
    function f() public m(t = 2) {}
}
// ----
// TypeError 1581: (89-90): Cannot write to immutable here: Immutable variables can only be initialized inline or assigned directly in the constructor.
