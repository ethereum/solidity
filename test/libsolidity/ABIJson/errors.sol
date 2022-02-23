error FreeError();
contract X {
    error E(uint);
    error E2(uint a, uint b);
    error E4(uint a, bytes[][] c);
    struct S { uint x; }
    error E5(S t);
    function f() public pure {
        revert FreeError();
    }
}
// ----
//     :X
// [
//   {
//     "inputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "",
//         "type": "uint256"
//       }
//     ],
//     "name": "E",
//     "type": "error"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "a",
//         "type": "uint256"
//       },
//       {
//         "internalType": "uint256",
//         "name": "b",
//         "type": "uint256"
//       }
//     ],
//     "name": "E2",
//     "type": "error"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "a",
//         "type": "uint256"
//       },
//       {
//         "internalType": "bytes[][]",
//         "name": "c",
//         "type": "bytes[][]"
//       }
//     ],
//     "name": "E4",
//     "type": "error"
//   },
//   {
//     "inputs":
//     [
//       {
//         "components":
//         [
//           {
//             "internalType": "uint256",
//             "name": "x",
//             "type": "uint256"
//           }
//         ],
//         "internalType": "struct X.S",
//         "name": "t",
//         "type": "tuple"
//       }
//     ],
//     "name": "E5",
//     "type": "error"
//   },
//   {
//     "inputs": [],
//     "name": "FreeError",
//     "type": "error"
//   },
//   {
//     "inputs": [],
//     "name": "f",
//     "outputs": [],
//     "stateMutability": "pure",
//     "type": "function"
//   }
// ]
