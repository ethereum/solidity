struct S {
    uint x;
    address a;
    bool b;
}

contract A {
    S public s1;
    uint transient t1;
}

contract C is A layout at 42 {
    uint y;
    uint8 z;
    uint transient t2;
    S public s2;

    function initS1() public returns (uint, address, bool) {
        s1.x = 7;
        s1.a = address(0xABC);
        s1.b = true;

        return (s1.x, s1.a, s1.b);
    }
    function initS2() public returns (uint, address, bool) {
        s2.x = 8;
        s2.a = address(0xDEF);
        s2.b = false;

        return (s2.x, s2.a, s2.b);
    }
}
// ====
// EVMVersion: >=cancun
// targetContract: C
// ----
// initS1() -> 7, 0x0abc, 1
// initS2() -> 8, 0x0def, 0
// s1() -> 7, 0x0abc, 1
// s2() -> 8, 0x0def, 0
