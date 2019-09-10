{
    let x := 1
    for { } calldataload(0) { }
    {
        // This will go out of scope at the end of the block,
        // but the continue/break statements still refer to it.
        {
            let y := 9
            if callvalue() {
                y := 2 // will be removed
                break
            }
            if eq(callvalue(), 3) {
                y := 12 // will be removed
                continue
            }
        }
    }
    mstore(x, 0x42)
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let x := 1
//     for { } calldataload(0) { }
//     {
//         {
//             let y := 9
//             if callvalue() { break }
//             if eq(callvalue(), 3) { continue }
//         }
//     }
//     mstore(x, 0x42)
// }
