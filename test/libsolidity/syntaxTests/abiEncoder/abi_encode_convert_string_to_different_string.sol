interface I {
    function f(string calldata) external;
}

contract C {
    string s;
    function main() external view {
        abi.encodeCall(I.f, (s));
    }
}
// ----
// TypeError 5407: (150-153): Cannot implicitly convert component at position 0 from "string storage ref" to "string calldata".
