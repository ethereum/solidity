event Event();
event Event(uint);

event UnusedEvent();

function f() {
    emit Event();
}

contract C {
    function c_main() public {
        emit Event(42);
        f();
    }
}

contract D is C {
    event Event(string);

    function d_main() public {
        emit Event("abc");
    }
}
// ----
//     :C
// [
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "Event",
//     "type": "event"
//   },
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "uint256",
//         "name": "",
//         "type": "uint256"
//       }
//     ],
//     "name": "Event",
//     "type": "event"
//   },
//   {
//     "inputs": [],
//     "name": "c_main",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
//
//
//     :D
// [
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "Event",
//     "type": "event"
//   },
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "uint256",
//         "name": "",
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
//         "indexed": false,
//         "internalType": "string",
//         "name": "",
//         "type": "string"
//       }
//     ],
//     "name": "Event",
//     "type": "event"
//   },
//   {
//     "inputs": [],
//     "name": "c_main",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "inputs": [],
//     "name": "d_main",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
