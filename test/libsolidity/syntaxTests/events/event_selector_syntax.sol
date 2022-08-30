library L {
    event E(bytes32, bool, bytes indexed);
}

contract B {
    event E(bytes32, bool, bytes indexed);
}

contract C is B {
    bytes32 public librarySelector = L.E.selector;
    bytes32 inheritedSelector = E.selector;

    function f() public {
        assert(librarySelector == L.E.selector);
        assert(E.selector == B.E.selector);

        emit E(E.selector, true, "123");
        emit L.E((B.E.selector), true, "123");
    }
}
