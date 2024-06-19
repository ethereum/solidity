{
    let a := mload(0)
    let b := mload(1)
    switch mload(2)
    case 0 {
        a := mload(b)
        b := mload(a)
        a := mload(b)
        b := mload(a)
    }
    default {
        b := mload(a)
        a := mload(b)
        b := mload(a)
        a := mload(b)
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
//         switch mload(2)
//         case 0 {
//             let a_1 := mload(b)
//             let b_1 := mload(a_1)
//             a := mload(b_1)
//             b := mload(a)
//         }
//         default {
//             let b_2 := mload(a)
//             let a_2 := mload(b_2)
//             b := mload(a_2)
//             a := mload(b)
//         }
//         mstore(a, b)
//     }
// }
