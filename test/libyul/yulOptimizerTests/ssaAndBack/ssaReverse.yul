{
     function abi_decode_t_bytes_calldata_ptr(offset_12, end_13) -> arrayPos_14, length_15
     {
         if iszero(slt(add(offset_12, 0x1f), end_13))
         {
             revert(0, 0)
         }
         length_15 := calldataload(offset_12)
         if gt(length_15, 0xffffffffffffffff)
         {
             revert(0, 0)
         }
         arrayPos_14 := add(offset_12, 0x20)
         if gt(add(add(offset_12, length_15), 0x20), end_13)
         {
             revert(0, 0)
         }
     }
     // prevent removal of the function
     let a,b := abi_decode_t_bytes_calldata_ptr(mload(0),mload(1))
     mstore(a,b)
}
// ====
// step: ssaAndBack
// ----
// {
//     function abi_decode_t_bytes_calldata_ptr(offset_12, end_13) -> arrayPos_14, length_15
//     {
//         if iszero(slt(add(offset_12, 0x1f), end_13))
//         {
//             revert(arrayPos_14, arrayPos_14)
//         }
//         length_15 := calldataload(offset_12)
//         if gt(length_15, 0xffffffffffffffff)
//         {
//             revert(arrayPos_14, arrayPos_14)
//         }
//         arrayPos_14 := add(offset_12, 0x20)
//         if gt(add(add(offset_12, length_15), 0x20), end_13)
//         {
//             revert(0, 0)
//         }
//     }
//     let a, b := abi_decode_t_bytes_calldata_ptr(mload(0), mload(1))
//     mstore(a, b)
// }
