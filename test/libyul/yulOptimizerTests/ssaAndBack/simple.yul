{
    let a := mload(0)
    a := mload(1)
    mstore(a, 0)
}
// ----
// step: ssaAndBack
//
// {
//     {
//         let a := mload(1)
//         mstore(a, 0)
//     }
// }
