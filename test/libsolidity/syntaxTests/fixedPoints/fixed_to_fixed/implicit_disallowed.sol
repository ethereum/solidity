contract C {
    ufixed a = 4;
    fixed b = -4;

    ufixed128x30 f1 = a;
    ufixed128x30 f2 = b;

    fixed128x30 f3 = a;
    fixed128x30 f4 = b;

    fixed f5 = a;
}
// ----
// TypeError: (72-73): Type ufixed128x18 is not implicitly convertible to expected type ufixed128x30.
// TypeError: (97-98): Type fixed128x18 is not implicitly convertible to expected type ufixed128x30.
// TypeError: (122-123): Type ufixed128x18 is not implicitly convertible to expected type fixed128x30.
// TypeError: (146-147): Type fixed128x18 is not implicitly convertible to expected type fixed128x30.
// TypeError: (165-166): Type ufixed128x18 is not implicitly convertible to expected type fixed128x18.
