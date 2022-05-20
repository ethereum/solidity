contract A {
    constructor(uint) {}
}
contract B {
    constructor(uint) {}
}
contract C {
    constructor(uint) {}
}
contract D {
    constructor(uint) {}
}
contract X is D, C, B, A {
    uint[] x;
    function f(uint _x) internal returns (uint) {
        x.push(_x);
    }
    function g() public view returns (uint[] memory) { return x; }
    constructor() A(f(1)) C(f(2)) B(f(3)) D(f(4)) {}
}
// ----
// g() -> 0x20, 4, 1, 3, 2, 4
