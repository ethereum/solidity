contract A {
    uint x;
    uint128 y;
}

contract C is A layout at 7 {
    uint32 w;
    uint z;

    function xSlotOffset() public returns(uint s, uint o) { assembly { s := x.slot o := x.offset } }
    function ySlotOffset() public returns(uint s, uint o) { assembly { s := y.slot o := y.offset } }
    function wSlotOffset() public returns(uint s, uint o) { assembly { s := w.slot o := w.offset } }
    function zSlotOffset() public returns(uint s, uint o) { assembly { s := z.slot o := z.offset } }
}
// ====
// targetContract: C
// ----
// xSlotOffset() -> 7, 0
// ySlotOffset() -> 8, 0
// wSlotOffset() -> 8, 16
// zSlotOffset() -> 9, 0
