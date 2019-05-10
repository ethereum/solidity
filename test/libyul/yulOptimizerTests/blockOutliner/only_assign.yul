{
    let a
    let b
    let c
    {
        a := mul(b,c)
    }
    {
        b := mul(c,a)
    }
    {
        c := mul(a,b)
    }
}
// ====
// step: blockOutliner
// ----
// {
//     let a
//     let b
//     let c
//     { a := outlined$36$(b, c) }
//     { b := outlined$36$(c, a) }
//     { c := outlined$36$(a, b) }
//     function outlined$36$(b, c) -> a
//     { a := mul(b, c) }
// }
