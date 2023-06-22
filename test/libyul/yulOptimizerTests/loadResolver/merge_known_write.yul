{
    mstore(calldataload(0), calldataload(10))
    if calldataload(1) {
        mstore(calldataload(0), 1)
    }
    let t := mload(0)
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
//         let _2 := calldataload(10)
//         let _3 := calldataload(0)
//         mstore(_3, _2)
//         let _4 := 1
//         if calldataload(_4) { mstore(_3, _4) }
//         let t := mload(0)
//         sstore(t, mload(_3))
//     }
// }
