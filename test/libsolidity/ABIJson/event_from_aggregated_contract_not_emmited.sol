contract C {
    event e();
    function f() public {
        emit e();
    }
}

contract D {
    C c;
    constructor() {
        c = new C();
        c.f();
    }
}

// ----
//     :C
// [
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "e",
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
//
//
//     :D
// [
//   {
//     "inputs": [],
//     "stateMutability": "nonpayable",
//     "type": "constructor"
//   }
// ]
