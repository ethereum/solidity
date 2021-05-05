{
    let _1 := 0
    let value := calldataload(4)
    if iszero(eq(value, and(value, sub(shl(160, 1), 1)))) { revert(_1, _1) }
    let length := extcodesize(value)
    let _2 := 0xffffffffffffffff
    if gt(length, _2) { revert(0, 0) }
    let _3 := not(31)
    let memPtr := mload(64)
    let newFreePtr := add(memPtr, and(add(and(add(length, 31), _3), 63), _3))
    if or(gt(newFreePtr, _2), lt(newFreePtr, memPtr)) { revert(0, 0) }
    mstore(64, newFreePtr)
    mstore(memPtr, length)

    // We aim to optimize this out.
    extcodecopy(value, add(memPtr, 32), _1, length)
    sstore(_1, mload(memPtr))
}
// ====
// EVMVersion: >byzantium
// ----
// step: fullSuite
//
// {
//     {
//         let value := calldataload(4)
//         if iszero(eq(value, and(value, sub(shl(160, 1), 1)))) { revert(0, 0) }
//         let length := extcodesize(value)
//         let _1 := 0xffffffffffffffff
//         if gt(length, _1) { revert(0, 0) }
//         let memPtr := mload(64)
//         let _2 := not(31)
//         let newFreePtr := add(memPtr, and(add(and(add(length, 31), _2), 63), _2))
//         if or(gt(newFreePtr, _1), lt(newFreePtr, memPtr)) { revert(0, 0) }
//         sstore(0, length)
//     }
// }
