contract C {
    type MyInt is int;
    MyInt constant mi = MyInt.wrap(5);
    // This is currently unsupported.
    uint[MyInt.unwrap(mi)] arr;
}
// ----
// TypeError 5462: (122-138): Invalid array length, expected integer literal or constant expression.
