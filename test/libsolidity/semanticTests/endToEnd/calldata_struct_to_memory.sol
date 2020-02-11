pragma experimental ABIEncoderV2;
contract C {
    struct S {
        uint256 a;
        uint256 b;
    }

    function f(S calldata s) external pure returns(uint256, uint256) {
        S memory m = s;
        return (m.a, m.b);
    }
}

// ----
// f((uint256,uint256)): encodeArgs(42), 23)) -> 42, 23
// f((uint256,uint256)):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2a,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17]" -> "42, 23"
