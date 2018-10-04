contract A{
    function f() public pure{

    }
}
contract B{
    A private a;
}
contract C{
    B b;
    function f() public view{
        b.a.f();
    }
}

// ----
// TypeError: (141-144): Member "a" not found or not visible after argument-dependent lookup in contract B.
