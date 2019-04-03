{
    // This is an abi decode function with the SSA transform applied once.
    // This test is supposed to verify that the SSA transform is correctly reversed by the full suite.
    function abi_decode_t_bytes_calldata_ptr(offset_12, end_13) -> arrayPos_14, length_15
    {
        if iszero(slt(add(offset_12, 0x1f), end_13))
        {
            revert(0, 0)
        }
        let length_15_1 := calldataload(offset_12)
        length_15 := length_15_1
        if gt(length_15_1, 0xffffffffffffffff)
        {
            revert(0, 0)
        }
        let arrayPos_14_2 := add(offset_12, 0x20)
        arrayPos_14 := arrayPos_14_2
        if gt(add(arrayPos_14_2, mul(length_15_1, 0x1)), end_13)
        {
            revert(0, 0)
        }
    }

    // prevent inlining
    let a,b := abi_decode_t_bytes_calldata_ptr(mload(0),mload(1))
    a,b := abi_decode_t_bytes_calldata_ptr(a,b)
    a,b := abi_decode_t_bytes_calldata_ptr(a,b)
    a,b := abi_decode_t_bytes_calldata_ptr(a,b)
    a,b := abi_decode_t_bytes_calldata_ptr(a,b)
    a,b := abi_decode_t_bytes_calldata_ptr(a,b)
    a,b := abi_decode_t_bytes_calldata_ptr(a,b)
    mstore(a,b)
}
// ====
// step: fullSuite
// ----
// {
//     let a, b := abi_decode_t_bytes_calldata_ptr(mload(0), mload(1))
//     a, b := abi_decode_t_bytes_calldata_ptr(a, b)
//     a, b := abi_decode_t_bytes_calldata_ptr(a, b)
//     a, b := abi_decode_t_bytes_calldata_ptr(a, b)
//     a, b := abi_decode_t_bytes_calldata_ptr(a, b)
//     a, b := abi_decode_t_bytes_calldata_ptr(a, b)
//     a, b := abi_decode_t_bytes_calldata_ptr(a, b)
//     mstore(a, b)
//     function abi_decode_t_bytes_calldata_ptr(offset, end) -> arrayPos, length
//     {
//         if iszero(slt(add(offset, 0x1f), end))
//         {
//             revert(0, 0)
//         }
//         length := calldataload(offset)
//         if gt(length, 0xffffffffffffffff)
//         {
//             revert(0, 0)
//         }
//         arrayPos := add(offset, 0x20)
//         if gt(add(add(offset, length), 0x20), end)
//         {
//             revert(0, 0)
//         }
//     }
// }
