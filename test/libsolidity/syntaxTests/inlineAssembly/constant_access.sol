contract C {
    uint constant x = 2**20;
    bool constant b = true;
    bytes4 constant s = "ab";
    function f() public pure {
        assembly {
            let c1 := x
            let c2 := b
            let c3 := s
        }
    }
}
// ----
