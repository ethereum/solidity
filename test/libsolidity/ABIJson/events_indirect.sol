library L {
    event e1(uint b);
    event e2();
    event e2(uint a);
    event e3() anonymous;
}
contract test {
    function f() public {
        emit L.e1(1);
        emit L.e3();
    }
}
// ----
//     :L
// [
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "uint256",
//         "name": "b",
//         "type": "uint256"
//       }
//     ],
//     "name": "e1",
//     "type": "event"
//   },
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "e2",
//     "type": "event"
//   },
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "uint256",
//         "name": "a",
//         "type": "uint256"
//       }
//     ],
//     "name": "e2",
//     "type": "event"
//   },
//   {
//     "anonymous": true,
//     "inputs": [],
//     "name": "e3",
//     "type": "event"
//   }
// ]
//
//
//     :test
// [
//   {
//     "anonymous": false,
//     "inputs":
//     [
//       {
//         "indexed": false,
//         "internalType": "uint256",
//         "name": "b",
//         "type": "uint256"
//       }
//     ],
//     "name": "e1",
//     "type": "event"
//   },
//   {
//     "anonymous": true,
//     "inputs": [],
//     "name": "e3",
//     "type": "event"
//   },
//   {
//     "inputs": [],
//     "name": "f",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
