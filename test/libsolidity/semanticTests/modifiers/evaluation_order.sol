contract A { constructor(uint) {} }
contract B { constructor(uint) {} }
contract C { constructor(uint) {} }

contract D is A, B, C {
    uint[] x;
    constructor() m2(f(1)) B(f(2)) m1(f(3)) C(f(4)) m3(f(5)) A(f(6)) {
        f(7);
    }

    function query() public view returns (uint[] memory) { return x; }

    modifier m1(uint) { _; }
    modifier m2(uint) { _; }
    modifier m3(uint) { _; }

    function f(uint y) internal returns (uint) { x.push(y); return 0; }
}
// ----
// query() -> 0x20, 7, 4, 2, 6, 1, 3, 5, 7
