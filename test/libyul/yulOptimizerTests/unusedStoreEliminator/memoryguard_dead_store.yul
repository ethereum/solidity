// Regression test for: dead mstore instructions not eliminated when the store base
// is derived from memoryguard(N).
//
// The KnowledgeBase must recognize memoryguard(literal) as a constant so that
// knownUnrelated() can prove that stores at _1..._1+96 do not overlap with
// a read at a different constant address.
//
// This simulates the pattern after LoadResolver has already forwarded the mload(0x40)
// result so that memPos is a known constant (newFreePtr), enabling the
// unusedStoreEliminator to prove non-overlap and remove the dead stores.
{
    let _1 := memoryguard(0x80)
    let newFreePtr := add(_1, 128)
    mstore(64, newFreePtr)

    let _a1 := _1
    let _a2 := add(_1, 32)
    let _a3 := add(_1, 64)
    let _a4 := add(_1, 96)

    let value := calldataload(36)
    let v2 := calldataload(68)
    let v3 := calldataload(100)
    let v4 := calldataload(132)

    mstore(_a1, value)    // dead: _a1=0x80, newFreePtr=0x100, non-overlapping
    mstore(_a2, v2)       // dead: _a2=0xa0
    mstore(_a3, v3)       // dead: _a3=0xc0
    mstore(_a4, v4)       // dead: _a4=0xe0

    // Use newFreePtr (=0x100) as the write/return target,
    // known to be >=32 bytes away from _a1..._a4 (=0x80..0xfe).
    mstore(newFreePtr, value)
    return(newFreePtr, 32)
}
// ====
// EVMVersion: >=cancun
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let _1 := memoryguard(0x80)
//         let newFreePtr := add(_1, 128)
//         let _3 := 64
//         let _a1 := _1
//         let _a2 := add(_1, 32)
//         let _a3 := add(_1, 64)
//         let _a4 := add(_1, 96)
//         let value := calldataload(36)
//         let v2 := calldataload(68)
//         let v3 := calldataload(100)
//         let v4 := calldataload(132)
//         mstore(newFreePtr, value)
//         return(newFreePtr, 32)
//     }
// }
