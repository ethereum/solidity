{
    function f() {
        let a := calldataload(0)
        if calldataload(1) {
            // this can be removed
            a := 2
            leave
        }
        sstore(0, a)
    }
}
// ----
// step: unusedAssignEliminator
//
// {
//     function f()
//     {
//         let a := calldataload(0)
//         if calldataload(1) { leave }
//         sstore(0, a)
//     }
// }
