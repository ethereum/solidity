{
    function g() -> x {
        x := 7
        if calldataload(0) {
            x := 3
            reverting()
        }
        if calldataload(1) {
            x := 3
            leave
        }
        x := 2
        reverting()
    }
    function reverting() { revert(0, 0) }
    sstore(0, g())
}
// ----
// step: unusedAssignEliminator
//
// {
//     function g() -> x
//     {
//         if calldataload(0) { reverting() }
//         if calldataload(1)
//         {
//             x := 3
//             leave
//         }
//         reverting()
//     }
//     function reverting()
//     { revert(0, 0) }
//     sstore(0, g())
// }
