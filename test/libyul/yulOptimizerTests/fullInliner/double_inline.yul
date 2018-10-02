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
//         let f_b
//         let f_c
//         f_b := sload(mload(f_a))
//         f_c := 3
//         let b3 := f_b
//         let f_a_1 := f_c
//         let f_b_1
//         let f_c_1
//         f_b_1 := sload(mload(f_a_1))
//         f_c_1 := 3
//         let b4 := f_b_1
//         let c4 := f_c_1
//     }
//     function f(a) -> b, c
//     {
//         b := sload(mload(a))
//         c := 3
//     }
// }
