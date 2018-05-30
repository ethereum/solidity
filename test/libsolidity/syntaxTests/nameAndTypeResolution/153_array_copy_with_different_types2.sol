contract c {
    uint32[] a;
    uint8[] b;
    function f() public { b = a; }
}
// ----
// TypeError: (74-75): Type uint32[] storage ref is not implicitly convertible to expected type uint8[] storage ref.
