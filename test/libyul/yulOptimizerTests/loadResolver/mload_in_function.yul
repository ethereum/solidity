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
//     {
//         let _1 := 1337
//         let _2 := 0
//         mstore(_2, _1)
//         foo(42)
//         sstore(_2, mload(_2))
//     }
//     function foo(x)
//     {
//         for { }
//         x
//         {
//             let _7 := 0
//             x := mload(_7)
//             mstore(_7, _7)
//         }
//         { }
//     }
// }
