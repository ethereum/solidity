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
//         let a_2 := calldataload(0)
//         let b_3 := 0
//         let c_4 := 0
//         b_3 := sload(mload(a_2))
//         c_4 := 3
//         let b3 := b_3
//         let a_6 := c_4
//         let b_7 := 0
//         let c_8 := 0
//         b_7 := sload(mload(a_6))
//         c_8 := 3
//         let b4 := b_7
//         let c4 := c_8
//     }
//     function f(a) -> b, c
//     {
//         b := sload(mload(a))
//         c := 3
//     }
// }
