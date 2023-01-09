contract test {
    mapping(address owner => mapping(address spender => uint value)) public allowance;
    mapping(bytes32 => address sender) public commits;
    mapping(bytes32 => bytes32) public something;
}
// ----
//     :test
// [
//   {
//     "inputs":
//     [
//       {
//         "internalType": "address",
//         "name": "owner",
//         "type": "address"
//       },
//       {
//         "internalType": "address",
//         "name": "spender",
//         "type": "address"
//       }
//     ],
//     "name": "allowance",
//     "outputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "value",
//         "type": "uint256"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "bytes32",
//         "name": "",
//         "type": "bytes32"
//       }
//     ],
//     "name": "commits",
//     "outputs":
//     [
//       {
//         "internalType": "address",
//         "name": "sender",
//         "type": "address"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "bytes32",
//         "name": "",
//         "type": "bytes32"
//       }
//     ],
//     "name": "something",
//     "outputs":
//     [
//       {
//         "internalType": "bytes32",
//         "name": "",
//         "type": "bytes32"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   }
// ]
