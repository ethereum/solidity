{
    let a := mload(0)
    // This could be more efficient:
    // all cases could use the value of the variable from just before
    // the switch and not just the first
    switch a
    case 0 { a := add(a, 4) }
    default { a := add(a, 8) }
    mstore(0, a)
}
// ====
// step: ssaTransform
// ----
// {
//     let a_1 := mload(0)
//     let a := a_1
//     switch a_1
//     case 0 {
//         let a_2 := add(a_1, 4)
//         a := a_2
//     }
//     default {
//         let a_4 := a
//         let a_3 := add(a_4, 8)
//         a := a_3
//     }
//     let a_5 := a
//     mstore(0, a_5)
// }
