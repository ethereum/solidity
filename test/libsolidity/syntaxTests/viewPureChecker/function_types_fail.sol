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
// TypeError: (92-103): Function declared as pure, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (193-202): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError: (289-300): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
