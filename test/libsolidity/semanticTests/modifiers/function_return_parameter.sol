// The IR of this contract used to throw
contract B {
    function f(uint8 a) mod1(a, true) mod2(r) pure public returns (bytes7 r) { }
    modifier mod1(uint a, bool b) { if (b) _; }
    modifier mod2(bytes7 a) { while (a == "1234567") _; }
}
// ----
// f(uint8): 5 -> 0x00
