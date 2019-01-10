{
	function f(a) -> b {
        let x := mload(a)
        b := sload(x)
    }
    // This will stop inlining at some point because
    // the function gets too big.
    function g() -> x {
        x := f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(2)))))))))))))))))))
    }
}
// ----
// fullInliner
// {
//     function f(a) -> b
//     {
//         b := sload(mload(a))
//     }
//     function g() -> x_1
//     {
//         let f_a := 2
//         let f_b := 0
//         f_b := sload(mload(f_a))
//         let f_a_20 := f_b
//         let f_b_21 := 0
//         f_b_21 := sload(mload(f_a_20))
//         let f_a_23 := f_b_21
//         let f_b_24 := 0
//         f_b_24 := sload(mload(f_a_23))
//         let f_a_26 := f_b_24
//         let f_b_27 := 0
//         f_b_27 := sload(mload(f_a_26))
//         let f_a_29 := f_b_27
//         let f_b_30 := 0
//         f_b_30 := sload(mload(f_a_29))
//         let f_a_32 := f_b_30
//         let f_b_33 := 0
//         f_b_33 := sload(mload(f_a_32))
//         let f_a_35 := f_b_33
//         let f_b_36 := 0
//         f_b_36 := sload(mload(f_a_35))
//         let f_a_38 := f_b_36
//         let f_b_39 := 0
//         f_b_39 := sload(mload(f_a_38))
//         let f_a_41 := f_b_39
//         let f_b_42 := 0
//         f_b_42 := sload(mload(f_a_41))
//         let f_a_44 := f_b_42
//         let f_b_45 := 0
//         f_b_45 := sload(mload(f_a_44))
//         let f_a_47 := f_b_45
//         let f_b_48 := 0
//         f_b_48 := sload(mload(f_a_47))
//         let f_a_50 := f_b_48
//         let f_b_51 := 0
//         f_b_51 := sload(mload(f_a_50))
//         let f_a_53 := f_b_51
//         let f_b_54 := 0
//         f_b_54 := sload(mload(f_a_53))
//         x_1 := f(f(f(f(f(f(f_b_54))))))
//     }
// }
