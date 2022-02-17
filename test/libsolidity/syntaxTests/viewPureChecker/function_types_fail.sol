contract C {
    function f() pure public {
        function () external nonpayFun;
        nonpayFun();
    }
    function g() pure public {
        function () external view viewFun;
        viewFun();
    }
    function h() view public {
        function () external nonpayFun;
        nonpayFun();
    }
}
// ----
// TypeError 8961: (92-103): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 2527: (193-202): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (289-300): Function cannot be declared as view because this expression (potentially) modifies the state.
