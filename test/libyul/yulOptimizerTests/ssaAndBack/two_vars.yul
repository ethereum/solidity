{
    let a := mload(0)
    let b := mload(a)
    a := mload(b)
    b := mload(a)
    a := mload(b)
    b := mload(a)
    mstore(a, b)
}
// ----
// step: ssaAndBack
//
// {
//     {
//         let a := mload(0)
//         let b := mload(a)
//         let a_1 := mload(b)
//         let b_1 := mload(a_1)
//         let a_2 := mload(b_1)
//         let b_2 := mload(a_2)
//         mstore(a_2, b_2)
//     }
// }
