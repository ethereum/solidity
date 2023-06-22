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
// ====
// EVMVersion: >=shanghai
// ----
// step: fullInliner
//
// {
//     {
//         let a_1 := 0
//         let r_2 := 0
//         sstore(a_1, 0)
//         r_2 := a_1
//         let x := r_2
//         for { }
//         f(x)
//         {
//             let a_3 := x
//             let r_4 := 0
//             sstore(a_3, 0)
//             r_4 := a_3
//             x := r_4
//         }
//         {
//             let a_5 := x
//             let r_6 := 0
//             sstore(a_5, 0)
//             r_6 := a_5
//             let t := r_6
//         }
//     }
//     function f(a) -> r
//     {
//         sstore(a, 0)
//         r := a
//     }
// }
