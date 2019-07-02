pragma experimental ABIEncoderV2;
contract C {
    struct S { uint a; T[] sub; bytes b; }
    struct T { uint[2] x; }
    event E(T t, S s);
}
// ----
//     :C
// [
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "components":
//         [
//           {
//             "internalType": "uint256[2]",
//             "name": "x",
//             "type": "uint256[2]"
//           }
//         ],
//         "indexed": false,
//         "internalType": "struct C.T",
//         "name": "t",
//         "type": "tuple"
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
//           },
//           {
//             "internalType": "bytes",
//             "name": "b",
//             "type": "bytes"
//           }
//         ],
//         "indexed": false,
//         "internalType": "struct C.S",
//         "name": "s",
//         "type": "tuple"
//       }
//     ],
//     "name": "E",
//     "type": "event"
//   }
// ]
