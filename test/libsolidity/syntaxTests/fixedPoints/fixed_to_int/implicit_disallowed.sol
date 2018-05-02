contract C {
    ufixed a = 4;
    fixed b = -4;

    uint i1 = a;
    uint i2 = b;
    int i3 = a;
    int i4 = b;
}
// ----
// TypeError: (64-65): Type ufixed128x18 is not implicitly convertible to expected type uint256.
// TypeError: (81-82): Type fixed128x18 is not implicitly convertible to expected type uint256.
// TypeError: (97-98): Type ufixed128x18 is not implicitly convertible to expected type int256.
// TypeError: (113-114): Type fixed128x18 is not implicitly convertible to expected type int256.
