library L {
    error E(bytes4, bool, bytes);
}

error E(bytes4, bool, bytes);

interface I {
    error E(bytes4, bool, bytes);
}

contract B {
    error E(bytes4, bool, bytes);
}

contract C is B {
    bytes4 public librarySelector = L.E.selector;
    bytes4 internal freeSelector = E.selector;
    bytes4 internal contractSelector = B.E.selector;
    bytes4 private interfaceSelector = I.E.selector;

    function f(bool condition) public view {
        assert(librarySelector == L.E.selector);
        assert(E.selector == B.E.selector);

        if (condition)
            revert E(E.selector, true, "123");
        else
            revert L.E((B.E.selector), true, "123");
    }
}
// ----
// Warning 2519: (16-45): This declaration shadows an existing declaration.
// Warning 2519: (98-127): This declaration shadows an existing declaration.
// Warning 2519: (148-177): This declaration shadows an existing declaration.
