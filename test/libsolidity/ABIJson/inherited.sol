contract Base {
    function baseFunction(uint p) public returns (uint i) { return p; }
    event baseEvent(bytes32 indexed evtArgBase);
}
contract Derived is Base {
    function derivedFunction(bytes32 p) public returns (bytes32 i) { return p; }
    event derivedEvent(uint indexed evtArgDerived);
}
// ----
//     :Base
// [
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "name": "p",
//         "type": "uint256"
//       }
//     ],
//     "name": "baseFunction",
//     "outputs":
//     [
//       {
//         "name": "i",
//         "type": "uint256"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": true,
//         "name": "evtArgBase",
//         "type": "bytes32"
//       }
//     ],
//     "name": "baseEvent",
//     "type": "event"
//   }
// ]
//
//
//     :Derived
// [
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "name": "p",
//         "type": "uint256"
//       }
//     ],
//     "name": "baseFunction",
//     "outputs":
//     [
//       {
//         "name": "i",
//         "type": "uint256"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "name": "p",
//         "type": "bytes32"
//       }
//     ],
//     "name": "derivedFunction",
//     "outputs":
//     [
//       {
//         "name": "i",
//         "type": "bytes32"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": true,
//         "name": "evtArgDerived",
//         "type": "uint256"
//       }
//     ],
//     "name": "derivedEvent",
//     "type": "event"
//   },
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": true,
//         "name": "evtArgBase",
//         "type": "bytes32"
//       }
//     ],
//     "name": "baseEvent",
//     "type": "event"
//   }
// ]
