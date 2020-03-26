contract C {
    struct X {
        uint256 x1;
        uint256 x2;
    }
    struct S {
        uint256 s1;
        uint256[3] s2;
        X s3;
    }
    S s;

    constructor() public {
        uint256[3] memory s2;
        s2[1] = 9;
        s = S(1, s2, X(4, 5));
    }

    function get()
        public
        returns (uint256 s1, uint256[3] memory s2, uint256 x1, uint256 x2)
    {
        s1 = s.s1;
        s2 = s.s2;
        x1 = s.s3.x1;
        x2 = s.s3.x2;
    }
}
// ----
// get() -> 0x01, 0x00, 0x09, 0x00, 0x04, 0x05
