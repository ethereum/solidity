interface B {
    event EB();
}

contract C is B {
    event EC();
}

contract D is C {
    event ED();
}

// ----
//     :B
// [
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "EB",
//     "type": "event"
//   }
// ]
//
//
//     :C
// [
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "EB",
//     "type": "event"
//   },
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "EC",
//     "type": "event"
//   }
// ]
//
//
//     :D
// [
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "EB",
//     "type": "event"
//   },
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "EC",
//     "type": "event"
//   },
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "ED",
//     "type": "event"
//   }
// ]
