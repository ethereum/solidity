{
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
}
// ----
// ssaReverser
// {
//     function abi_decode_t_bytes_calldata_ptr(offset_12, end_13) -> arrayPos_14, length_15
//     {
//         if iszero(slt(add(offset_12, 0x1f), end_13))
//         {
//             revert(0, 0)
//         }
//         length_15 := calldataload(offset_12)
//         let length_15_1 := length_15
//         if gt(length_15_1, 0xffffffffffffffff)
//         {
//             revert(0, 0)
//         }
//         arrayPos_14 := add(offset_12, 0x20)
//         let arrayPos_14_2 := arrayPos_14
//         if gt(add(arrayPos_14_2, mul(length_15_1, 0x1)), end_13)
//         {
//             revert(0, 0)
//         }
//     }
// }
