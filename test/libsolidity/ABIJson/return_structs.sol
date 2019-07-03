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
//     "constant": false,
//     "inputs": [],
//     "name": "f",
//     "outputs":
//     [
//       {
//         "name": "x",
//         "type": "uint256"
//       },
//       {
//         "components":
//         [
//           {
//             "name": "a",
//             "type": "uint256"
//           },
//           {
//             "components":
//             [
//               {
//                 "name": "x",
//                 "type": "uint256[2]"
//               }
//             ],
//             "name": "sub",
//             "type": "tuple[]"
//           }
//         ],
//         "name": "s",
//         "type": "tuple"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
