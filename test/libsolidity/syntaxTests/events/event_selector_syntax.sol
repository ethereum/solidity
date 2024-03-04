library L {
    event E(bytes32, bool, bytes indexed);
}

interface I {
    event E(bytes32, bool, bytes indexed);
}

contract A {
    event E(bytes32, bool, bytes indexed);
}

contract B {
    event E(bytes32, bool, bytes indexed);
}

contract C is B {
    bytes32 public librarySelector = L.E.selector;
    bytes32 public interfaceSelector = I.E.selector;
    bytes32 public foreignContractSelector = A.E.selector;
    bytes32 inheritedSelector = E.selector;

    function f() public {
        assert(librarySelector == L.E.selector);
        assert(interfaceSelector == I.E.selector);
        assert(foreignContractSelector == A.E.selector);
        assert(E.selector == B.E.selector);

        emit E(E.selector, true, "123");
        emit I.E((B.E.selector), true, "123");
        emit A.E((B.E.selector), true, "123");
        emit L.E((B.E.selector), true, "123");
    }
}
// ----
