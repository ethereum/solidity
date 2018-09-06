contract c {
    uint[] a;
    uint[80] b;
    function f() public { b = a; }
}
// ----
// TypeError: (73-74): Type uint256[] storage ref is not implicitly convertible to expected type uint256[80] storage ref.
