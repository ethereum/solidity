contract C {
    function f() public {
        uint a = (1);
        var (b,) = (uint8(1),);
        var (c,d) = (uint32(1), 2 + a);
        var (e,) = (uint64(1), 2, b);
        a;b;c;d;e;
    }
}
// ----
// Warning: (74-75): Use of the "var" keyword is deprecated.
// Warning: (106-107): Use of the "var" keyword is deprecated.
// Warning: (108-109): Use of the "var" keyword is deprecated.
// Warning: (146-147): Use of the "var" keyword is deprecated.
// Warning: (69-91): Different number of components on the left hand side (2) than on the right hand side (1).
// Warning: (141-169): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (17-195): Function state mutability can be restricted to pure
