{
    for { let i := 1 } iszero(eq(i, 10)) { i := add(i, 1) }
    {
        tstore(0, i)
        sstore(i, tload(0))
    }
}
// ====
// EVMVersion: >=cancun
// ----
// step: loopInvariantCodeMotion
//
// {
//     let i := 1
//     for { } iszero(eq(i, 10)) { i := add(i, 1) }
//     {
//         tstore(0, i)
//         sstore(i, tload(0))
//     }
// }
