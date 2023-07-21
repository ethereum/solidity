interface I {
    event Event(uint256 value);
}
contract C {
    event Event(address indexed sender);
}
contract D {
    event Event(address indexed sender);
    function test(address sender) public {
        emit I.Event(1);
        emit C.Event(msg.sender);
        emit Event(msg.sender);
    }
}
// ----
//     :C
// [
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": true,
//         "internalType": "address",
//         "name": "sender",
//         "type": "address"
//       }
//     ],
//     "name": "Event",
//     "type": "event"
//   }
// ]
//
//
//     :D
// [
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "uint256",
//         "name": "value",
//         "type": "uint256"
//       }
//     ],
//     "name": "Event",
//     "type": "event"
//   },
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": true,
//         "internalType": "address",
//         "name": "sender",
//         "type": "address"
//       }
//     ],
//     "name": "Event",
//     "type": "event"
//   },
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": true,
//         "internalType": "address",
//         "name": "sender",
//         "type": "address"
//       }
//     ],
//     "name": "Event",
//     "type": "event"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "address",
//         "name": "sender",
//         "type": "address"
//       }
//     ],
//     "name": "test",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
//
//
//     :I
// [
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "uint256",
//         "name": "value",
//         "type": "uint256"
//       }
//     ],
//     "name": "Event",
//     "type": "event"
//   }
// ]
