{
	let f_2
	function f(x_12) -> x, y_14
	{
		let y := x_12
		y_14 := y
		x := y_14
	}
	let f_10
}
// ----
// step: varNameCleaner
//
// {
//     {
//         let f_2
//         let f_10
//     }
//     function f(x_12) -> x, y_14
//     {
//         let y := x_12
//         y_14 := y
//         x := y_14
//     }
// }
