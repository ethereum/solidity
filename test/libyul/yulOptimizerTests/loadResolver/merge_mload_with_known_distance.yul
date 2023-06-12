{
    let x := mload(calldataload(0))
    if calldataload(1) {
        mstore(add(calldataload(0), 0x20), 1)
    }
    let t := mload(add(calldataload(0), 0x20))
    let q := mload(calldataload(0))
    sstore(t, q)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let _3 := 1
//         if calldataload(_3)
//         {
//             mstore(add(calldataload(0), 0x20), _3)
//         }
//         let t := mload(add(calldataload(0), 0x20))
//         sstore(t, mload(calldataload(0)))
//     }
// }
