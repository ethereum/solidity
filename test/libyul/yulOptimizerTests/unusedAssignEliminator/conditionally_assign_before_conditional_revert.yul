{
    function g() {
        if calldataload(10) { revert(0, 0) }
    }
    function f() {
        let a := calldataload(0)
        if calldataload(1) {
            // this can NOT be removed
            a := 2
            g()
        }
        sstore(0, a)
    }
}
// ----
// step: unusedAssignEliminator
//
// {
//     function g()
//     {
//         if calldataload(10) { revert(0, 0) }
//     }
//     function f()
//     {
//         let a := calldataload(0)
//         if calldataload(1)
//         {
//             a := 2
//             g()
//         }
//         sstore(0, a)
//     }
// }
