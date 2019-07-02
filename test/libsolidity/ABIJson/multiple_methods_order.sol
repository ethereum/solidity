// methods are expected to be in alphabetical order
contract test {
    function f(uint a) public returns (uint d) { return a * 7; }
    function c(uint b) public returns (uint e) { return b * 8; }
}
// ----
//     :test
// [
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "name": "b",
//         "type": "uint256"
//       }
//     ],
//     "name": "c",
//     "outputs":
//     [
//       {
//         "name": "e",
//         "type": "uint256"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "name": "a",
//         "type": "uint256"
//       }
//     ],
//     "name": "f",
//     "outputs":
//     [
//       {
//         "name": "d",
//         "type": "uint256"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
