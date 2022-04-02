contract C {
    function f() public pure {
        address payable[] memory a = new address payable[](4);
        address[] memory b = new address[](4);
        a = b;
        b = a;
    }
}
// ----
// TypeError 7407: (166-167='b'): Type address[] memory is not implicitly convertible to expected type address payable[] memory.
// TypeError 7407: (181-182='a'): Type address payable[] memory is not implicitly convertible to expected type address[] memory.
