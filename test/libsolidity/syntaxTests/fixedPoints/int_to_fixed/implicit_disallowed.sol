contract C {
    uint128 a = 4;
    int128 b = -4;

    // Same sign
    ufixed160x18 f1 = a;
    fixed160x18 f2 = b;
    ufixed256x30 f3 = a;
    fixed256x30 f4 = b;

    // Opposite sign
    ufixed256x18 f5 = b;
    ufixed192x18 f7 = b;
}
// ----
// TypeError: (91-92): Type uint128 is not implicitly convertible to expected type ufixed160x18.
// TypeError: (115-116): Type int128 is not implicitly convertible to expected type fixed160x18.
// TypeError: (211-212): Type int128 is not implicitly convertible to expected type ufixed256x18.
// TypeError: (236-237): Type int128 is not implicitly convertible to expected type ufixed192x18.
