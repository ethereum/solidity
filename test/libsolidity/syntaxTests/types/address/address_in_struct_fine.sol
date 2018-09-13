contract A {
    struct S {
        address a;
    }
    S s;
    function f() public {
        s.a = address(this);
    }
}
contract B {
    struct S {
        address payable a;
    }
    S s;
    function f() public {
        s.a = address(this);
    }
    function() external payable {
    }
}