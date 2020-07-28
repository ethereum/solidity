contract C {
    uint8 public a;
    uint16 public b;
    uint128 public c;
    uint public d;
    constructor() {
        a = 3;
        b = 4;
        c = 5;
        d = 6;
    }
}
// ====
// compileViaYul: also
// ----
// a() -> 3
// b() -> 4
// c() -> 5
// d() -> 6
