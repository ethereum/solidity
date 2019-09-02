pragma experimental ABIEncoderV2;
struct S { uint a; }
contract C {
    function f(S calldata s) external view {}
    function g(S memory s) public view {}
}
// ----
//     :C
// [
//   {
//     "constant": true,
//     "inputs":
//     [
//       {
//         "components":
//         [
//           {
//             "internalType": "uint256",
//             "name": "a",
//             "type": "uint256"
//           }
//         ],
//         "internalType": "struct S",
//         "name": "s",
//         "type": "tuple"
//       }
//     ],
//     "name": "g",
//     "outputs": [],
//     "payable": false,
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "constant": true,
//     "inputs":
//     [
//       {
//         "components":
//         [
//           {
//             "internalType": "uint256",
//             "name": "a",
//             "type": "uint256"
//           }
//         ],
//         "internalType": "struct S",
//         "name": "s",
//         "type": "tuple"
//       }
//     ],
//     "name": "f",
//     "outputs": [],
//     "payable": false,
//     "stateMutability": "view",
//     "type": "function"
//   }
// ]
