library L {
    struct R { uint[10][10] y; }
    struct S { uint a; uint b; uint[20][20][20] c; R d; }
}
contract C {
    function f(uint size) public {
        L.S[][] memory x = new L.S[][](10);
        var y = new uint[](20);
        var z = new bytes(size);
        x;y;z;
    }
}
// ----
// Warning: (205-210): Use of the "var" keyword is deprecated.
// Warning: (237-242): Use of the "var" keyword is deprecated.
// Warning: (122-282): Function state mutability can be restricted to pure
