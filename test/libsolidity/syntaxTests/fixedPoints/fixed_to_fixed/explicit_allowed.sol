contract C {
    ufixed a = 4;
    fixed b = -4;

    ufixed128x30 f1 = ufixed128x30(a);
    ufixed128x30 f2 = ufixed128x30(b);
    ufixed256x18 f3 = ufixed256x18(a);
    ufixed256x18 f4 = ufixed256x18(b);

    fixed128x30 f5 = fixed128x30(a);
    fixed128x30 f6 = fixed128x30(b);
    fixed256x18 f7 = fixed256x18(a);
    fixed256x18 f8 = fixed256x18(b);
}
