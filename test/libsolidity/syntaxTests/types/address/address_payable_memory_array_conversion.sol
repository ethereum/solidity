contract C {
    function f() public pure {
        address payable[] memory a = new address payable[](4);
        address[] memory b = new address[](4);
        a = b;
        b = a;
    }
}
// ----
// TypeError: (166-167): Type address[] memory is not implicitly convertible to expected type address payable[] memory.
// TypeError: (181-182): Type address payable[] memory is not implicitly convertible to expected type address[] memory.
