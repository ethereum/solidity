interface IThing {
    /// @return x a number
    /// @return y another number
    function value() external view returns (uint128 x, uint128 y);
}

contract Thing is IThing {
    struct Value {
        uint128 x;
        uint128 y;
    }

    Value public override value;
}

// ----
// ----
// :IThing devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "value()":
//         {
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
