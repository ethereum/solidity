{
    let a := 10
    let x := 20
    {
        let b := calldataload(0)
        let d := calldataload(1)
        x := d
    }
    // We had a bug where "calldataload(0)" was incorrectly replaced by "b"
    mstore(0, calldataload(0))
    mstore(0, x)
}
// ----
// commonSubexpressionEliminator
// {
//     let a := 10
//     let x := 20
//     {
//         let b := calldataload(0)
//         let d := calldataload(1)
//         x := d
//     }
//     mstore(0, b)
//     mstore(0, x)
// }
