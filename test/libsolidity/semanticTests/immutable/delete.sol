contract C {
    uint8 immutable public a;
    uint8 immutable public b = 0x42;
    uint public c;

    constructor() {
        delete a;
        delete b;
        c = b * 2 + a;
    }
}
// ----
// a() -> 0
// b() -> 0
// c() -> 0
