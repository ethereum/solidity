{
	let a:u256 function f() {}
}
// ====
// dialect: yul
// ----
// step: functionGrouper
//
// {
//     { let a }
//     function f()
//     { }
// }
