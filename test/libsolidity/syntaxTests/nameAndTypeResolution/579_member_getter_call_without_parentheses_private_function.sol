contract A{
    function f() private pure{

    }
}
contract B{
    A public a;
}
contract C{
    B b;
    function f() public view{
        b.a.f();
    }
}

// ----
// TypeError: (141-146): Member "f" not found or not visible after argument-dependent lookup in function () view external returns (contract A). Did you intend to call the function?
