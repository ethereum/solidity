contract A{
    function f() public pure{

    }
}
contract B{
    A public a;
}
contract C{
    B public b;
}
contract D{
    C c;
    function f() public view{
        c.b.a.f();
    }
}
// ----
// TypeError: (170-175): Member "a" not found or not visible after argument-dependent lookup in function () view external returns (contract B). Did you intend to call the function?
