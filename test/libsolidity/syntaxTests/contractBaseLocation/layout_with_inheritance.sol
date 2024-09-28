contract A { }
contract B { }
contract C is A, B layout at 0x1234 { }
contract D layout at 0xABCD is A, B { }
// ----
