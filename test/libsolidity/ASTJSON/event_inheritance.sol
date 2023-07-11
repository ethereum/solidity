interface B {
    event EB();
}
contract C is B {
    event EC();
}
contract D is C {
    event ED();
}

// ----
