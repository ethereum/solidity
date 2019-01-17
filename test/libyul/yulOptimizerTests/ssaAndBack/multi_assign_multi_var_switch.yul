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
// ssaAndBack
// {
//     let a := mload(0)
//     let b := mload(1)
//     switch mload(2)
//     case 0 {
//         let a_3 := mload(b)
//         let b_4 := mload(a_3)
//         a := mload(b_4)
//         b := mload(a)
//     }
//     default {
//         let b_7 := mload(a)
//         let a_8 := mload(b_7)
//         b := mload(a_8)
//         a := mload(b)
//     }
//     mstore(a, b)
// }
