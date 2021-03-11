{
    let x := 0
    function f() -> r {
        r := call(0, 0, 0, 0, 0, 0, 0)
        r := add(r, call(0, 0, 0, 0, 0, 0, 0))
    }
    sstore(f(), 0)
}
// ----
// step: zeroByReturndatasizeReplacer
//
// {
//     let x := returndatasize()
//     function f() -> r
//     {
//         r := call(returndatasize(), returndatasize(), returndatasize(), returndatasize(), returndatasize(), returndatasize(), returndatasize())
//         r := add(r, call(0, 0, 0, 0, 0, 0, 0))
//     }
//     sstore(f(), returndatasize())
// }
