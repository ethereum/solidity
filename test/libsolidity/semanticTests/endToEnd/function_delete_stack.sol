contract C {
    function a() public returns(uint) {
        return 7;
    }

    function test() public returns(uint) {
        function() returns(uint) y = a;
        delete y;
        y();
    }
}

// ====
// compileViaYul: also
// ----
// test() -> 
// test():"" -> ""
