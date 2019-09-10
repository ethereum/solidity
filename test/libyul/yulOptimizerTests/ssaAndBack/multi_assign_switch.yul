{
    let a := mload(0)
    switch mload(1)
    case 0 {
        a := mload(1)
        a := mload(2)
        a := mload(3)
    }
    default {
        a := mload(4)
        a := mload(5)
        a := mload(6)
    }
    mstore(a, 0)
}
// ====
// step: ssaAndBack
// ----
// {
//     let a := mload(0)
//     switch mload(1)
//     case 0 { a := mload(3) }
//     default { a := mload(6) }
//     mstore(a, 0)
// }
