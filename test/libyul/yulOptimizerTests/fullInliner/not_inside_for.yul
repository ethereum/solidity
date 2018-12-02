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
//             let f_a_1 := x
//             let f_r_1
//             sstore(f_a_1, 0)
//             f_r_1 := f_a_1
//             x := f_r_1
//         }
//         {
//             let f_a_2 := x
//             let f_r_2
//             sstore(f_a_2, 0)
//             f_r_2 := f_a_2
//             let t := f_r_2
//         }
//     }
//     function f(a) -> r
//     {
//         sstore(a, 0)
//         r := a
//     }
// }
