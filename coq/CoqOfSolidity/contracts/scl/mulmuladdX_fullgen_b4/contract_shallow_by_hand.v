Require Import CoqOfSolidity.CoqOfSolidity.
Require Import CoqOfSolidity.simulations.CoqOfSolidity.
Import Stdlib.

(*
//error calling modExpPrecompile
uint256 constant _ModExpError=0x7FF;

//Starting from mload(0x40) this is the mapping in allocated memory
//https://medium.com/@ac1d_eth/technical-exploration-of-inline-assembly-in-solidity-b7d2b0b2bda8
//mapping from 0x40 in memory
uint256 constant _Prec_T8=0x800;
uint256 constant _Ap=0x820;
uint256 constant _y2=0x840;
uint256 constant _zzz2=0x860;
uint256 constant _free=0x880;

//mapping from Q in input to function, contains Qx, Qy, Qx', Qy', p, a, gx, gy, gx', gy'
//where P' is P multiplied by 2 pow 128 for shamir's multidimensional trick
//todo: remove all magic numbers
uint constant _Qx=0x00;
uint constant _Qy=0x20;
uint constant _Qx2pow128=0x40;
uint constant _Qy2pow128=0x60;
uint constant _modp=0x80;
uint constant _a=0xa0;
uint constant _gx=0xc0;
uint constant _gy=0xe0;
uint constant _gpow2p128_x=0x100;
uint constant _gpow2p128_y=0x120;
*)
Definition _ModExpError := 0x7FF.

Definition _Prec_T8 := 0x800.
Definition _Ap := 0x820.
Definition _y2 := 0x840.
Definition _zzz2 := 0x860.
Definition _free := 0x880.

Definition _Qx := 0x00.
Definition _Qy := 0x20.
Definition _Qx2pow128 := 0x40.
Definition _Qy2pow128 := 0x60.
Definition _modp := 0x80.
Definition _a := 0xa0.
Definition _gx := 0xc0.
Definition _gy := 0xe0.
Definition _gpow2p128_x := 0x100.
Definition _gpow2p128_y := 0x120.

(*
//store 4 256 bits values starting from addr+offset
function mstore4(addr, offset, val1, val2, val3, val4){
    mstore(add(offset, addr),val1 )
    offset:=add(32, offset)
    mstore(add(offset, addr),val2 )
    offset:=add(32, offset)
    mstore(add(offset, addr),val3 )
    offset:=add(32, offset)
    mstore(add(offset, addr),val4 )
    offset:=add(32, offset)
}
*)
(* Definition mstore4 (addr offset val1 val2 val3 val4 : U256.t) : M.t unit :=
  do~ [[ mstore ~(| add ~(| offset, addr |), val1 |) ]] in
  let~ offset := [[ add ~(| 32, offset |) ]] in
  do~ [[ mstore ~(| add ~(| offset, addr |), val2 |) ]] in
  let~ offset := [[ add ~(| 32, offset |) ]] in
  do~ [[ mstore ~(| add ~(| offset, addr |), val3 |) ]] in
  let~ offset := [[ add ~(| 32, offset |) ]] in
  do~ [[ mstore ~(| add ~(| offset, addr |), val4 |) ]] in
  M.pure tt. *)

(*
//normalized addition of two point, must not be neutral input
function ecAddn2(x1, y1, zz1, zzz1, x2, y2, _p) -> _x, _y, _zz, _zzz {
  y1 := sub(_p, y1)
  y2 := addmod(mulmod(y2, zzz1, _p), y1, _p)
  x2 := addmod(mulmod(x2, zz1, _p), sub(_p, x1), _p)
  _x := mulmod(x2, x2, _p) //PP = P^2
  _y := mulmod(_x, x2, _p) //PPP = P*PP
  _zz := mulmod(zz1, _x, _p) ////ZZ3 = ZZ1*PP

  _zzz := mulmod(zzz1, _y, _p) ////ZZZ3 = ZZZ1*PPP
  zz1 := mulmod(x1, _x, _p) //Q = X1*PP
  _x := addmod(addmod(mulmod(y2, y2, _p), sub(_p, _y), _p), mulmod(sub(_p,2), zz1, _p), _p) //R^2-PPP-2*Q

  x1:=mulmod(addmod(zz1, sub(_p, _x), _p), y2, _p)//necessary split not to explose stack
  _y := addmod(x1, mulmod(y1, _y, _p), _p) //R*(Q-X3)
}
*)
Definition ecAddn2 (x1 y1 zz1 zzz1 x2 y2 _p : U256.t) : M.t (U256.t * U256.t * U256.t * U256.t) :=
  let~ y1 := [[ sub ~(| _p, y1 |) ]] in
  let~ y2 := [[ addmod ~(| mulmod ~(| y2, zzz1, _p |), y1, _p |) ]] in
  let~ x2 := [[ addmod ~(| mulmod ~(| x2, zz1, _p |), sub ~(| _p, x1 |), _p |) ]] in
  let~ _x := [[ mulmod ~(| x2, x2, _p |) ]] in
  let~ _y := [[ mulmod ~(| _x, x2, _p |) ]] in
  let~ _zz := [[ mulmod ~(| zz1, _x, _p |) ]] in

  let~ _zzz := [[ mulmod ~(| zzz1, _y, _p |) ]] in
  let~ zz1 := [[ mulmod ~(| x1, _x, _p |) ]] in
  let~ _x := [[ addmod ~(| addmod ~(| mulmod ~(| y2, y2, _p |), sub ~(| _p, _y |), _p |), mulmod ~(| sub ~(| _p, 2 |), zz1, _p |), _p |) ]] in

  let~ x1 := [[ mulmod ~(| addmod ~(| zz1, sub ~(| _p, _x |), _p |), y2, _p |) ]] in
  let~ _y := [[ addmod ~(| x1, mulmod ~(| y1, _y, _p |), _p |) ]] in

  M.pure (_x, _y, _zz, _zzz).

