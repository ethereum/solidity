contract A {
    uint x;
}

contract B is A {
    uint32 y;
}

contract C is B {
    uint160 w;
}

contract D is A, B, C layout at 2 {
    uint z;
    function xSlotOffset() public view returns (uint s, uint o) { assembly { s := x.slot o := x.offset } }
    function ySlotOffset() public view returns (uint s, uint o) { assembly { s := y.slot o := y.offset } }
    function wSlotOffset() public view returns (uint s, uint o) { assembly { s := w.slot o := w.offset } }
    function zSlotOffset() public view returns (uint s, uint o) { assembly { s := z.slot o := z.offset } }
}
// ====
// targetContract: D
// ----
// xSlotOffset() -> 2, 0
// ySlotOffset() -> 3, 0
// wSlotOffset() -> 3, 4
// zSlotOffset() -> 4, 0
