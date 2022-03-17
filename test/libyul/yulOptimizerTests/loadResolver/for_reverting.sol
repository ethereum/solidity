{
    sstore(0, 123213)
    let x := 0
    for {} sload(0) { mstore(0x50, sload(0)) } {
        if calldataload(0) { mstore(0, 1) revert(0, 0) }
        let t := sload(0)
        if calldataload(0) { mstore(0, 2) break }
        let u := sload(0)
        if calldataload(0) { mstore(0, 3) continue }
        let v := sload(0)
        mstore(0, t)
        mstore(0x20, u)
        mstore(0x40, v)
    }
    let z := sload(0)
    sstore(6, z)
    sstore(7, keccak256(0, 0x100))
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 123213
//         let _2 := 0
//         sstore(_2, _1)
//         for { } _1 { mstore(0x50, _1) }
//         {
//             let _7 := calldataload(_2)
//             if _7
//             {
//                 mstore(_2, 1)
//                 revert(_2, _2)
//             }
//             let t := _1
//             if _7
//             {
//                 mstore(_2, 2)
//                 break
//             }
//             let u := _1
//             if _7
//             {
//                 mstore(_2, 3)
//                 continue
//             }
//             let v := _1
//             mstore(_2, t)
//             mstore(0x20, u)
//             mstore(0x40, v)
//         }
//         sstore(6, _1)
//         sstore(7, keccak256(_2, 0x100))
//     }
// }
