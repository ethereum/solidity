pragma experimental ABIEncoderV2;
contract C {
    struct S {
        uint256 a;
        uint256 b;
    }

    function f(uint256 a, S calldata s, uint256 b) external pure returns(uint256, uint256, uint256, uint256) {
        return (a, s.a, s.b, b);
    }
}

// ----
// f(uint256,(uint256,uint256),uint256): encodeArgs(1), 2), 3), 4)) -> 1, 2, 3, 4
// f(uint256,(uint256,uint256),uint256):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4]" -> "1, 2, 3, 4"
