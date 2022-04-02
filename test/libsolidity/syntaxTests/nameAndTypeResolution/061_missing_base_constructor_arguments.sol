contract A { constructor(uint a) { } }
contract B is A { }
// ----
// TypeError 3656: (39-58='contract B is A { }'): Contract "B" should be marked as abstract.
