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
//             let f_a := 0
//             let f_r
//             sstore(f_a, 0)
//             f_r := f_a
//             let x := f_r
//         }
//         f(x)
//         {
//             let f_a_3 := x
//             let f_r_4
//             sstore(f_a_3, 0)
//             f_r_4 := f_a_3
//             x := f_r_4
//         }
//         {
//             let f_a_6 := x
//             let f_r_7
//             sstore(f_a_6, 0)
//             f_r_7 := f_a_6
//             let t := f_r_7
//         }
//     }
//     function f(a) -> r
//     {
//         sstore(a, 0)
//         r := a
//     }
// }
