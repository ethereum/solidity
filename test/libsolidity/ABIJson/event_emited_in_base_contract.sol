library   L { event E(uint8); }

contract B {
    constructor() {
        emit L.E(0);
    }
}

contract C is B {}

// ----
//     :B
// [
//   {
//     "inputs": [],
//     "stateMutability": "nonpayable",
//     "type": "constructor"
//   },
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "uint8",
//         "name": "",
//         "type": "uint8"
//       }
//     ],
//     "name": "E",
//     "type": "event"
//   }
// ]
//
//
//     :C
// [
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "uint8",
//         "name": "",
//         "type": "uint8"
//       }
//     ],
//     "name": "E",
//     "type": "event"
//   }
// ]
//
//
//     :L
// [
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "uint8",
//         "name": "",
//         "type": "uint8"
//       }
//     ],
//     "name": "E",
//     "type": "event"
//   }
// ]
