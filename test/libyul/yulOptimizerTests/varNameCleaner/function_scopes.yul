{
	function f() { let x_1 := 0 }
	function g() { let x_2 := 0 }
}
// ----
// varNameCleaner
// {
//     function f()
//     {
//         let x := 0
//     }
//     function g()
//     {
//         let x := 0
//     }
// }
