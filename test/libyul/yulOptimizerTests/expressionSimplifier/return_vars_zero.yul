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
//     let t, v := f()
//     function f() -> c, d
//     { sstore(0, 7) }
// }
