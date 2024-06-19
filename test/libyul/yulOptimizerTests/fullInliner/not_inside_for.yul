{
	let x := f(0)
	for {  } f(x) { x := f(x) }
	{
		let t := f(x)
	}
	function f(a) -> r {
		sstore(a, 0)
		r := a
	}
}
// ----
// step: fullInliner
//
// {
//     {
//         let a_1 := 0
//         let r_1 := 0
//         sstore(a_1, 0)
//         r_1 := a_1
//         let x := r_1
//         for { }
//         f(x)
//         {
//             let a_2 := x
//             let r_2 := 0
//             sstore(a_2, 0)
//             r_2 := a_2
//             x := r_2
//         }
//         {
//             let a_3 := x
//             let r_3 := 0
//             sstore(a_3, 0)
//             r_3 := a_3
//             let t := r_3
//         }
//     }
//     function f(a) -> r
//     {
//         sstore(a, 0)
//         r := a
//     }
// }
