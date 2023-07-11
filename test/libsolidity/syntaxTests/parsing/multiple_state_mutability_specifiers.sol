==== Source: A ====
contract c1 {
    function f() payable payable {}
}
contract c2 {
    function f() view view {}
}
contract c3 {
    function f() pure pure {}
}
contract c4 {
    function f() pure view {}
}
contract c5 {
    function f() payable view {}
}
contract c6 {
    function f() pure payable {}
}
contract c7 {
    function f() view payable {}
}
contract c8 {
    function f() payable pure {}
}
contract c9 {
    function f() view pure {}
}
==== Source: B ====
contract c {
    address payable payable v;
}
==== Source: C ====
contract c {
    function fn(address payable payable) public { }
}
==== Source: D ====
contract c {
    function fn() public returns (address payable payable) { }
}
==== Source: E ====
contract c {
    function fn() public {
        address payable payable v;
    }
}
==== Source: F ====
contract c {
    function fn(address payable payable v) public { }
}
==== Source: G ====
contract c {
    function fn() public returns (address payable payable res) { }
}
==== Source: H ====
address payable payable v;
==== Source: I ====
contract c {
    modifier m(address payable payable x)  { _; }
}
==== Source: J ====
contract c {
    event e(address payable payable x);
}
==== Source: K ====
contract c {
    error e1(address payable payable x);
}
==== Source: L ====
contract c {
    function f() public returns (address payable) {
        try this.f() returns (address payable payable) { } catch { }
    }
}
==== Source: M ====
contract c {
    function f() public returns (address payable) {
        try this.f() returns (address payable) { } catch Error(address payable payable) { }
    }
}
// ----
// ParserError 9680: (A:39-46): State mutability already specified as "payable".
// ParserError 9680: (A:88-92): State mutability already specified as "view".
// ParserError 9680: (A:134-138): State mutability already specified as "pure".
// ParserError 9680: (A:180-184): State mutability already specified as "pure".
// ParserError 9680: (A:229-233): State mutability already specified as "payable".
// ParserError 9680: (A:275-282): State mutability already specified as "pure".
// ParserError 9680: (A:324-331): State mutability already specified as "view".
// ParserError 9680: (A:376-380): State mutability already specified as "payable".
// ParserError 9680: (A:422-426): State mutability already specified as "view".
// ParserError 2314: (B:33-40): Expected identifier but got 'payable'
// ParserError 2314: (C:45-52): Expected ',' but got 'payable'
// ParserError 2314: (D:63-70): Expected ',' but got 'payable'
// ParserError 2314: (E:64-71): Expected identifier but got 'payable'
// ParserError 2314: (F:45-52): Expected ',' but got 'payable'
// ParserError 2314: (G:63-70): Expected ',' but got 'payable'
// ParserError 2314: (H:16-23): Expected identifier but got 'payable'
// ParserError 2314: (I:44-51): Expected ',' but got 'payable'
// ParserError 2314: (J:41-48): Expected ',' but got 'payable'
// ParserError 2314: (K:42-49): Expected ',' but got 'payable'
// ParserError 2314: (L:111-118): Expected ',' but got 'payable'
// ParserError 2314: (M:144-151): Expected ',' but got 'payable'

