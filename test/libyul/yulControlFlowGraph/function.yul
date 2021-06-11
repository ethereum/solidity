{
    function f(a, b) -> r {
        let x := add(a,b)
        r := sub(x,a)
    }
    function g() {
        sstore(0x01, 0x0101)
    }
    function h(x) {
        h(f(x, 0))
        g()
    }
    function i() -> v, w {
        v := 0x0202
        w := 0x0303
    }
    let x, y := i()
    h(x)
    h(y)
}
// ----
// Block 0:
//   Entries: None
//   i: [ RET[i] ] => [ TMP[i, 0] TMP[i, 1] ]
//   Assignment(x, y): [ TMP[i, 0] TMP[i, 1] ] => [ x y ]
//   h: [ RET[h] x ] => [ ]
//   h: [ RET[h] y ] => [ ]
//   MainExit
// function f(a, b) -> r:
//   Block 0:
//     Entries: None
//     add: [ b a ] => [ TMP[add, 0] ]
//     Assignment(x): [ TMP[add, 0] ] => [ x ]
//     sub: [ a x ] => [ TMP[sub, 0] ]
//     Assignment(r): [ TMP[sub, 0] ] => [ r ]
//     FunctionReturn of f
// function g():
//   Block 0:
//     Entries: None
//     sstore: [ 0x0101 0x01 ] => [ ]
//     FunctionReturn of g
// function h(x):
//   Block 0:
//     Entries: None
//     f: [ RET[f] 0x00 x ] => [ TMP[f, 0] ]
//     h: [ RET[h] TMP[f, 0] ] => [ ]
//     g: [ RET[g] ] => [ ]
//     FunctionReturn of h
// function i() -> v, w:
//   Block 0:
//     Entries: None
//     Assignment(v): [ 0x0202 ] => [ v ]
//     Assignment(w): [ 0x0303 ] => [ w ]
//     FunctionReturn of i
