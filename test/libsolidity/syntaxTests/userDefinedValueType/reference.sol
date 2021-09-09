library L {
    type MyInt is int;
}

contract C {
    L.MyInt a;
    type MyInt is int8;
}

contract D is C {
    C.MyInt b;
    L.MyInt c;
}
