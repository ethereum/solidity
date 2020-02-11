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
// f((uint256,uint256)): 1), 2) -> 0x44
// f((uint256,uint256)):"1, 2" -> "68"
// callContractFunctionNoEncoding("f((uint256,uint256)): bytes(63,0) -> 
// f((uint256,uint256)):"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" -> ""
// callContractFunctionNoEncoding("f((uint256,uint256)): bytes(33,0) -> 
// f((uint256,uint256)):"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" -> ""
// callContractFunctionNoEncoding("f((uint256,uint256)): bytes(32,0) -> 
// f((uint256,uint256)):"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" -> ""
// callContractFunctionNoEncoding("f((uint256,uint256)): bytes(31,0) -> 
// f((uint256,uint256)):"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" -> ""
// callContractFunctionNoEncoding("f((uint256,uint256)): bytes() -> 
// f((uint256,uint256)):"" -> ""
