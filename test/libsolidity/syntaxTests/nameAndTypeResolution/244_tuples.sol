contract C {
    function f() public {
        uint a = (1);
        (uint b,) = (uint8(1),);
        (uint c, uint d) = (uint32(1), 2 + a);
        (uint e,) = (uint64(1), 2, b);
        a;b;c;d;e;
    }
}
// ----
// Warning: (69-92): Different number of components on the left hand side (2) than on the right hand side (1).
// Warning: (149-178): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (17-204): Function state mutability can be restricted to pure
