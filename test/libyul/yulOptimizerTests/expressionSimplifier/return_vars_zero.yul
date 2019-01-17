// return variables are assumed to be zero initially.
{
    function f() -> c, d {
    	let y := add(d, add(c, 7))
    }
}
// ----
// expressionSimplifier
// {
//     function f() -> c, d
//     {
//         let y := 7
//     }
// }
