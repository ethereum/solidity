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
//     for {
//         mstore(0, a_1)
//     }
//     a
//     {
//         let a_2 := add(a, 3)
//         a := a_2
//     }
//     {
//         mstore(0, a)
//     }
//     mstore(0, a)
// }
