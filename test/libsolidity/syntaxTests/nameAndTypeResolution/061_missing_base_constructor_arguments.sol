contract A { constructor(uint a) { } }
contract B is A { }
// ----
// TypeError 3415: (39-58): No arguments passed to the base constructor. Specify the arguments or mark "B" as abstract.
