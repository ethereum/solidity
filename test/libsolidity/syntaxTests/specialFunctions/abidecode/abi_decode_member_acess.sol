library L {
    struct S { int a; }
    enum State { idle, running, blocked }
}

contract D {
    struct X { uint b; }
    enum Color { red, green, blue }
}

contract C {
    function f() pure public {
        abi.decode("", (L.S));
        abi.decode("", (L.State));
        abi.decode("", (D.X));
        abi.decode("", (D.Color));
    }
}
// ----
