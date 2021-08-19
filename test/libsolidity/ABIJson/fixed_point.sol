struct S { uint a; }
contract C {
    fixed public t;
    ufixed32x4 public x;
    mapping(fixed => ufixed) public y;
    event Ev(fixed x);
    error Er(fixed k);
    function f(fixed[] calldata) external view returns (ufixed64x2) {}
}
// ----
//     :C
// [
//   {
//     "inputs":
//     [
//       {
//         "internalType": "fixed128x18",
//         "name": "k",
//         "type": "fixed128x18"
//       }
//     ],
//     "name": "Er",
//     "type": "error"
//   },
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "fixed128x18",
//         "name": "x",
//         "type": "fixed128x18"
//       }
//     ],
//     "name": "Ev",
//     "type": "event"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "fixed128x18[]",
//         "name": "",
//         "type": "fixed128x18[]"
//       }
//     ],
//     "name": "f",
//     "outputs":
//     [
//       {
//         "internalType": "ufixed64x2",
//         "name": "",
//         "type": "ufixed64x2"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "inputs": [],
//     "name": "t",
//     "outputs":
//     [
//       {
//         "internalType": "fixed128x18",
//         "name": "",
//         "type": "fixed128x18"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "inputs": [],
//     "name": "x",
//     "outputs":
//     [
//       {
//         "internalType": "ufixed32x4",
//         "name": "",
//         "type": "ufixed32x4"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "fixed128x18",
//         "name": "",
//         "type": "fixed128x18"
//       }
//     ],
//     "name": "y",
//     "outputs":
//     [
//       {
//         "internalType": "ufixed128x18",
//         "name": "",
//         "type": "ufixed128x18"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   }
// ]
