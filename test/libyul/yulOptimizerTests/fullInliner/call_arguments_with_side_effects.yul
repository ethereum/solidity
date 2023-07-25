{
    function fun_revert() -> ret { revert(0, 0) }
    function fun_return() -> ret { return(0, 0) }
    function empty(a, b) {}

    // Evaluation order in Yul is right to left so fun_revert() should run first.
    empty(fun_return(), fun_revert())
}
// ====
// EVMVersion: >=shanghai
// ----
// step: fullInliner
//
// {
//     {
//         let ret_3 := 0
//         revert(0, 0)
//         let _1 := ret_3
//         let ret_1_4 := 0
//         return(0, 0)
//         let _2 := ret_1_4
//         let b_5 := _1
//         let a_6 := _2
//     }
//     function fun_revert() -> ret
//     { revert(0, 0) }
//     function fun_return() -> ret_1
//     { return(0, 0) }
//     function empty(a, b)
//     { }
// }
