contract C {
    uint16 x = 0x1234;
    uint16 a = 0xffff;
    uint16 b;

    function f() public returns (uint256, uint256, uint256, uint256) {
        a++;
        uint256 c = b;
        delete b;
        a -= 2;
        return (x, c, b, a);
    }
}

// ----
// f() -> 0x1234, 0x0, 0x0, 0xfffe
