type MyInt is int;
type MyByte1 is bytes1;
contract C {
    type MyAddress is address;
    type MyUInt8 is uint8;
    type MyBytes32 is bytes32;

    MyInt public myInt;
    MyByte1 public myByte1;
    MyAddress public myAddress;
    MyUInt8 public myUInt8;
    MyBytes32 public myBytes32;

    function setMyInt(MyInt a) external {
        myInt = a;
    }
    function setMyByte1(MyByte1 a) external {
        myByte1 = a;
    }
    function setMyAddress(MyAddress a) external {
        myAddress = a;
    }
    function setMyUInt8(MyUInt8 a) external {
        myUInt8 = a;
    }
    function setMyBytes32(MyBytes32 a) external {
        myBytes32 = a;
    }
}
// ----
//     :C
// [
//   {
//     "inputs": [],
//     "name": "myAddress",
//     "outputs":
//     [
//       {
//         "internalType": "C.MyAddress",
//         "name": "",
//         "type": "address"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "inputs": [],
//     "name": "myByte1",
//     "outputs":
//     [
//       {
//         "internalType": "MyByte1",
//         "name": "",
//         "type": "bytes1"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "inputs": [],
//     "name": "myBytes32",
//     "outputs":
//     [
//       {
//         "internalType": "C.MyBytes32",
//         "name": "",
//         "type": "bytes32"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "inputs": [],
//     "name": "myInt",
//     "outputs":
//     [
//       {
//         "internalType": "MyInt",
//         "name": "",
//         "type": "int256"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "inputs": [],
//     "name": "myUInt8",
//     "outputs":
//     [
//       {
//         "internalType": "C.MyUInt8",
//         "name": "",
//         "type": "uint8"
//       }
//     ],
//     "stateMutability": "view",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "C.MyAddress",
//         "name": "a",
//         "type": "address"
//       }
//     ],
//     "name": "setMyAddress",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "MyByte1",
//         "name": "a",
//         "type": "bytes1"
//       }
//     ],
//     "name": "setMyByte1",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "C.MyBytes32",
//         "name": "a",
//         "type": "bytes32"
//       }
//     ],
//     "name": "setMyBytes32",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "MyInt",
//         "name": "a",
//         "type": "int256"
//       }
//     ],
//     "name": "setMyInt",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "C.MyUInt8",
//         "name": "a",
//         "type": "uint8"
//       }
//     ],
//     "name": "setMyUInt8",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
