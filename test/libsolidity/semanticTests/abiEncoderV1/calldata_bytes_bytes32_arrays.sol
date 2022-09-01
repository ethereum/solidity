pragma abicoder v1;

contract C {
    function f(bool a, bytes calldata b, bytes32[2] calldata c)
        public
        returns (bool, bytes calldata, bytes32[2] calldata)
    {
        return (a, b, c);
    }
}
// ====
// compileViaYul: false
// ----
// f(bool,bytes,bytes32[2]): true, 0x80, "a", "b", 4, "abcd" -> true, 0x80, "a", "b", 4, "abcd"
