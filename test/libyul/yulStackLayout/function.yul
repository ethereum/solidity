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
//   Entry Layout: [ ]
//   [ RET[h] RET[h] RET[i] ] >> i
//   [ RET[h] RET[h] TMP[i, 0] TMP[i, 1] ] >> Assignment(x, y)
//   [ RET[h] y RET[h] x ] >> h
//   [ RET[h] y ] >> h
//   Exit Layout: [ ]
//   MainExit
// function f(a, b) -> r:
//   Block 0:
//     Entries: None
//     Entry Layout: [ RET a b ]
//     [ RET a b a ] >> add
//     [ RET a TMP[add, 0] ] >> Assignment(x)
//     [ RET a x ] >> sub
//     [ RET TMP[sub, 0] ] >> Assignment(r)
//     Exit Layout: [ r RET ]
//     FunctionReturn of f
// function g():
//   Block 0:
//     Entries: None
//     Entry Layout: [ RET ]
//     [ RET 0x0101 0x01 ] >> sstore
//     Exit Layout: [ RET ]
//     FunctionReturn of g
// function h(x):
//   Block 0:
//     Entries: None
//     Entry Layout: [ RET RET[h] RET[f] 0x00 x ]
//     [ RET RET[h] RET[f] 0x00 x ] >> f
//     [ RET RET[h] TMP[f, 0] ] >> h
//     [ RET RET[g] ] >> g
//     Exit Layout: [ RET ]
//     FunctionReturn of h
// function i() -> v, w:
//   Block 0:
//     Entries: None
//     Entry Layout: [ RET ]
//     [ RET 0x0202 ] >> Assignment(v)
//     [ v RET 0x0303 ] >> Assignment(w)
//     Exit Layout: [ v w RET ]
//     FunctionReturn of i
