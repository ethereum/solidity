pragma experimental ABIEncoderV2;
contract C {
    struct S {
        uint256 a;
        uint256 b;
    }

    function f(S calldata) external pure returns(uint256) {
        return msg.data.length;
    }
}

// ----
// f((uint256,uint256)): 1, 2 -> 0x44
// f((uint256,uint256)): 63, 0 -> 68
// f((uint256,uint256)): 33, 0 -> 68
// f((uint256,uint256)): 32, 0 -> 68
// f((uint256,uint256)): 31, 0 -> 68
// f((uint256,uint256)): 0 -> FAILURE
// f((uint256,uint256)) -> FAILURE
