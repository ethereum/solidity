interface D {
    function f() view external;
}
contract C is D {
    function f() view external {}
}
