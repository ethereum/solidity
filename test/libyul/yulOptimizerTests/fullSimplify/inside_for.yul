{
    let x := calldataload(3)
    for { let a := 10 } iszero(eq(a, sub(x, calldataload(3)))) { a := add(a, 1) } {}
}
// ----
// fullSimplify
// {
//     for {
//         let a := 10
//     }
//     iszero(iszero(a))
//     {
//         a := add(a, 1)
//     }
//     {
//     }
// }
