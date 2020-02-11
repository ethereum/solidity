pragma experimental ABIEncoderV2;
contract C {
    struct S1 {
        uint256 a;
        uint256 b;
    }
    struct S2 {
        uint256 a;
        uint256 b;
        S1 s;
        uint256 c;
    }

    function f(S2 calldata s) external pure returns(uint256 a, uint256 b, uint256 sa, uint256 sb, uint256 c) {
        S2 memory m = s;
        return (m.a, m.b, m.s.a, m.s.b, m.c);
    }
}

// ----
// f((uint256,uint256,(uint256,uint256),uint256)): encodeArgs(1), 2), 3), 4), 5)) -> 1, 2, 3, 4, 5
// f((uint256,uint256,(uint256,uint256),uint256)):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5]" -> "1, 2, 3, 4, 5"
