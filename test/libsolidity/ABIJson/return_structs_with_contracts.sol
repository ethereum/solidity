pragma experimental ABIEncoderV2;
contract C {
    struct S { C[] x; C y; }
    function f() public returns (S memory s, C c) {
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
//         "components":
//         [
//           {
//             "name": "x",
//             "type": "address[]"
//           },
//           {
//             "name": "y",
//             "type": "address"
//           }
//         ],
//         "name": "s",
//         "type": "tuple"
//       },
//       {
//         "name": "c",
//         "type": "address"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
