contract A { constructor(uint a) public { } }
contract B is A { }
// ----
// TypeError 3656: (46-65): Contract "B" should be marked as abstract.
