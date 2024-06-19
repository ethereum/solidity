{
    let a := mload(0)
    let b := mload(1)
    if mload(2) {
        a := mload(b)
        b := mload(a)
        a := mload(b)
        b := mload(a)
    }
    mstore(a, b)
}
// ----
// step: ssaAndBack
//
// {
//     {
//         let a := mload(0)
//         let b := mload(1)
//         if mload(2)
//         {
//             let a_1 := mload(b)
//             let b_1 := mload(a_1)
//             a := mload(b_1)
//             b := mload(a)
//         }
//         mstore(a, b)
//     }
// }
