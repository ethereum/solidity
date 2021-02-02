error E1();
function f() {
    revert E1();
}
contract C {
    // unreferenced but inherited
    error E2();
}
contract D {
    // referenced
    error E3(uint);
}
contract X is C {
    // unreferenced but defined
    error E3();
    function g(uint x) public {
        if (x == 0)
            f();
        else if (x == 2)
            revert D.E3(1);
    }
}
// ----
//     :C
// [
//   {
//     "inputs": [],
//     "name": "E2",
//     "type": "error"
//   }
// ]
//
//
//     :D
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
//     "name": "E3",
//     "type": "error"
//   }
// ]
//
//
//     :X
// [
//   {
//     "inputs": [],
//     "name": "E1",
//     "type": "error"
//   },
//   {
//     "inputs": [],
//     "name": "E2",
//     "type": "error"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "",
//         "type": "uint256"
//       }
//     ],
//     "name": "E3",
//     "type": "error"
//   },
//   {
//     "inputs": [],
//     "name": "E3",
//     "type": "error"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "x",
//         "type": "uint256"
//       }
//     ],
//     "name": "g",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
