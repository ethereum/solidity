{
    let x := 10
    if x { sstore(0, 1) }
}
// ----
// step: conditionalBranchFlattener
//
// {
//     let x := 10
//     if x { sstore(0, 1) }
// }
