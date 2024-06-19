{
	function f() { let f_1 }
	let f_2
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
//     function f()
//     { let f_1 }
// }
