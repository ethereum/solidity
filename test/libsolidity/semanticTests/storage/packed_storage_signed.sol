contract C {
    int8 a;
    uint8 b;
    int8 c;
    uint8 d;

    function test()
        public
        returns (uint256 x1, uint256 x2, uint256 x3, uint256 x4)
    {
        a = -2;
        unchecked {
            b = (0 - uint8(a)) * 2;
            c = a * int8(120) * int8(121);
        }
        x1 = uint256(a);
        x2 = b;
        x3 = uint256(c);
        x4 = d;
    }
}

// ====
// compileViaYul: also
// ----
// test() -> -2, 4, -112, 0
