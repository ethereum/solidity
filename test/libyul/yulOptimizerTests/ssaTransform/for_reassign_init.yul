{
    let a := mload(0)
    for { a := add(a, 3) } a { mstore(0, a) }
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
//     for {
//         let a_2 := add(a_1, 3)
//         a := a_2
//     }
//     a
//     {
//         mstore(0, a)
//     }
//     {
//         mstore(0, a)
//     }
//     mstore(0, a)
// }
