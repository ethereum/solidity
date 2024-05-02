contract C {
    enum testEnum { choice1, choice2, choice3 }

    error e0(uint8, uint8);
    error e1(uint32);
    error e2(uint);
    error e3(bytes);
    error e4(bytes32);
    error e5(string);
    error e6(bool);
    error e7(address);
    error e8(address payable);
    error e9(testEnum);

    function f() pure public {
        abi.encodeError(e0, (this.f, this.f));
        abi.encodeError(e1, (this.f));
        abi.encodeError(e2, (this.f));
        abi.encodeError(e3, (this.f));
        abi.encodeError(e4, (this.f));
        abi.encodeError(e5, (this.f));
        abi.encodeError(e6, (this.f));
        abi.encodeError(e7, (this.f));
        abi.encodeError(e8, (this.f));
        abi.encodeError(e9, (this.f));
    }
}
// ----
// TypeError 5407: (357-363): Cannot implicitly convert component at position 0 from "function () pure external" to "uint8".
// TypeError 5407: (365-371): Cannot implicitly convert component at position 1 from "function () pure external" to "uint8".
// TypeError 5407: (403-411): Cannot implicitly convert component at position 0 from "function () pure external" to "uint32".
// TypeError 5407: (442-450): Cannot implicitly convert component at position 0 from "function () pure external" to "uint256".
// TypeError 5407: (481-489): Cannot implicitly convert component at position 0 from "function () pure external" to "bytes memory".
// TypeError 5407: (520-528): Cannot implicitly convert component at position 0 from "function () pure external" to "bytes32".
// TypeError 5407: (559-567): Cannot implicitly convert component at position 0 from "function () pure external" to "string memory".
// TypeError 5407: (598-606): Cannot implicitly convert component at position 0 from "function () pure external" to "bool".
// TypeError 5407: (637-645): Cannot implicitly convert component at position 0 from "function () pure external" to "address".
// TypeError 5407: (676-684): Cannot implicitly convert component at position 0 from "function () pure external" to "address payable".
// TypeError 5407: (715-723): Cannot implicitly convert component at position 0 from "function () pure external" to "enum C.testEnum".
