{
    let x := mload(0)
    let y := mload(0)
    if x { revert(0, 0) }
    if y { revert(0, 0) }
    for {} and(x, y) {} {
        x := 2
    }
}
// ====
// step: conditionalSimplifier
// ----
// {
//     let x := mload(0)
//     let y := mload(0)
//     if x { revert(0, 0) }
//     x := 0
//     if y { revert(0, 0) }
//     y := 0
//     for { } and(x, y) { }
//     { x := 2 }
// }
