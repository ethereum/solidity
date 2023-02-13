contract A { constructor(uint a) { } }
contract B is A { constructor(bytes4 a, bytes28 b) { } }
// ----
// TypeError 3415: (39-95): No arguments passed to the base constructor. Specify the arguments or mark "B" as abstract.
