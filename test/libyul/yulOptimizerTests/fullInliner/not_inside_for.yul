{
	for { let x := f(0) } f(x) { x := f(x) }
	{
		let t := f(x)
	}
	function f(a) -> r {
		sstore(a, 0)
		r := a
	}
}
// ----
// fullInliner
// {
//     {
//         for {
//             let a_3 := 0
//             let r_4 := 0
//             sstore(a_3, 0)
//             r_4 := a_3
//             let x := r_4
//         }
//         f(x)
//         {
//             let a_6 := x
//             let r_7 := 0
//             sstore(a_6, 0)
//             r_7 := a_6
//             x := r_7
//         }
//         {
//             let a_9 := x
//             let r_10 := 0
//             sstore(a_9, 0)
//             r_10 := a_9
//             let t := r_10
//         }
//     }
//     function f(a) -> r
//     {
//         sstore(a, 0)
//         r := a
//     }
// }
