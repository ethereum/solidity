event E();

contract C {
    event E();
}

library L {
    event E();
}

interface I {
    event E();
}
// ----
// Warning 2519: (29-39): This declaration shadows an existing declaration.
// Warning 2519: (59-69): This declaration shadows an existing declaration.
// Warning 2519: (91-101): This declaration shadows an existing declaration.
