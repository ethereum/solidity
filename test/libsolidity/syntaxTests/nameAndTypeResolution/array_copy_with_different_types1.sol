contract c {
    bytes a;
    uint[] b;
    function f() public { b = a; }
}
// ----
// TypeError: (70-71): Type bytes storage ref is not implicitly convertible to expected type uint256[] storage ref.
