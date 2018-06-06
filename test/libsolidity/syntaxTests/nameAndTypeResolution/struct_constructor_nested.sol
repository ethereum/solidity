contract C {
    struct X { uint x1; uint x2; }
    struct S { uint s1; uint[3] s2; X s3; }
    function f() public {
        uint[3] memory s2;
        S memory s = S(1, s2, X(4, 5));
    }
}
// ----
// Warning: (153-163): Unused local variable.
// Warning: (96-190): Function state mutability can be restricted to pure
