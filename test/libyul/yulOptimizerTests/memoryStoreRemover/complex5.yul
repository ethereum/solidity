{
    let x := 0
    let y := 50
    // cannot be removed, since z is not in SSA form
    mstore(x, y)

    let z := 200
    // z is not in SSA form, so the solver would use a free variable.
    z := 100
    pop(mload(z))
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 0
//     let y := 50
//     mstore(x, y)
//     let z := 200
//     z := 100
//     pop(mload(z))
// }
