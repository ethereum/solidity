library L1 { event e(); }
library L2 { event e(); }
contract C {
	constructor() {
		emit L1.e();
	}
	function f() public {
		emit L2.e();
	}
}

// ----
//     :C
// [
//   {
//     "inputs": [],
//     "stateMutability": "nonpayable",
//     "type": "constructor"
//   },
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "e",
//     "type": "event"
//   },
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
//     :L1
// [
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "e",
//     "type": "event"
//   }
// ]
//
//
//     :L2
// [
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "e",
//     "type": "event"
//   }
// ]
