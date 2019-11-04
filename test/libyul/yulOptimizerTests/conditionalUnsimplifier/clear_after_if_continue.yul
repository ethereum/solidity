{
    let y := mload(0x20)
    for {} and(y, 8) { pop(y) } {
        if y { continue }
        y := 0
    }
}
// ====
// step: conditionalUnsimplifier
// ----
// {
//     let y := mload(0x20)
//     for { } and(y, 8) { pop(y) }
//     { if y { continue } }
// }
