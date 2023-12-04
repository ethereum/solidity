// a<------b
// |       ^
// |       |
// |       |
// |       |
// |       |
// +------>c------->d-------->e------->f
//                            ^        |
//                            |        |
//                            |        |
//                            |        |
//                            |        v
//                            h<-------g

pragma experimental solidity;

function a()
{
    c();
}

function b()
{
    a();
}

function c()
{
    b();
    d();
}

function d()
{
    e();
}

function e()
{
    f();
}

function f()
{
    g();
}

function g()
{
    h();
}

function h()
{
    e();
}

// ----
// (a) --> {c,}
// (b) --> {a,}
// (c) --> {b,d,}
// (d) --> {e,}
// (e) --> {f,}
// (f) --> {g,}
// (g) --> {h,}
// (h) --> {e,}
