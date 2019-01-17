{
	function f(a) -> b, c { let x := mload(a) b := sload(x) c := 3 }
    let a1 := calldataload(0)
    let b3, c3 := f(a1)
    let b4, c4 := f(c3)
}
// ----
// fullInliner
// {
//     {
//         let f_a := calldataload(0)
//         let f_b := 0
//         let f_c := 0
//         f_b := sload(mload(f_a))
//         f_c := 3
//         let b3 := f_b
//         let f_a_2 := f_c
//         let f_b_3 := 0
//         let f_c_4 := 0
//         f_b_3 := sload(mload(f_a_2))
//         f_c_4 := 3
//         let b4 := f_b_3
//         let c4 := f_c_4
//     }
//     function f(a) -> b, c
//     {
//         b := sload(mload(a))
//         c := 3
//     }
// }
