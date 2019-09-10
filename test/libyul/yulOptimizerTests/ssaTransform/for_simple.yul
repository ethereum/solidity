{
    let a := mload(0)
    a := add(a, 1)
    if a {
        a := add(a, 2)
    }
    {
        a := add(a, 4)
    }
    for { a := add(a, 3) } a { a := add(a, 6) }
    {
        a := add(a, 12)
    }
    a := add(a, 8)
}
// ====
// step: ssaTransform
// ----
// {
//     let a_1 := mload(0)
//     let a := a_1
//     let a_2 := add(a_1, 1)
//     a := a_2
//     if a_2
//     {
//         let a_3 := add(a_2, 2)
//         a := a_3
//     }
//     {
//         let a_4 := add(a, 4)
//         a := a_4
//     }
//     for {
//         let a_5 := add(a, 3)
//         a := a_5
//     }
//     a
//     {
//         let a_7 := add(a, 6)
//         a := a_7
//     }
//     {
//         let a_6 := add(a, 12)
//         a := a_6
//     }
//     let a_8 := add(a, 8)
//     a := a_8
// }
