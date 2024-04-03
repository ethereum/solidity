struct S {
    function () f;
}

contract C {
    enum testEnum { choice1, choice2, choice3 }

    function f1(uint8) external {}
    function f2(uint32) external {}
    function f3(uint) external {}
    function g1(bytes memory) external {}
    function g2(bytes32) external {}
    function h(string memory) external {}
    function i(bool) external {}
    function j(address) external {}
    function k(address payable) external {}
    function l(testEnum) external {}

    function main() external view {
        S memory s;
        abi.encodeCall(this.f1, (s));
        abi.encodeCall(this.f2, (s));
        abi.encodeCall(this.f3, (s));
        abi.encodeCall(this.g1, (s));
        abi.encodeCall(this.g2, (s));
        abi.encodeCall(this.h, (s));
        abi.encodeCall(this.i, (s));
        abi.encodeCall(this.j, (s));
        abi.encodeCall(this.k, (s));
        abi.encodeCall(this.l, (s));
    }
}
// ----
// TypeError 5407: (560-563): Cannot implicitly convert component at position 0 from "struct S memory" to "uint8".
// TypeError 5407: (598-601): Cannot implicitly convert component at position 0 from "struct S memory" to "uint32".
// TypeError 5407: (636-639): Cannot implicitly convert component at position 0 from "struct S memory" to "uint256".
// TypeError 5407: (674-677): Cannot implicitly convert component at position 0 from "struct S memory" to "bytes memory".
// TypeError 5407: (712-715): Cannot implicitly convert component at position 0 from "struct S memory" to "bytes32".
// TypeError 5407: (749-752): Cannot implicitly convert component at position 0 from "struct S memory" to "string memory".
// TypeError 5407: (786-789): Cannot implicitly convert component at position 0 from "struct S memory" to "bool".
// TypeError 5407: (823-826): Cannot implicitly convert component at position 0 from "struct S memory" to "address".
// TypeError 5407: (860-863): Cannot implicitly convert component at position 0 from "struct S memory" to "address payable".
// TypeError 5407: (897-900): Cannot implicitly convert component at position 0 from "struct S memory" to "enum C.testEnum".
