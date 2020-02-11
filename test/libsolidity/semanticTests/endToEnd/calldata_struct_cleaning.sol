pragma experimental ABIEncoderV2;
contract C {
    struct S {
        uint8 a;
        bytes1 b;
    }

    function f(S calldata s) external pure returns(uint256 a, bytes32 b) {
        uint8 tmp1 = s.a;
        bytes1 tmp2 = s.b;
        assembly {
            a := tmp1
            b := tmp2
        }

    }
}

// ----
// f((uint8,bytes1)): 0x12, 0x34, 31, 0 -> FAILURE
// f((uint8,bytes1)): 0x1234, 0x56, 0x78, 30, 0 -> FAILURE
// f((uint8,bytes1)): -1, -1 -> FAILURE
