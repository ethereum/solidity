uint constant X = 42;
contract C layout at 0xffff * (50 - X) { }
// ----
