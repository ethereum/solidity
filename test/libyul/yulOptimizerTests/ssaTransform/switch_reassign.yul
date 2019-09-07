{
    let a := mload(0)
    switch a
    case 0 { a := add(a, 4) }
    default { }
    // should still create an SSA variable for a
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
//     default { }
//     let a_3 := a
//     mstore(0, a_3)
// }