Module Q.
  (* store Qx, Qy, Q'x, Q'y p, a, gx, gy, gx2pow128, gy2pow128  *)
  Record t : Set := {
    Qx : U256.t;
    Qy : U256.t;
    Q'x : U256.t;
    Q'y : U256.t;
    p : U256.t;
    a : U256.t;
    gx : U256.t;
    gy : U256.t;
    gx2pow128 : U256.t;
    gy2pow128 : U256.t;
  }.
End Q.

Module PZZ.
  Record t : Set := {
    X : U256.t;
    Y : U256.t;
    ZZ : U256.t;
    ZZZ : U256.t;
  }.
End PZZ.

(*
function ecGenMulmuladdX_store(
    uint256 [10] memory Q,//store Qx, Qy, Q'x, Q'y p, a, gx, gy, gx2pow128, gy2pow128 
    uint256 scalar_u,
    uint256 scalar_v
)   public view returns (uint256 X) {
*)
Definition ecGenMulmuladdX_store (Q : Q.t) (scalar_u scalar_v : U256.t) : M.t U256.t :=
  (*
    uint256 mask=1<<127;
    /* I. precomputations phase */

    if(scalar_u==0&&scalar_v==0){
        return 0;
    }
  *)
  let mask := 2 ^ 127 in
  if (scalar_u =? 0) && (scalar_v =? 0) then
    M.pure 0
  else
    (* let _modulusp:=mload(add(mload(0x40), _Ap)) *)
    let _modulusp := Q.(Q.p) in
    (* mstore4(mload(0x40), 128, mload(add(Q,_gx)), mload(add(Q,_gy)), 1, 1) //G the base point *)
    let M_128 : PZZ.t := {|
      PZZ.X := Q.(Q.gx);
      PZZ.Y := Q.(Q.gy);
      PZZ.ZZ := 1;
      PZZ.ZZZ := 1;
    |} in
    (* mstore4(mload(0x40), 256, mload(add(Q,_gpow2p128_x)), mload(add(Q,_gpow2p128_y)), 1, 1) //G'=2^128.G *)
    let M_256 : PZZ.t := {|
      PZZ.X := Q.(Q.gx2pow128);
      PZZ.Y := Q.(Q.gy2pow128);
      PZZ.ZZ := 1;
      PZZ.ZZZ := 1;
    |} in
    (* X:=mload(add(Q,_gpow2p128_x)) *)
    let X := Q.(Q.gx2pow128) in
    (* Y:=mload(add(Q,_gpow2p128_y)) *)
    let Y := Q.(Q.gy2pow128) in
    (* X,Y,ZZ,ZZZ:=ecAddn2( X,Y,1,1, mload(add(Q,_gx)),mload(add(Q,_gy)), _modulusp) //G+G' *)
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 X Y 1 1 Q.(Q.gx) Q.(Q.gy) _modulusp in
    (* mstore4(mload(0x40), 384, X,Y,ZZ,ZZZ) // Q, the public key *)
    let M_384 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    (* mstore4(mload(0x40), 512, mload(Q),mload(add(32,Q)),1,1) *)
    let M_512 : PZZ.t := {|
      PZZ.X := Q.(Q.Qx);
      PZZ.Y := Q.(Q.Qy);
      PZZ.ZZ := 1;
      PZZ.ZZZ := 1;
    |} in
    (* X,Y,ZZ,ZZZ:=ecAddn2( mload(Q),mload(add(Q,32)),1,1, mload(add(Q,_gx)),mload(add(Q,_gy)),_modulusp )//G+Q *)
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 Q.(Q.Qx) Q.(Q.Qy) 1 1 Q.(Q.gx) Q.(Q.gy) _modulusp in
    (* mstore4(mload(0x40), 640, X,Y,ZZ,ZZZ) *)
    let M_640 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    (* X:=mload(add(Q,_gpow2p128_x)) *)
    let X := Q.(Q.gx2pow128) in
    (* Y:=mload(add(Q,_gpow2p128_y)) *)
    let Y := Q.(Q.gy2pow128) in
    (* X,Y,ZZ,ZZZ:=ecAddn2(X,Y,1,1,mload(Q),mload(add(Q,32)), _modulusp)//G'+Q *)
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 X Y 1 1 Q.(Q.Qx) Q.(Q.Qy) _modulusp in
    (* mstore4(mload(0x40), 768, X,Y,ZZ,ZZZ) *)
    let M_768 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    (* X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ, mload(add(Q,_gx)), mload(add(Q,_gy)), _modulusp)//G'+Q+G *)
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 X Y ZZ ZZZ Q.(Q.gx) Q.(Q.gy) _modulusp in
    (* mstore4(mload(0x40), 896, X,Y,ZZ,ZZZ) *)
    let M_896 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    (* mstore4(mload(0x40), 1024, mload(add(Q, 64)), mload(add(Q, 96)),1,1) //Q'=2^128.Q *)
    let M_1024 : PZZ.t := {|
      PZZ.X := Q.(Q.Q'x);
      PZZ.Y := Q.(Q.Q'y);
      PZZ.ZZ := 1;
      PZZ.ZZZ := 1;
    |} in
    (*
    X,Y,ZZ,ZZZ:=ecAddn2(mload(add(Q, 64)), mload(add(Q, 96)),1,1, mload(add(Q,_gx)),mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//Q'+G
    mstore4(mload(0x40), 1152, X,Y,ZZ,ZZZ)
    *)
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 Q.(Q.Q'x) Q.(Q.Q'y) 1 1 Q.(Q.gx) Q.(Q.gy) _modulusp in
    let M_1152 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    (*
    X:=mload(add(Q,_gpow2p128_x))
    Y:=mload(add(Q,_gpow2p128_y))
    X,Y,ZZ,ZZZ:=ecAddn2(mload(add(Q, 64)), mload(add(Q, 96)),1,1, X,Y, mload(add(mload(0x40), _Ap))   )//Q'+G'
    mstore4(mload(0x40), 1280, X,Y,ZZ,ZZZ)
    *)
    let X := Q.(Q.gx2pow128) in
    let Y := Q.(Q.gy2pow128) in
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 Q.(Q.Q'x) Q.(Q.Q'y) 1 1 X Y _modulusp in
    let M_1280 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    (*
    X,Y,ZZ,ZZZ:=ecAddn2(X, Y, ZZ, ZZZ, mload(add(Q,_gx)), mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//Q'+G'+G
    mstore4(mload(0x40), 1408, X,Y,ZZ,ZZZ)
    *)
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 X Y ZZ ZZZ Q.(Q.gx) Q.(Q.gy) _modulusp in
    let M_1408 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    (*
    X,Y,ZZ,ZZZ:=ecAddn2( mload(Q),mload(add(Q,32)),1,1, mload(add(Q, 64)), mload(add(Q, 96)), mload(add(mload(0x40), _Ap))   )//Q+Q'
    mstore4(mload(0x40), 1536, X,Y,ZZ,ZZZ)
    *)
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 Q.(Q.Qx) Q.(Q.Qy) 1 1 Q.(Q.Q'x) Q.(Q.Q'y) _modulusp in
    let M_1536 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    (*
    X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ, mload(add(Q,_gx)), mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//Q+Q'+G
    mstore4(mload(0x40), 1664, X,Y,ZZ,ZZZ)
    *)
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 X Y ZZ ZZZ Q.(Q.gx) Q.(Q.gy) _modulusp in
    let M_1664 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    (*
    X:= mload(add(768, mload(0x40)) )//G'+Q
    Y:= mload(add(800, mload(0x40)) )
    ZZ:= mload(add(832, mload(0x40)) )
    ZZZ:=mload(add(864, mload(0x40)) )
    *)
    let X := M_768.(PZZ.X) in
    let Y := M_768.(PZZ.Y) in
    let ZZ := M_768.(PZZ.ZZ) in
    let ZZZ := M_768.(PZZ.ZZZ) in
    (* X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ,mload(add(Q, 64)), mload(add(Q, 96)), mload(add(mload(0x40), _Ap))   )//G'+Q+Q'+ *)
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 X Y ZZ ZZZ Q.(Q.Q'x) Q.(Q.Q'y) _modulusp in
    (* mstore4(mload(0x40), 1792, X,Y,ZZ,ZZZ) *)
    let M_1792 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    (* X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ,mload(add(Q,0xc0)),mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//G'+Q+Q'+G *)
    let~ '(X, Y, ZZ, ZZZ) := ecAddn2 X Y ZZ ZZZ Q.(Q.gx) Q.(Q.gy) _modulusp in
    (* mstore4(mload(0x40), 1920, X,Y,ZZ,ZZZ) *)
    let M_1920 : PZZ.t := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ;
    |} in
    M.pure 0.
