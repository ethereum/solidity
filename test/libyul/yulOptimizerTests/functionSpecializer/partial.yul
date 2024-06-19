{
    // All arguments are constants
    let x := 2
    f(1, x, 3)

    function f(a, b, c) {
        sstore(a, b)
        sstore(b, c)
        // Prevents getting inlined
        if calldataload(0) { leave }
    }
}
// ----
// step: functionSpecializer
//
// {
//     let x := 2
//     f_1(x)
//     function f_1(b_1)
//     {
//         let a_1 := 1
//         let c_1 := 3
//         sstore(a_1, b_1)
//         sstore(b_1, c_1)
//         if calldataload(0) { leave }
//     }
//     function f(a, b, c)
//     {
//         sstore(a, b)
//         sstore(b, c)
//         if calldataload(0) { leave }
//     }
// }
