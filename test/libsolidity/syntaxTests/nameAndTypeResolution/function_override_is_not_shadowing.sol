contract D { function f() pure public {} }
contract C is D {
    function f(uint) pure public {}
}
