{
    function fun_revert() -> ret { revert(0, 0) }
    function fun_return() -> ret { return(0, 0) }
    function empty(a, b) {}

    // Evaluation order in Yul is right to left so fun_revert() should run first.
    empty(fun_return(), fun_revert())
}
// ----
// step: fullInliner
//
// {
//     {
//         let ret_2 := 0
//         revert(0, 0)
//         let _1 := ret_2
//         let ret_3 := 0
//         return(0, 0)
//         let _2 := ret_3
//         let b_1 := _1
//         let a_1 := _2
//     }
//     function fun_revert() -> ret
//     { revert(0, 0) }
//     function fun_return() -> ret_1
//     { return(0, 0) }
//     function empty(a, b)
//     { }
// }
