{
    function f(a) -> b { }
    mstore(0, sub(f(0), f(1)))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: fullSimplify
//
// {
//     { mstore(0, sub(f(0), f(1))) }
//     function f(a) -> b
//     { }
// }
