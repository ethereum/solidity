/// @custom:x one two three
/// @custom:y line
/// break
/// @custom:t one
/// @custom:t two
contract A {
    /// @custom:note statevar
    uint x;
    /// @custom:since 2014
    function g(int x) public pure virtual returns (int, int) { return (1, 2); }
}

// ----
// ----
// :A devdoc
// {
//     "custom:t": "onetwo",
//     "custom:x": "one two three",
//     "custom:y": "line break",
//     "kind": "dev",
//     "methods":
//     {
//         "g(int256)":
//         {
//             "custom:since": "2014"
//         }
//     },
//     "stateVariables":
//     {
//         "x":
//         {
//             "custom:note": "statevar"
//         }
//     },
//     "version": 1
// }
