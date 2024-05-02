struct S {
    function () f;
}

contract C {
    enum testEnum { choice1, choice2, choice3 }

    error f1(uint8);
    error f2(uint32);
    error f3(uint);
    error g1(bytes);
    error g2(bytes32);
    error h(string);
    error i(bool);
    error j(address);
    error k(address payable);
    error l(testEnum);

    function main() external pure {
        S memory s;
        abi.encodeError(f1, (s));
        abi.encodeError(f2, (s));
        abi.encodeError(f3, (s));
        abi.encodeError(g1, (s));
        abi.encodeError(g2, (s));
        abi.encodeError(h, (s));
        abi.encodeError(i, (s));
        abi.encodeError(j, (s));
        abi.encodeError(k, (s));
        abi.encodeError(l, (s));
    }
}
// ----
// TypeError 5407: (402-405): Cannot implicitly convert component at position 0 from "struct S memory" to "uint8".
// TypeError 5407: (436-439): Cannot implicitly convert component at position 0 from "struct S memory" to "uint32".
// TypeError 5407: (470-473): Cannot implicitly convert component at position 0 from "struct S memory" to "uint256".
// TypeError 5407: (504-507): Cannot implicitly convert component at position 0 from "struct S memory" to "bytes memory".
// TypeError 5407: (538-541): Cannot implicitly convert component at position 0 from "struct S memory" to "bytes32".
// TypeError 5407: (571-574): Cannot implicitly convert component at position 0 from "struct S memory" to "string memory".
// TypeError 5407: (604-607): Cannot implicitly convert component at position 0 from "struct S memory" to "bool".
// TypeError 5407: (637-640): Cannot implicitly convert component at position 0 from "struct S memory" to "address".
// TypeError 5407: (670-673): Cannot implicitly convert component at position 0 from "struct S memory" to "address payable".
// TypeError 5407: (703-706): Cannot implicitly convert component at position 0 from "struct S memory" to "enum C.testEnum".
