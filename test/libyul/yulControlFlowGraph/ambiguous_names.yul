{
  a()
  c()
  function a() {
    let x := 42
    sstore(x,x)
    b()
    function b() {}
  }
  function c() {
    let x := 21
    mstore(x,x)
    b()
    function b() {}
  }
}
// ----
// digraph CFG {
// nodesep=0.7;
// node[shape=box];
//
// Entry [label="Entry"];
// Entry -> Block0;
// Block0 [label="\
// a: [ RET[a] ] => [ ]\l\
// c: [ RET[c] ] => [ ]\l\
// "];
// Block0Exit [label="MainExit"];
// Block0 -> Block0Exit;
//
// FunctionEntry_a_1 [label="function a()"];
// FunctionEntry_a_1 -> Block1;
// Block1 [label="\
// Assignment(x): [ 0x2a ] => [ x ]\l\
// sstore: [ x x ] => [ ]\l\
// b: [ RET[b] ] => [ ]\l\
// "];
// Block1Exit [label="FunctionReturn[a]"];
// Block1 -> Block1Exit;
//
// FunctionEntry_b_2 [label="function b()"];
// FunctionEntry_b_2 -> Block2;
// Block2 [label="\
// "];
// Block2Exit [label="FunctionReturn[b]"];
// Block2 -> Block2Exit;
//
// FunctionEntry_c_3 [label="function c()"];
// FunctionEntry_c_3 -> Block3;
// Block3 [label="\
// Assignment(x): [ 0x15 ] => [ x ]\l\
// mstore: [ x x ] => [ ]\l\
// b: [ RET[b] ] => [ ]\l\
// "];
// Block3Exit [label="FunctionReturn[c]"];
// Block3 -> Block3Exit;
//
// FunctionEntry_b_4 [label="function b()"];
// FunctionEntry_b_4 -> Block4;
// Block4 [label="\
// "];
// Block4Exit [label="FunctionReturn[b]"];
// Block4 -> Block4Exit;
//
// }
