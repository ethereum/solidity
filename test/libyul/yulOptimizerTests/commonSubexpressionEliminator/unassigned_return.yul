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
// commonSubexpressionEliminator
// {
//     function f() -> x
//     {
//         let y := x
//         mstore(x, 7)
//     }
//     let a
//     let b := a
//     sstore(a, a)
// }
