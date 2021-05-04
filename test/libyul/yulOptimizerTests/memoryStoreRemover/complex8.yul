{
    let x := 0
    let y := 10
    f(x, y)
    function f(a, b) {
        // cannot be removed, since a and b are considered as free variables
        mstore(a, b)
    }
}
// ----
// step: memoryStoreRemover
//
// {
//     let x := 0
//     let y := 10
//     f(x, y)
//     function f(a, b)
//     { mstore(a, b) }
// }
