{
    let a := mload(0)
    switch mload(1)
    case 0 {
        a := mload(1)
    }
    default {
        a := mload(2)
    }
    mstore(a, 0)
}
// ====
// step: ssaAndBack
// ----
// {
//     let a := mload(0)
//     switch mload(1)
//     case 0 {
//         a := mload(1)
//     }
//     default {
//         a := mload(2)
//     }
//     mstore(a, 0)
// }
