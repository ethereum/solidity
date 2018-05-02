contract C {
    ufixed a = 4;
    fixed b = -4;

    ufixed256x18 f1 = a;
    ufixed256x19 f2 = a;

    fixed256x18 f3 = b;
    fixed256x18 f4 = b;

    fixed256x18 f5 = a;
    fixed256x18 f6 = a;
}
