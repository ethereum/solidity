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
//             "name": "x",
//             "type": "uint256[2]"
//           }
//         ],
//         "indexed": false,
//         "name": "t",
//         "type": "tuple"
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
//           },
//           {
//             "name": "b",
//             "type": "bytes"
//           }
//         ],
//         "indexed": false,
//         "name": "s",
//         "type": "tuple"
//       }
//     ],
//     "name": "E",
//     "type": "event"
//   }
// ]
