{
    let a := address()
    let ret := balance(a)
    sstore(a, ret)
}
// ====
// EVMVersion: >=istanbul
// ----
// step: expressionSimplifier
//
// {
//     let a := address()
//     sstore(a, selfbalance())
// }
