{
    function f() -> x {
        // can re-use x
        let y := 0
        mstore(y, 7)
    }
    let a
    // can re-use a
    let b := 0
    sstore(a, b)
}
// ----
// step: commonSubexpressionEliminator
//
// {
//     let a
//     let b := a
//     sstore(a, a)
//     function f() -> x
//     {
//         let y := 0
//         mstore(y, 7)
//     }
// }
