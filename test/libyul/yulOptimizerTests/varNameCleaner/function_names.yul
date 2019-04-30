{
	let f_2
	function f() { let f_1 }
	let f_10
}
// ====
// step: varNameCleaner
// ----
// {
//     let f_1
//     function f()
//     {
//         let f_1
//     }
//     let f_2
// }
