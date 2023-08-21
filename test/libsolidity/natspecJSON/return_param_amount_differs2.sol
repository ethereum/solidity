interface IThing {
    /// @param v value to search for
    /// @return x a number
    /// @return y another number
    function value(uint256 v) external view returns (uint128 x, uint128 y);
}

contract Thing is IThing {
    struct Value {
        uint128 x;
        uint128 y;
    }

    mapping(uint256=>Value) public override value;
}
// ----
// ----
// :IThing devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "value(uint256)":
//         {
//             "params":
//             {
//                 "v": "value to search for"
//             },
//             "returns":
//             {
//                 "x": "a number",
//                 "y": "another number"
//             }
//         }
//     },
//     "version": 1
// }
//
// :IThing userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
//
// :Thing devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "stateVariables":
//     {
//         "value":
//         {
//             "params":
//             {
//                 "v": "value to search for"
//             },
//             "returns":
//             {
//                 "x": "a number",
//                 "y": "another number"
//             }
//         }
//     },
//     "version": 1
// }
//
// :Thing userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
