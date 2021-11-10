{
    function f() -> a {}
    let b := add(7, sub(f(), 7))
    mstore(b, 0)
}
// ----
// step: fullSimplify
//
// {
//     { mstore(f(), 0) }
//     function f() -> a
//     { }
// }
