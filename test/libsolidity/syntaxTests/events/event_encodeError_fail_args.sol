contract C {
    event MyCustomEvent(uint);

    enum testEnum { choice1, choice2, choice3 }

    error f1(uint8, uint8);
    error f2(uint32);
    error f3(uint);
    error g1(bytes);
    error g2(bytes32);
    error h(string);
    error i(bool);
    error j(address);
    error k(address payable);
    error l(testEnum);

    function f() pure public {
        abi.encodeError(f1, (MyCustomEvent, MyCustomEvent));
        abi.encodeError(f2, (MyCustomEvent));
        abi.encodeError(f3, (MyCustomEvent));
        abi.encodeError(g1, (MyCustomEvent));
        abi.encodeError(g2, (MyCustomEvent));
        abi.encodeError(h, (MyCustomEvent));
        abi.encodeError(i, (MyCustomEvent));
        abi.encodeError(j, (MyCustomEvent));
        abi.encodeError(k, (MyCustomEvent));
        abi.encodeError(l, (MyCustomEvent));
    }
}
// ----
// TypeError 5408: (384-397): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "uint8".
// TypeError 5408: (399-412): Cannot implicitly convert component at position 1 from "event MyCustomEvent(uint256)" to "uint8".
// TypeError 5408: (444-459): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "uint32".
// TypeError 5408: (490-505): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "uint256".
// TypeError 5408: (536-551): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "bytes memory".
// TypeError 5408: (582-597): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "bytes32".
// TypeError 5408: (627-642): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "string memory".
// TypeError 5408: (672-687): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "bool".
// TypeError 5408: (717-732): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "address".
// TypeError 5408: (762-777): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "address payable".
// TypeError 5408: (807-822): Cannot implicitly convert component at position 0 from "event MyCustomEvent(uint256)" to "enum C.testEnum".
