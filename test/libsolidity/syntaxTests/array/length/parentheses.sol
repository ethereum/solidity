contract C {
    uint constant L1 = (2);
    uint constant L2 = ((2));
    uint constant L3 = ((((2))));
    uint constant L4 = (2 + 1);
    uint constant L5 = ((2 + 1));
    uint constant L6 = (((2) + ((1))));
    uint constant L7 = (2 + 1) / 1;
    uint constant L8 = (2 + ((1))) / (1);
    uint[L1] a1;
    uint[L2] a2;
    uint[L3] a3;
    uint[L4] a4;
    uint[L5] a5;
    uint[L6] a6;
    uint[L7] a7;
    uint[L8] a8;
    uint[(2)] a9;
    uint[(2 + 1)] a10;
    uint[(2 + 1) + 1] a11;
    uint[((2) + 1) + 1] a12;
    uint[(2 + 1) + ((1))] a13;
    uint[(((2) + 1)) + (((1)))] a14;
    uint[((((3) + 1)) + (((1))))%2] a15;
}
