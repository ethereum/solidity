struct S {
    uint x;
    address a;
    bool b;
}

contract A {
    S public s1;
    uint transient t1;
}

contract C is A layout at 42 {
    uint transient t2;
    S[][5] dArray;
    uint[10][2] sArray;
    bytes bArray;
    string str;


    function s1SlotOffset() public returns (uint s, uint o)     { assembly { s:= s1.slot     o:= s1.offset } }
    function dArraySlotOffset() public returns (uint s, uint o) { assembly { s:= dArray.slot o:= dArray.offset } }
    function sArraySlotOffset() public returns (uint s, uint o) { assembly { s:= sArray.slot o:= sArray.offset } }
    function bArraySlotOffset() public returns (uint s, uint o) { assembly { s:= bArray.slot o:= bArray.offset } }
    function strSlotOffset() public returns (uint s, uint o)    { assembly { s:= str.slot    o:= str.offset } }
}
// ====
// EVMVersion: >=cancun
// targetContract: C
// ----
// s1SlotOffset() -> 42, 0
// dArraySlotOffset() -> 44, 0
// sArraySlotOffset() -> 49, 0
// bArraySlotOffset() -> 69, 0
// strSlotOffset() -> 70, 0
