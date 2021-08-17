contract C {
    fixed64x4 public a = -2.123;
    fixed64x4 public immutable b = -2.456;
    mapping(uint => fixed64x4) public m;
    constructor() { m[3] = 1.123; }
}
// ====
// compileViaYul: also
// ----
// a() -> -2.1230
// b() -> -2.4560
// m(uint256): 2 -> 0.0000
// m(uint256): 3 -> 1.1230
