pragma experimental ABIEncoderV2;
library L {
    struct S { uint a; T[] sub; bytes b; }
    struct T { uint[2] x; }
    function f(L.S storage s) public view {}
    function g(L.S memory s) public view {}
}
// ----
//     :L
// [
//   {
//     "constant": true,
//     "inputs":
//     [
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
//         "name": "s",
//         "type": "tuple"
//       }
//     ],
//     "name": "g",
//     "outputs": [],
//     "payable": false,
//     "stateMutability": "view",
//     "type": "function"
//   }
// ]
