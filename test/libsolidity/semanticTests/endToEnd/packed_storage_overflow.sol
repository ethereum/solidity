contract C {
    uint16 x = 0x1234;
    uint16 a = 0xffff;
    uint16 b;

    function f() public returns(uint, uint, uint, uint) {
        a++;
        uint c = b;
        delete b;
        a -= 2;
        return (x, c, b, a);
    }
}

// ----
// f() -> 0x1234, 0, 0, 0xfffe
// f():"" -> "4660, 0, 0, 65534"
