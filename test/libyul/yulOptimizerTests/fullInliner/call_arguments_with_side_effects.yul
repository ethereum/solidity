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
//         let ret_7 := 0
//         revert(0, 0)
//         let _1 := ret_7
//         let ret_1_10 := 0
//         return(0, 0)
//         let _2 := ret_1_10
//         let b_13 := _1
//         let a_14 := _2
//     }
//     function fun_revert() -> ret
//     { revert(0, 0) }
//     function fun_return() -> ret_1
//     { return(0, 0) }
//     function empty(a, b)
//     { }
// }
