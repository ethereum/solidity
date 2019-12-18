{
	let a:u256 function f() {}
}
// ====
// step: functionGrouper
// dialect: yul
// ----
// {
//     { let a:u256 }
//     function f()
//     { }
// }
