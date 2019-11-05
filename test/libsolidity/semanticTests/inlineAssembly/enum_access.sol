contract C {
    enum A { X, Y, Z }
    function f() public pure returns (uint a, uint b, uint c) {
        assembly {
            a := A.X
            b := A.Y
            c := A.Z
        }
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0, 1, 2
