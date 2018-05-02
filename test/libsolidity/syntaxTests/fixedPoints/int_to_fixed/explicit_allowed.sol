contract C {
    uint128 a = 4;
    int128 b = -4;

    // Same sign
    ufixed256x18 f1 = ufixed256x18(a);
    fixed256x18 f2 = fixed256x18(b);
    ufixed192x18 f3 = ufixed192x18(a);
    fixed192x18 f4 = fixed192x18(b);
    ufixed256x30 f5 = ufixed256x30(a);
    fixed256x30 f6 = fixed256x30(b);

    // Opposite sign
    ufixed256x18 f7 = ufixed256x18(b);
    fixed256x18 f8 = fixed256x18(a);
    ufixed192x18 f9 = ufixed192x18(b);
    fixed192x18 f10 = fixed192x18(a);
}
