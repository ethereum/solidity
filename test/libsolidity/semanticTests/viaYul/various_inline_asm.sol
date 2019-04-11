contract C {
    function f() public pure returns (uint32 x) {
        uint32 a;
        uint32 b;
        uint32 c;
        assembly {
            function myAwesomeFunction(param) -> returnMe {
                let localVar := 10
                returnMe := add(localVar, param)
            }
            let abc := sub(10, a)
            let xyz := 20
            a := abc
            b := myAwesomeFunction(30)
            c := xyz
        }
        x = a + b + c;
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 70
