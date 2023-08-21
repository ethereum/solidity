abstract contract C {
    /// @return value The value returned by this function.
    function vote() public virtual returns (uint value);
}
// ----
// ----
// :C devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "vote()":
//         {
//             "returns":
//             {
//                 "value": "The value returned by this function."
//             }
//         }
//     },
//     "version": 1
// }
//
// :C userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
