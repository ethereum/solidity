struct S { uint128 x; }

using {add as +} for S;
using {sub as -} for S;
using {mul as *} for S;
using {div as *} for S;
using {bitor as |} for S;
using {unsub as -} for S;

function add(S memory, S storage) returns (S memory) {}
function sub(S memory, S storage) returns (S storage) {}
function mul(S storage, S memory) returns (S memory) {}
function div(S storage, S memory) returns (S storage) {}
function bitor(S storage, S storage) pure returns (S memory) {}
function unsub(S memory, S memory) pure returns (S storage) {}
// ----
// TypeError 1884: (186-207): Wrong parameters in operator definition. The function "add" needs to have two parameters of type S and the same data location to be used for the operator +.
// TypeError 1884: (242-263): Wrong parameters in operator definition. The function "sub" needs to have one or two parameters of type S and the same data location to be used for the operator -.
// TypeError 7743: (272-283): Wrong return parameters in operator definition. The function "sub" needs to return a value of the same type and data location as its parameters to be used for the operator -.
// TypeError 1884: (299-320): Wrong parameters in operator definition. The function "mul" needs to have two parameters of type S and the same data location to be used for the operator *.
// TypeError 7743: (329-339): Wrong return parameters in operator definition. The function "mul" needs to return a value of the same type and data location as its parameters to be used for the operator *.
// TypeError 1884: (355-376): Wrong parameters in operator definition. The function "div" needs to have two parameters of type S and the same data location to be used for the operator *.
// TypeError 7743: (450-460): Wrong return parameters in operator definition. The function "bitor" needs to return a value of the same type and data location as its parameters to be used for the operator |.
// TypeError 7743: (512-523): Wrong return parameters in operator definition. The function "unsub" needs to return a value of the same type and data location as its parameters to be used for the operator -.
