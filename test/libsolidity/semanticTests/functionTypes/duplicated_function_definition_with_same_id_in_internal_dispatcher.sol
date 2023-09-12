contract C {
    function a() internal {}
    function f() public {
        function() ptr1 = a;
        function() ptr2 = a;
    }
}
// ----
// f()
