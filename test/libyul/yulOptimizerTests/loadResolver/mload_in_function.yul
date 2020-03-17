{
        function foo(x) {
                for {} x { x := mload(0) mstore(0, 0)} {}
        }
        mstore(0, 1337)
        foo(42)
        sstore(0, mload(0))
}
// ----
// step: loadResolver
//
// {
//     function foo(x)
//     {
//         for { }
//         x
//         {
//             let _1 := 0
//             x := mload(_1)
//             mstore(_1, _1)
//         }
//         { }
//     }
//     let _4 := 1337
//     let _5 := 0
//     mstore(_5, _4)
//     foo(42)
//     sstore(_5, mload(_5))
// }
