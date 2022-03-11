interface I {
    function f(address payable) external;
}

contract C {
    function main() external view {
        abi.encodeCall(I.f, (address(0)));
    }
}
// ----
// TypeError 5407: (136-148): Cannot implicitly convert component at position 0 from "address" to "address payable".
