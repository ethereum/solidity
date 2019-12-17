pragma experimental ABIEncoderV2;
contract C {
    struct S { uint a; T[] sub; }
    struct T { uint[2] x; }
    function f() public returns (uint x, S memory s) {
    }
}
// ----
//     :C
// [
//   {
//     "inputs": [],
//     "name": "f",
//     "outputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "x",
//         "type": "uint256"
//       },
//       {
//         "components":
//         [
//           {
//             "internalType": "uint256",
//             "name": "a",
//             "type": "uint256"
//           },
//           {
//             "components":
//             [
//               {
//                 "internalType": "uint256[2]",
//                 "name": "x",
//                 "type": "uint256[2]"
//               }
//             ],
//             "internalType": "struct C.T[]",
//             "name": "sub",
//             "type": "tuple[]"
//           }
//         ],
//         "internalType": "struct C.S",
//         "name": "s",
//         "type": "tuple"
//       }
//     ],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
