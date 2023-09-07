event E();

contract C {
    event E() anonymous;
}

library L {
    event E() anonymous;
}

interface I {
    event E() anonymous;
}
// ----
// Warning 2519: (29-49): This declaration shadows an existing declaration.
// Warning 2519: (69-89): This declaration shadows an existing declaration.
// Warning 2519: (111-131): This declaration shadows an existing declaration.
