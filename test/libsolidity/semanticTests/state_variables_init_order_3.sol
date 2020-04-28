contract A {
    uint public a = 42;
    uint public b;
    uint public c;
    constructor(uint x) public {
        b = a;
        a = x;
    }
    function f(uint x) public returns (uint256) { c = x * 3;  return 23; }
}
contract B is A {
    uint public d = f(a);
    uint public e = b;
    uint public b_a;
    uint public b_b;
    uint public b_c;
    constructor() public A(17) { b_a = a; b_b = b; b_c = c; }
}
// ====
// compileViaYul: true
// ----
// a() -> 17
// b() -> 42
// c() -> 51
// b_a() -> 17
// b_b() -> 42
// b_c() -> 51
// d() -> 23
// e() -> 42
