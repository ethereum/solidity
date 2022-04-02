contract test {
    struct S {
        function (uint x, uint y) internal returns (uint) f;
        function (uint, uint) external returns (uint) g;
        uint d;
    }
}
// ----
// Warning 6162: (49-55='uint x'): Naming function type parameters is deprecated.
// Warning 6162: (57-63='uint y'): Naming function type parameters is deprecated.
