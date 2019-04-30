{
    let x := calldataload(3)
    let a := 10
    for { } iszero(eq(a, sub(x, calldataload(3)))) { a := add(a, 1) } {}
}
// ====
// step: fullSimplify
// ----
// {
//     let a := 10
//     for {
//     }
//     iszero(iszero(a))
//     {
//         a := add(a, 1)
//     }
//     {
//     }
// }
