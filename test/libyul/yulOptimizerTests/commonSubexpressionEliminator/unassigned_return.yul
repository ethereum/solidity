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
// ====
// EVMVersion: >=shanghai
// ----
// step: commonSubexpressionEliminator
//
// {
//     let a
//     let b := 0
//     sstore(a, b)
//     function f() -> x
//     {
//         let y := 0
//         mstore(y, 7)
//     }
// }
