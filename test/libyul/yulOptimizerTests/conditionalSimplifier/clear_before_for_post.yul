{
    let x
    for {} x { sstore(1, x) } {
        if x { continue }
        // x is 0 here, but should not be 0
        // anymore in the for loop post block
        sstore(0, x)
    }
    sstore(0, x)
}
// ====
// step: conditionalSimplifier
// ----
// {
//     let x
//     for { } x { sstore(1, x) }
//     {
//         if x { continue }
//         x := 0
//         sstore(0, x)
//     }
//     sstore(0, x)
// }
