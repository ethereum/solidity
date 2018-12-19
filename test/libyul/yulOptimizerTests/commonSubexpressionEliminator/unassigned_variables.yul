{
    // This does not replace b by a because there is no
    // explicit assignment, even though both hold the same value.
    let a
    let b
    mstore(sub(a, b), 7)
}
// ----
// commonSubexpressionEliminator
// {
//     let a
//     let b
//     mstore(sub(a, b), 7)
// }
