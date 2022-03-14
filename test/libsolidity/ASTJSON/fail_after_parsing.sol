function g() public;

interface I {
    struct S { S s; }

    function f(E storage e) {
        error E;
        emit E();

        ++c;
        uint calldata c = 123.4;
    }
}

// ----
// failAfter: Parsed
