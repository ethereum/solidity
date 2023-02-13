contract A { constructor(uint a) { } }
contract B is A { constructor(bytes4 a, bytes28 b) { } }
// ----
// TypeError 3656: (39-58): Contract "B" should be marked as abstract.
