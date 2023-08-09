contract test {
    /// @param a the parameter a is really nice and very useful
    /// @param second the second parameter is not very useful, it just provides additional confusion
    constructor(uint a, uint second) { }
}

// ----
// ----
// :test devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "constructor":
//         {
//             "params":
//             {
//                 "a": "the parameter a is really nice and very useful",
//                 "second": "the second parameter is not very useful, it just provides additional confusion"
//             }
//         }
//     },
//     "version": 1
// }
