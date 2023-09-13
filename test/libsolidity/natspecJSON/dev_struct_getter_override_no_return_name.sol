interface IThing {
    ///@return
    function value(uint) external returns (uint128,uint128);
}

contract Thing is IThing {
    struct Value {
        uint128 x;
        uint128 A;
    }
    mapping(uint=>Value) public override value;
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
//             "returns":
//             {
//                 "_0": ""
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
//             "return": "x ",
//             "returns":
//             {
//                 "x": ""
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
