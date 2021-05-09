contract A { constructor(uint) {} }
contract C is A { constructor() A(2) {} }

// ----
