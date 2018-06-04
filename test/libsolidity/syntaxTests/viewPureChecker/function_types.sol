contract C {
    function f() pure public {
        function () external nonpayFun;
        function () external view viewFun;
        function () external pure pureFun;

        nonpayFun;
        viewFun;
        pureFun;
        pureFun();
    }
    function g() view public {
        function () external view viewFun;

        viewFun();
    }
    function h() public {
        function () external nonpayFun;

        nonpayFun();
    }
}
