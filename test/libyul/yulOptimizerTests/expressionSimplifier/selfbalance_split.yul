{
    let a := address()
    let ret := balance(a)
}
// ====
// EVMVersion: >=istanbul
// ----
// step: expressionSimplifier
//
// {
//     let a := address()
//     let ret := selfbalance()
// }
