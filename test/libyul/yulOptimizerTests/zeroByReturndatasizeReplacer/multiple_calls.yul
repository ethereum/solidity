{
    let x := 0
    function f() -> r {
        // cannot replace zeroes due to the second call to f below
        r := call(0, 0, 0, 0, 0, 0, 0)
        r := add(r, call(0, 0, 0, 0, 0, 0, 0))
    }
    pop(f())
    pop(f())
}
// ----
// step: zeroByReturndatasizeReplacer
//
// {
//     let x := returndatasize()
//     function f() -> r
//     {
//         r := call(0, 0, 0, 0, 0, 0, 0)
//         r := add(r, call(0, 0, 0, 0, 0, 0, 0))
//     }
//     pop(f())
//     pop(f())
// }
