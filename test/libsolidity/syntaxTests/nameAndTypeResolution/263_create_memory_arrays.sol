library L {
    struct R { uint[10][10] y; }
    struct S { uint a; uint b; uint[20][20][20] c; R d; }
}
contract C {
    function f(uint size) public {
        L.S[][] memory x = new L.S[][](10);
        uint[] memory y = new uint[](20);
        bytes memory z = new bytes(size);
        x;y;z;
    }
}
// ----
// Warning: (122-301): Function state mutability can be restricted to pure
