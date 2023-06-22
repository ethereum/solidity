{
	function f(a) -> b, c { let x := mload(a) b := sload(x) c := 3 }
    let a1 := calldataload(0)
    let b3, c3 := f(a1)
    let b4, c4 := f(c3)
}
// ----
// step: fullInliner
//
// {
//     {
//         let a_1 := calldataload(0)
//         let b_2 := 0
//         let c_3 := 0
//         b_2 := sload(mload(a_1))
//         c_3 := 3
//         let b3 := b_2
//         let a_5 := c_3
//         let b_6 := 0
//         let c_7 := 0
//         b_6 := sload(mload(a_5))
//         c_7 := 3
//         let b4 := b_6
//         let c4 := c_7
//     }
//     function f(a) -> b, c
//     {
//         b := sload(mload(a))
//         c := 3
//     }
// }
