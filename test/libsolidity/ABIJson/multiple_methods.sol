contract test {
    function f(uint a) public returns (uint d) { return a * 7; }
    function g(uint b) public returns (uint e) { return b * 8; }
}
// ----
//     :test
// [
//   {
//     "inputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "a",
//         "type": "uint256"
//       }
//     ],
//     "name": "f",
//     "outputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "d",
//         "type": "uint256"
//       }
//     ],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "b",
//         "type": "uint256"
//       }
//     ],
//     "name": "g",
//     "outputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "e",
//         "type": "uint256"
//       }
//     ],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
