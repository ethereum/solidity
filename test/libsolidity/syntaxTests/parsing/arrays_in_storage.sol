contract c {
    uint[10] a;
    uint[] a2;
    struct x { uint[2**20] b; y[0] c; }
    struct y { uint d; mapping(uint=>x)[] e; }
}
