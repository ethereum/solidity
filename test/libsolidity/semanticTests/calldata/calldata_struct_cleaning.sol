pragma experimental ABIEncoderV2;


contract C {
    struct S {
        uint8 a;
        bytes1 b;
    }

    function f(S calldata s) external pure returns (uint256 a, bytes32 b) {
        uint8 tmp1 = s.a;
        bytes1 tmp2 = s.b;
        assembly {
            a := tmp1
            b := tmp2
        }
    }
}
// ====
// compileViaYul: also
// ----
// f((uint8,bytes1)): 0x12, hex"3400000000000000000000000000000000000000000000000000000000000000" -> 0x12, hex"3400000000000000000000000000000000000000000000000000000000000000" # double check that the valid case goes through #
// f((uint8,bytes1)): 0x1234, hex"5678000000000000000000000000000000000000000000000000000000000000" -> FAILURE
// f((uint8,bytes1)): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> FAILURE
