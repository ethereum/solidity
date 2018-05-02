contract C {
    uint128 a = 4;
    int128 b = -4;

    // Same sign
    ufixed256x18 f1 = a;
    fixed256x18 f2 = b;
    ufixed192x18 f3 = a;
    fixed192x18 f4 = b;

    // Opposite sign
    //ufixed256x18 f5 = b;
    fixed256x18 f6 = a;
    //ufixed192x18 f7 = b;
    fixed192x18 f8 = a;
}
