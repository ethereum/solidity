{
    let x
    for {} x { sstore(1, x) } {
        if x { continue }
        x := 0
        sstore(0, x)
    }
    sstore(0, x)
}
// ====
// step: conditionalUnsimplifier
// ----
// {
//     let x
//     for { } x { sstore(1, x) }
//     {
//         if x { continue }
//         sstore(0, x)
//     }
//     sstore(0, x)
// }
