// yul
{
	let a:u256 function f() {}
}
// ----
// functionGrouper
// {
//     {
//         let a:u256
//     }
//     function f()
//     {
//     }
// }
