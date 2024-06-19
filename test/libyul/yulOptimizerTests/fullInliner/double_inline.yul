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
//         let b_1 := 0
//         let c_1 := 0
//         b_1 := sload(mload(a_1))
//         c_1 := 3
//         let b3 := b_1
//         let a_2 := c_1
//         let b_2 := 0
//         let c_2 := 0
//         b_2 := sload(mload(a_2))
//         c_2 := 3
//         let b4 := b_2
//         let c4 := c_2
//     }
//     function f(a) -> b, c
//     {
//         b := sload(mload(a))
//         c := 3
//     }
// }
