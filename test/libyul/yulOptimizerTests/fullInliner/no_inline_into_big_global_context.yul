{
	function f(a) -> b {
        let x := mload(a)
        b := sload(x)
    }
    // This will stop inlining at some point because
    // the global context gets too big.
    let x := f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(2)))))))))))))))))))
}
// ----
// fullInliner
// {
//     {
//         let f_a := 2
//         let f_b
//         f_b := sload(mload(f_a))
//         let f_a_20 := f_b
//         let f_b_21
//         f_b_21 := sload(mload(f_a_20))
//         let f_a_23 := f_b_21
//         let f_b_24
//         f_b_24 := sload(mload(f_a_23))
//         let f_a_26 := f_b_24
//         let f_b_27
//         f_b_27 := sload(mload(f_a_26))
//         let f_a_29 := f_b_27
//         let f_b_30
//         f_b_30 := sload(mload(f_a_29))
//         let f_a_32 := f_b_30
//         let f_b_33
//         f_b_33 := sload(mload(f_a_32))
//         let f_a_35 := f_b_33
//         let f_b_36
//         f_b_36 := sload(mload(f_a_35))
//         let x_1 := f(f(f(f(f(f(f(f(f(f(f(f(f_b_36))))))))))))
//     }
//     function f(a) -> b
//     {
//         b := sload(mload(a))
//     }
// }
