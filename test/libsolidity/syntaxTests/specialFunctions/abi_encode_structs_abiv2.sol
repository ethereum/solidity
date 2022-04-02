pragma abicoder               v2;

contract C {
    struct S { uint x; }
    S s;
    struct T { uint y; }
    T t;
    function e() public view {
        S memory st;
        abi.encodePacked(st);
    }
    function f() public view {
        abi.encode(s, t);
    }
    function g() public view {
        abi.encodePacked(s, t);
    }
}
// ----
// TypeError 9578: (193-195='st'): Type not supported in packed mode.
// TypeError 9578: (323-324='s'): Type not supported in packed mode.
// TypeError 9578: (326-327='t'): Type not supported in packed mode.
