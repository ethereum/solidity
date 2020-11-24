pragma abicoder               v2;

contract C {
    enum E { A, B }
    function f(E e) public pure returns (uint x) {
        assembly { x := e }
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint8): 0 -> 0
// f(uint8): 1 -> 1
// f(uint8): 2 -> FAILURE
// f(uint8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> FAILURE
