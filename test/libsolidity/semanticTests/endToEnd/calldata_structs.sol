pragma experimental ABIEncoderV2;
contract C {
    struct S1 {
        uint256 a;
        uint256 b;
    }
    struct S2 {
        uint256 a;
    }

    function f(S1 calldata s1, S2 calldata s2, S1 calldata s3)
    external pure returns(uint256 a, uint256 b, uint256 c, uint256 d, uint256 e) {
        a = s1.a;
        b = s1.b;
        c = s2.a;
        d = s3.a;
        e = s3.b;
    }
}

// ----
// f((uint256,uint256),(uint256),(uint256,uint256)): encodeArgs(1), 2), 3), 4), 5)) -> 1, 2, 3, 4, 5
// f((uint256,uint256),(uint256),(uint256,uint256)):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5]" -> "1, 2, 3, 4, 5"
