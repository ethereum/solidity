{
    let a := mload(0)
    for { mstore(0, a) } a { a := add(a, 3) }
    {
        mstore(0, a)
    }
    mstore(0, a)
}
// ====
// step: ssaTransform
// ----
// {
//     let a_1 := mload(0)
//     let a := a_1
//     for { mstore(0, a_1) }
//     a
//     {
//         let a_4 := a
//         let a_2 := add(a_4, 3)
//         a := a_2
//     }
//     {
//         let a_3 := a
//         mstore(0, a_3)
//     }
//     let a_5 := a
//     mstore(0, a_5)
// }
