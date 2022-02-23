contract A { constructor(uint a) { } }
contract B is A { }
// ----
// TypeError 3656: (39-58): Contract "B" should be marked as abstract.
