contract C layout at uint(0) + 123 {
    int[uint(42)] a;
    int[uint(3 * 3 - 1)] b;
    int[uint(-(-8))] c;
    int[uint(uint(10))] d;
    int[uint((8 - 10) * -1)] e;
    int[uint(1e2)] f;
    int[uint(8 / 2)] g;
    int[uint(2.0)] h;
    int[uint(3 / 2 * 4)] i;
    int[uint(2**256 - 1) / 2**255] j;
}
// ----
