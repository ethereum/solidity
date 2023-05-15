// The IR of this contract used to throw
contract A { modifier m1{_;} }
contract B is A { constructor() A() m1{} }
// ----
// constructor() ->
