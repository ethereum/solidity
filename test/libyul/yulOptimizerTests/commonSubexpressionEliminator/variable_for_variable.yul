{

    let a := mload(0)
    let b := add(a, 7)
    let c := a
    let d := c
    let x := add(a, b)
    // CSE has to recognize equality with x here.
    let y := add(d, add(c, 7))
    // some reassignments
    b := mload(a)
    a := b
    mstore(2, a)
}
// ====
// step: commonSubexpressionEliminator
// ----
// {
//     let a := mload(0)
//     let b := add(a, 7)
//     let c := a
//     let d := a
//     let x := add(a, b)
//     let y := x
//     b := mload(a)
//     a := b
//     mstore(2, b)
// }
