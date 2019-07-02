// bug #1801
contract test {
    function f(string calldata a, bytes calldata b, uint[] calldata c) external {}
}
// ----
//     :test
// [
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "name": "a",
//         "type": "string"
//       },
//       {
//         "name": "b",
//         "type": "bytes"
//       },
//       {
//         "name": "c",
//         "type": "uint256[]"
//       }
//     ],
//     "name": "f",
//     "outputs": [],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
