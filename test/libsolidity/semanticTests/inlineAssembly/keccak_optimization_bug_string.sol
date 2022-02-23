contract C {
    function f(string memory s) public returns (bool ret) {
        assembly {
            let a := keccak256(s, 32)
            let b := keccak256(s, 8)
            ret := eq(a, b)
        }
    }
}
// ====
// compileViaYul: also
// ----
// f(string): "" -> false
// f(string): 0x20, 5, "hello" -> false
// f(string): 0x20, 0x2e, 29457663690442756349866640336617293820574110049925353194191585327958485180523, 45859201465615193776739262511799714667061496775486067316261261194408342061056 -> false
