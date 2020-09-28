// return variables are assumed to be zero initially.
{
    function f() -> c, d {
    	let y := add(d, add(c, 7))
        sstore(0, y)
    }
    let t, v := f()
}
// ----
// step: expressionSimplifier
//
// {
//     function f() -> c, d
//     { sstore(d, 7) }
//     let t, v := f()
// }
