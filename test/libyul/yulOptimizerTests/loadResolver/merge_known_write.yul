{
    mstore(calldataload(0), calldataload(10))
    if calldataload(1) {
        mstore(calldataload(0), 1)
    }
    let t := mload(0)
    let q := mload(calldataload(0))
    sstore(t, q)
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := calldataload(10)
//         let _2 := 0
//         let _3 := calldataload(_2)
//         mstore(_3, _1)
//         let _4 := 1
//         if calldataload(_4) { mstore(_3, _4) }
//         let t := mload(_2)
//         sstore(t, mload(_3))
//     }
// }
