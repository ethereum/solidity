contract C {
    modifier m1(uint[] storage a) { _; }
    modifier m2(uint[] storage a) { _; }
    uint[] s;
    function f() m1(b) m2(b = s) internal view returns (uint[] storage b) {}
}
// ----
// TypeError: (129-130): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
