contract test {
    event e() anonymous;
}
// ----
//     :test
// [
//   {
//     "anonymous": true,
//     "inputs": [],
//     "name": "e",
//     "type": "event"
//   }
// ]
