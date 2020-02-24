contract Test {
    struct S {
        uint8 x;
        uint16 y;
        uint256 z;
    }

    function test() public returns (uint256 x, uint256 y, uint256 z) {
        S memory data = combine(1, 2, 3);
        x = extract(data, 0);
        y = extract(data, 1);
        z = extract(data, 2);
    }

    function extract(S memory s, uint256 which) internal returns (uint256 x) {
        if (which == 0) return s.x;
        else if (which == 1) return s.y;
        else return s.z;
    }

    function combine(uint8 x, uint16 y, uint256 z)
        internal
        returns (S memory s)
    {
        s.x = x;
        s.y = y;
        s.z = z;
    }
}

// ----
// test() -> 1, 2, 3
