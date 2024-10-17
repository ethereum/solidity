Require Import CoqOfSolidity.CoqOfSolidity.
Require Import CoqOfSolidity.proofs.CoqOfSolidity.
Require Import CoqOfSolidity.simulations.CoqOfSolidity.
Require Import CoqOfSolidity.contracts.scl.mulmuladdX_fullgen_b4.contract_shallow.
Require Import CoqOfSolidity.contracts.scl.mulmuladdX_fullgen_b4.curve.
Import Stdlib.
Import RunO.

Module Params.
  (* store Qx, Qy, Q'x, Q'y p, a, gx, gy, gx2pow128, gy2pow128  *)
  Record t : Set := {
    Qx : U256.t;
    Qy : U256.t;
    Q'x : U256.t;
    Q'y : U256.t;
    p : U256.t;
    a : U256.t;
    Gx : U256.t;
    Gy : U256.t;
    G'x : U256.t;
    G'y : U256.t;
  }.
End Params.

Definition ecAddn2 (p : U256.t) (P1 : PZZ.t) (P2 : PA.t) : PZZ.t :=
  let usr'dollar'x1 := P1.(PZZ.X) in
  let usr'dollar'y1 := P1.(PZZ.Y) in
  let usr'dollar'zz1 := P1.(PZZ.ZZ) in
  let usr'dollar'zzz1 := P1.(PZZ.ZZZ) in
  let usr'dollar'x2 := P2.(PA.X) in
  let usr'dollar'y2 := P2.(PA.Y) in
  let usr'dollar'_p := p in

  let usr'dollar'y2_1 :=
    Pure.addmod
      (Pure.mulmod usr'dollar'y2 usr'dollar'zzz1 usr'dollar'_p)
      (Pure.sub usr'dollar'_p usr'dollar'y1)
      usr'dollar'_p in
  let usr'dollar'x2_1 :=
    Pure.addmod
      (Pure.mulmod usr'dollar'x2 usr'dollar'zz1 usr'dollar'_p)
      (Pure.sub usr'dollar'_p usr'dollar'x1)
      usr'dollar'_p in
  let usr_x_1 :=
    Pure.mulmod usr'dollar'x2_1 usr'dollar'x2_1 usr'dollar'_p in
  let usr_y_1 :=
    Pure.mulmod usr_x_1 usr'dollar'x2_1 usr'dollar'_p in
  let usr_zz :=
    Pure.mulmod usr'dollar'zz1 usr_x_1 usr'dollar'_p in
  let usr_zzz :=
    Pure.mulmod usr'dollar'zzz1 usr_y_1 usr'dollar'_p in
  let usr'dollar'zz1_1 :=
    Pure.mulmod usr'dollar'x1 usr_x_1 usr'dollar'_p in
  let usr_x :=
    Pure.addmod
      (Pure.addmod
        (Pure.mulmod usr'dollar'y2_1 usr'dollar'y2_1 usr'dollar'_p)
        (Pure.sub usr'dollar'_p usr_y_1)
        usr'dollar'_p)
      (Pure.mulmod
        (Pure.add usr'dollar'_p (Pure.not 1))
        usr'dollar'zz1_1
        usr'dollar'_p)
      usr'dollar'_p in
  let usr_y :=
    Pure.addmod
      (Pure.mulmod
        (Pure.addmod
          usr'dollar'zz1_1
          (Pure.sub usr'dollar'_p usr_x)
          usr'dollar'_p)
        usr'dollar'y2_1
        usr'dollar'_p)
      (Pure.mulmod
        (Pure.sub usr'dollar'_p usr'dollar'y1)
        usr_y_1
        usr'dollar'_p)
      usr'dollar'_p in

  {|
    PZZ.X := usr_x;
    PZZ.Y := usr_y;
    PZZ.ZZ := usr_zz;
    PZZ.ZZZ := usr_zzz
  |}.

Lemma run_usr'dollar'ecAddn2 codes environment state
    (P1_X P1_Y P1_ZZ P1_ZZZ P2_X P2_Y : U256.t) (p : U256.t) :
  let output :=
    ecAddn2 p
      {| PZZ.X := P1_X; PZZ.Y := P1_Y; PZZ.ZZ := P1_ZZ; PZZ.ZZZ := P1_ZZZ |}
      {| PA.X := P2_X; PA.Y := P2_Y |} in
  let output := Result.Ok (output.(PZZ.X), output.(PZZ.Y), output.(PZZ.ZZ), output.(PZZ.ZZZ)) in
  {{? codes, environment, Some state |
    Contract_91.Contract_91_deployed.usr'dollar'ecAddn2 P1_X P1_Y P1_ZZ P1_ZZZ P2_X P2_Y p ⇓
    output
  | Some state ?}}.
Proof.
  simpl.
  unfold Contract_91.Contract_91_deployed.usr'dollar'ecAddn2.
  l. {
    repeat (l; [repeat cu; p|]).
    p.
  }
  p.
Qed.

Lemma run_usr'dollar'ecAddn2_2189 codes environment state
    (P1_X P1_Y P2_X P2_Y : U256.t) (p : U256.t) :
  let output :=
    ecAddn2 p
      {| PZZ.X := P1_X; PZZ.Y := P1_Y; PZZ.ZZ := 1; PZZ.ZZZ := 1 |}
      {| PA.X := P2_X; PA.Y := P2_Y |} in
  let output := Result.Ok (output.(PZZ.X), output.(PZZ.Y), output.(PZZ.ZZ), output.(PZZ.ZZZ)) in
  {{? codes, environment, Some state |
    Contract_91.Contract_91_deployed.usr'dollar'ecAddn2_2189 P1_X P1_Y P2_X P2_Y p ⇓
    output
  | Some state ?}}.
Proof.
  simpl.
  unfold Contract_91.Contract_91_deployed.usr'dollar'ecAddn2_2189.
  l. {
    repeat (l; [repeat cu; p|]).
    p.
  }
  p.
Qed.

Module Ts.
  Definition t : Set := list PZZ.t.

  Definition get (Ts : t) (index : nat) : PZZ.t :=
    List.nth index Ts PZZ.zero.
End Ts.

(*
(* For-loop bounded by some [fuel]. *)
Fixpoint for_loop {State : Set}
    (fuel : nat)
    (state : State)
    (condition : State -> bool)
    (next : State -> State)
    (body : State -> State)
    {struct fuel} :
    State :=
  match fuel with
  | O => state
  | S fuel =>
    let should_continue := condition state in
    if should_continue then
      let state := body state in
      let state := next state in
      for_loop fuel state condition next body
    else
      state
  end.
*)

(*
Definition get_Ts (Q : Q.t) : list PZZ.t :=
  (* let _modulusp:=mload(add(mload(0x40), _Ap)) *)
  let _modulusp := Q.(Q.p) in
  let Ts : Ts.t := [{|
    PZZ.X := 0;
    PZZ.Y := 0;
    PZZ.ZZ := 0;
    PZZ.ZZZ := 0;
  |}] in
  (* mstore4(mload(0x40), 128, mload(add(Q,_gx)), mload(add(Q,_gy)), 1, 1) //G the base point *)
  let Ts := Ts ++ [{|
    PZZ.X := Q.(Q.gx);
    PZZ.Y := Q.(Q.gy);
    PZZ.ZZ := 1;
    PZZ.ZZZ := 1;
  |}] in
  (* mstore4(mload(0x40), 256, mload(add(Q,_gpow2p128_x)), mload(add(Q,_gpow2p128_y)), 1, 1) //G'=2^128.G *)
  let Ts := Ts ++ [{|
    PZZ.X := Q.(Q.gx2pow128);
    PZZ.Y := Q.(Q.gy2pow128);
    PZZ.ZZ := 1;
    PZZ.ZZZ := 1;
  |}] in
  (* X:=mload(add(Q,_gpow2p128_x)) *)
  let X := Q.(Q.gx2pow128) in
  (* Y:=mload(add(Q,_gpow2p128_y)) *)
  let Y := Q.(Q.gy2pow128) in
  (* X,Y,ZZ,ZZZ:=ecAddn2( X,Y,1,1, mload(add(Q,_gx)),mload(add(Q,_gy)), _modulusp) //G+G' *)
  let T :=
    ecAddn2
      (PZZ.of_PA {| PA.X := X; PA.Y := Y |})
      {| PA.X := Q.(Q.gx); PA.Y := Q.(Q.gy) |}
      _modulusp in
  (* mstore4(mload(0x40), 384, X,Y,ZZ,ZZZ) // Q, the public key *)
  let Ts := Ts ++ [T] in
  (* mstore4(mload(0x40), 512, mload(Q),mload(add(32,Q)),1,1) *)
  let Ts := Ts ++ [{|
    PZZ.X := Q.(Q.Qx);
    PZZ.Y := Q.(Q.Qy);
    PZZ.ZZ := 1;
    PZZ.ZZZ := 1;
  |}] in
  (* X,Y,ZZ,ZZZ:=ecAddn2( mload(Q),mload(add(Q,32)),1,1, mload(add(Q,_gx)),mload(add(Q,_gy)),_modulusp )//G+Q *)
  let T :=
    ecAddn2
      (PZZ.of_PA {| PA.X := Q.(Q.Qx); PA.Y := Q.(Q.Qy) |})
      {| PA.X := Q.(Q.gx); PA.Y := Q.(Q.gy) |}
      _modulusp in
  (* mstore4(mload(0x40), 640, X,Y,ZZ,ZZZ) *)
  let Ts := Ts ++ [T] in
  (* X:=mload(add(Q,_gpow2p128_x)) *)
  let X := Q.(Q.gx2pow128) in
  (* Y:=mload(add(Q,_gpow2p128_y)) *)
  let Y := Q.(Q.gy2pow128) in
  (* X,Y,ZZ,ZZZ:=ecAddn2(X,Y,1,1,mload(Q),mload(add(Q,32)), _modulusp)//G'+Q *)
  let T :=
    ecAddn2
      (PZZ.of_PA {| PA.X := X; PA.Y := Y |})
      {| PA.X := Q.(Q.Qx); PA.Y := Q.(Q.Qy) |}
      _modulusp in
  (* mstore4(mload(0x40), 768, X,Y,ZZ,ZZZ) *)
  let Ts := Ts ++ [T] in
  (* X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ, mload(add(Q,_gx)), mload(add(Q,_gy)), _modulusp)//G'+Q+G *)
  let T := ecAddn2 T {| PA.X := Q.(Q.gx); PA.Y := Q.(Q.gy) |} _modulusp in
  (* mstore4(mload(0x40), 896, X,Y,ZZ,ZZZ) *)
  let Ts := Ts ++ [T] in
  (* mstore4(mload(0x40), 1024, mload(add(Q, 64)), mload(add(Q, 96)),1,1) //Q'=2^128.Q *)
  let Ts := Ts ++ [{|
    PZZ.X := Q.(Q.Q'x);
    PZZ.Y := Q.(Q.Q'y);
    PZZ.ZZ := 1;
    PZZ.ZZZ := 1;
  |}] in
  (*
  X,Y,ZZ,ZZZ:=ecAddn2(mload(add(Q, 64)), mload(add(Q, 96)),1,1, mload(add(Q,_gx)),mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//Q'+G
  mstore4(mload(0x40), 1152, X,Y,ZZ,ZZZ)
  *)
  let T :=
    ecAddn2
      (PZZ.of_PA {| PA.X := Q.(Q.Q'x); PA.Y := Q.(Q.Q'y) |})
      {| PA.X := Q.(Q.gx); PA.Y := Q.(Q.gy) |}
      _modulusp in
  let Ts := Ts ++ [T] in
  (*
  X:=mload(add(Q,_gpow2p128_x))
  Y:=mload(add(Q,_gpow2p128_y))
  X,Y,ZZ,ZZZ:=ecAddn2(mload(add(Q, 64)), mload(add(Q, 96)),1,1, X,Y, mload(add(mload(0x40), _Ap))   )//Q'+G'
  mstore4(mload(0x40), 1280, X,Y,ZZ,ZZZ)
  *)
  let X := Q.(Q.gx2pow128) in
  let Y := Q.(Q.gy2pow128) in
  let T :=
    ecAddn2
      (PZZ.of_PA {| PA.X := Q.(Q.Q'x); PA.Y := Q.(Q.Q'y) |})
      {| PA.X := X; PA.Y := Y |}
      _modulusp in
  let Ts := Ts ++ [T] in
  (*
  X,Y,ZZ,ZZZ:=ecAddn2(X, Y, ZZ, ZZZ, mload(add(Q,_gx)), mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//Q'+G'+G
  mstore4(mload(0x40), 1408, X,Y,ZZ,ZZZ)
  *)
  let T := ecAddn2 T {| PA.X := Q.(Q.gx); PA.Y := Q.(Q.gy) |} _modulusp in
  let Ts := Ts ++ [T] in
  (*
  X,Y,ZZ,ZZZ:=ecAddn2( mload(Q),mload(add(Q,32)),1,1, mload(add(Q, 64)), mload(add(Q, 96)), mload(add(mload(0x40), _Ap))   )//Q+Q'
  mstore4(mload(0x40), 1536, X,Y,ZZ,ZZZ)
  *)
  let T :=
    ecAddn2
      (PZZ.of_PA {| PA.X := Q.(Q.Qx); PA.Y := Q.(Q.Qy) |})
      {| PA.X := Q.(Q.Q'x); PA.Y := Q.(Q.Q'y) |}
      _modulusp in
  let Ts := Ts ++ [T] in
  (*
  X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ, mload(add(Q,_gx)), mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//Q+Q'+G
  mstore4(mload(0x40), 1664, X,Y,ZZ,ZZZ)
  *)
  let T := ecAddn2 T {| PA.X := Q.(Q.gx); PA.Y := Q.(Q.gy) |} _modulusp in
  let Ts := Ts ++ [T] in
  (*
  X:= mload(add(768, mload(0x40)) )//G'+Q
  Y:= mload(add(800, mload(0x40)) )
  ZZ:= mload(add(832, mload(0x40)) )
  ZZZ:=mload(add(864, mload(0x40)) )
  *)
  let T := Ts.get Ts 6 in
  (* X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ,mload(add(Q, 64)), mload(add(Q, 96)), mload(add(mload(0x40), _Ap))   )//G'+Q+Q'+ *)
  let T := ecAddn2 T {| PA.X := Q.(Q.Q'x); PA.Y := Q.(Q.Q'y) |} _modulusp in
  (* mstore4(mload(0x40), 1792, X,Y,ZZ,ZZZ) *)
  let Ts := Ts ++ [T] in
  (* X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ,mload(add(Q,0xc0)),mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//G'+Q+Q'+G *)
  let T := ecAddn2 T {| PA.X := Q.(Q.gx); PA.Y := Q.(Q.gy) |} _modulusp in
  (* mstore4(mload(0x40), 1920, X,Y,ZZ,ZZZ) *)
  let Ts := Ts ++ [T] in
  Ts.
*)

(*
  {
  //X,Y,ZZ,ZZZ:=ecDblNeg(X,Y,ZZ,ZZZ), not having it inplace increase by 12K the cost of the function

  let T1 := mulmod(2, Y, _p) //U = 2*Y1, y free
  let T2 := mulmod(T1, T1, _p) // V=U^2
  let T3 := mulmod(X, T2, _p) // S = X1*V
  T1 := mulmod(T1, T2, _p) // W=UV
  let T4:=mulmod(mload(add(Q,_a)),mulmod(ZZ,ZZ,_p),_p)

  T4 := addmod(mulmod(3, mulmod(X,X,_p),_p),T4,_p)//M=3*X12+aZZ12  
  ZZZ := mulmod(T1, ZZZ, _p) //zzz3=W*zzz1
  ZZ := mulmod(T2, ZZ, _p) //zz3=V*ZZ1
  X:=sub(_p,2)//-2
  X := addmod(mulmod(T4, T4, _p), mulmod(X, T3, _p), _p) //X3=M^2-2S
  T2 := mulmod(T4, addmod(X, sub(_p, T3), _p), _p) //-M(S-X3)=M(X3-S)
  Y := addmod(mulmod(T1, Y, _p), T2, _p) //-Y3= W*Y1-M(S-X3), we replace Y by -Y to avoid a sub in ecAdd
  //Y:=sub(p,Y)*/

  }
*)
Definition ecDblNeg (a p : Z) (P : PZZ.t) : PZZ.t :=
  let '{|
    PZZ.X := X;
    PZZ.Y := Y;
    PZZ.ZZ := ZZ;
    PZZ.ZZZ := ZZZ
  |} := P in

  let T1 := Pure.mulmod 2 Y p in
  let T2 := Pure.mulmod T1 T1 p in
  let T3 := Pure.mulmod X T2 p in
  let T1 := Pure.mulmod T1 T2 p in
  let T4 := Pure.mulmod a (Pure.mulmod ZZ ZZ p) p in
  let T4 := Pure.addmod (Pure.mulmod 3 (Pure.mulmod X X p) p) T4 p in
  let ZZZ := Pure.mulmod T1 ZZZ p in
  let ZZ := Pure.mulmod T2 ZZ p in
  let X := Pure.sub p 2 in
  let X := Pure.addmod
    (Pure.mulmod T4 T4 p)
    (Pure.mulmod X T3 p)
    p in
  let T2 := Pure.mulmod T4 (Pure.addmod X (Pure.sub p T3) p) p in
  let Y := Pure.addmod (Pure.mulmod T1 Y p) T2 p in

  {|
    PZZ.X := X;
    PZZ.Y := Y;
    PZZ.ZZ := ZZ;
    PZZ.ZZZ := ZZZ
  |}.

(*
Module MainLoop.
  Module State.
    Record t : Set := {
      mask : U256.t;
      P1 : PZZ.t;
      _y2 : U256.t;
      _zzz2 : U256.t;
    }.
  End State.

  Definition condition (state : State.t) : bool :=
    let '{| State.mask := mask |} := state in
    mask >? 0.

  Definition next (state : State.t) : State.t :=
    let mask := Pure.shr 1 state.(State.mask) in
    state <| State.mask := mask |>.

  Definition body (a p : Z) (u v : U256.t) (Ts : list PZZ.t) (state : State.t) : State.t :=
    let '{|
      State.mask := mask;
      State.P1 := P1;
      State._y2 := _y2;
      State._zzz2 := _zzz2
    |} := state in
    (*
    {
    //X,Y,ZZ,ZZZ:=ecDblNeg(X,Y,ZZ,ZZZ), not having it inplace increase by 12K the cost of the function

    ...

    }
    *)
    let P1 := ecDblNeg a p P1 in
    (*
    let T1:=add(add(sub(1,iszero(and(scalar_u, mask))), shl(1,sub(1,iszero(and(shr(128, scalar_u), mask))))),
      add(shl(2,sub(1,iszero(and(scalar_v, mask)))), shl(3,sub(1,iszero(and(shr(128, scalar_v), mask))))))
    *)
    let s := get_selector u v mask in
    (*
    if iszero(T1) {
                  Y := sub(_p, Y)
                  continue
    }
    *)
    if s =? 0 then
      let Y := Pure.sub p P1.(PZZ.Y) in
      let state := {|
        State.mask := mask;
        State.P1 := {|
          PZZ.X := P1.(PZZ.X);
          PZZ.Y := Y;
          PZZ.ZZ := P1.(PZZ.ZZ);
          PZZ.ZZZ := P1.(PZZ.ZZZ)
        |};
        State._y2 := _y2;
        State._zzz2 := _zzz2
      |} in
      state
    else
    (*
    T1:=shl(7, T1)//Commputed value address offset
    let T4:=mload(add(Mem,T1))//X2
    *)
    let T4 := (Ts.get Ts (Z.to_nat s)).(PZZ.X) in
    (* mstore(add(Mem,_y2), addmod(mulmod( mload(add(Mem,add(32,T1))), ZZZ, _p), mulmod(Y,mload(add(Mem, _zzz2)), _p), _p))//R=S2-S1, sub avoided *)
    let _y2 := Pure.addmod
      (Pure.mulmod (Ts.get Ts (Z.to_nat s)).(PZZ.Y) P1.(PZZ.ZZZ) p)
      (Pure.mulmod P1.(PZZ.Y) _zzz2 p)
      p in
    (* T1:=mload(add(Mem,add(64,T1)))//zz2 *)
    let T1 := (Ts.get Ts (Z.to_nat s)).(PZZ.ZZ) in
    (* let T2 := addmod(mulmod(T4, ZZ, _p), sub(_p, mulmod(X,T1,_p)), _p)//P=U2-U1 *)
    let T2 := Pure.addmod
      (Pure.mulmod T4 P1.(PZZ.ZZ) p)
      (Pure.sub p (Pure.mulmod P1.(PZZ.X) T1 p))
      p in
    (*
    //special case ecAdd(P,P)=EcDbl
    if iszero(mload(add(Mem,_y2))) {
        if iszero(T2) {
            T1 := mulmod(sub(_p,2), Y, _p) //U = 2*Y1, y free
            T2 := mulmod(T1, T1, _p) // V=U^2
            mstore(add(Mem,_y2), mulmod(X, T2, _p)) // S = X1*V

            T1 := mulmod(T1, T2, _p) // W=UV
            T4:=mulmod(mload(add(Q,_a)),mulmod(ZZ,ZZ,_p),_p)
            T4 := addmod(mulmod(3, mulmod(X,X,_p),_p),T4,_p)//M=3*X12+aZZ12   //M

            ZZZ := mulmod(T1, ZZZ, _p) //zzz3=W*zzz1
            ZZ := mulmod(T2, ZZ, _p) //zz3=V*ZZ1, V free

            X := addmod(mulmod(T4, T4, _p), mulmod(sub(_p,2), mload(add(Mem, _y2)), _p), _p) //X3=M^2-2S
            T2 := mulmod(T4, addmod(mload(add(Mem, _y2)), sub(_p, X), _p), _p) //M(S-X3)

            Y := addmod(T2, mulmod(T1, Y, _p), _p) //Y3= M(S-X3)-W*Y1

            continue
        }
    }
    *)
    if (_y2 =? 0) && (T2 =? 0) then
      let T1 := Pure.mulmod (Pure.sub p 2) P1.(PZZ.Y) p in
      let T2 := Pure.mulmod T1 T1 p in
      let _y2 := Pure.mulmod P1.(PZZ.X) T2 p in
      let T1 := Pure.mulmod T1 T2 p in
      let T4 := Pure.mulmod a (Pure.mulmod P1.(PZZ.ZZ) P1.(PZZ.ZZ) p) p in
      let T4 := Pure.addmod (Pure.mulmod 3 (Pure.mulmod P1.(PZZ.X) P1.(PZZ.X) p) p) T4 p in
      let ZZZ := Pure.mulmod T1 P1.(PZZ.ZZZ) p in
      let ZZ := Pure.mulmod T2 P1.(PZZ.ZZ) p in
      let X := Pure.addmod
        (Pure.mulmod T4 T4 p)
        (Pure.mulmod (Pure.sub p 2) _y2 p)
        p in
      let T2 := Pure.mulmod T4 (Pure.addmod _y2 (Pure.sub p X) p) p in
      let Y := Pure.addmod T2 (Pure.mulmod T1 P1.(PZZ.Y) p) p in
      let state := {|
        State.mask := mask;
        State.P1 := {|
          PZZ.X := X;
          PZZ.Y := Y;
          PZZ.ZZ := ZZ;
          PZZ.ZZZ := ZZZ
        |};
        State._y2 := _y2;
        State._zzz2 := _zzz2
      |} in
      state
    else
    (*
    T4 := mulmod(T2, T2, _p) //PP
    T2 := mulmod(T4, T2, _p) //PPP
    ZZ := mulmod(mulmod(ZZ, T4,_p), T1 ,_p)//zz3=zz1*zz2*PP
    T1:= mulmod(X,T1, _p)
    ZZZ := mulmod(mulmod(ZZZ, T2, _p), mload(add(Mem, _zzz2)),_p) // zzz3=zzz1*zzz2*PPP
    X := addmod(addmod(mulmod(mload(add(Mem, _y2)), mload(add(Mem, _y2)), _p), sub(_p, T2), _p), mulmod( T1 ,mulmod(sub(_p,2), T4, _p),_p ), _p)// R2-PPP-2*U1*PP
    T4 := mulmod(T1, T4, _p)///Q=U1*PP
    Y := addmod(mulmod(addmod(T4, sub(_p, X), _p), mload(add(Mem, _y2)), _p), mulmod(mulmod(Y,mload(add(Mem, _zzz2)), _p), T2, _p), _p)// R*(Q-X3)-S1*PPP
    *)
    let T4 := Pure.mulmod T2 T2 p in
    let T2 := Pure.mulmod T4 T2 p in
    let ZZ := Pure.mulmod (Pure.mulmod P1.(PZZ.ZZ) T4 p) T1 p in
    let T1 := Pure.mulmod P1.(PZZ.X) T1 p in
    let ZZZ := Pure.mulmod (Pure.mulmod P1.(PZZ.ZZZ) T2 p) _zzz2 p in
    let X := Pure.addmod
      (Pure.addmod (Pure.mulmod _y2 _y2 p) (Pure.sub p T2) p)
      (Pure.mulmod T1 (Pure.mulmod (Pure.sub p 2) T4 p) p)
      p in
    let T4 := Pure.mulmod T1 T4 p in
    let Y := Pure.addmod
      (Pure.mulmod (Pure.addmod T4 (Pure.sub p X) p) _y2 p)
      (Pure.mulmod (Pure.mulmod P1.(PZZ.Y) _zzz2 p) T2 p)
      p in
    let state := {|
      State.mask := mask;
      State.P1 := {|
        PZZ.X := X;
        PZZ.Y := Y;
        PZZ.ZZ := ZZ;
        PZZ.ZZZ := ZZZ
      |};
      State._y2 := _y2;
      State._zzz2 := _zzz2
    |} in
    state.
End MainLoop.
*)

(*
Definition sim_fun_ecGenMulmuladdX_store_2814_beginning (Q : Q.t) (scalar_u scalar_v : U256.t) :
    unit :=
  (*
    uint256 mask=1<<127;
    /* I. precomputations phase */

    if(scalar_u==0&&scalar_v==0){
        return 0;
    }
  *)
  let mask := 2 ^ 127 in
  if (scalar_u =? 0) && (scalar_v =? 0) then
    tt
  else
  let _modulusp := Q.(Q.p) in
  let Ts := get_Ts Q in
  tt.
*)

Module PointsSelector.
  Record t : Set := {
    u_low : bool;
    u_high : bool;
    v_low : bool;
    v_high : bool;
  }.

  Definition is_zero (selector : t) : bool :=
    match selector with
    | Build_t false false false false => true
    | _ => false
    end.

  Definition to_Z (selector : t) : Z :=
    let 'Build_t u_low u_high v_low v_high := selector in
    Z.b2z u_low + 2 * Z.b2z u_high + 4 * Z.b2z v_low + 8 * Z.b2z v_high.

  Definition add_points (p : Z) (Q Q' G G' : PA.t) (selector : t) : PZZ.t :=
    List.fold_left (ecAddn2 p) [Q; Q'; G; G'] PZZ.zero.
End PointsSelector.

Module U128.
  Definition t : Set :=
    Z.

  Module Valid.
    Definition t (n : Z) : Prop :=
      0 <= n < 2 ^ 128.
  End Valid.
End U128.

Module HighLow.
  Definition high (x : U256.t) : U256.t :=
    x / (2 ^ 128).

  Definition low (x : U256.t) : U256.t :=
    x mod (2 ^ 128).

  Definition merge (high low : U256.t) : U256.t :=
    high * (2 ^ 128) + low.

  Lemma merge_high_low_eq (x : U256.t) :
    merge (high x) (low x) = x.
  Proof.
    unfold U256.Valid.t, high, low, merge in *.
    lia.
  Qed.

  Lemma test_bit_merge_low_eq (n_low n_high : U128.t) (index : Z)
      (H_n_low : U128.Valid.t n_low)
      (H_n_high : U128.Valid.t n_high)
      (H_index : 0 <= index < 128) :
    Z.testbit (merge n_high n_low) index = Z.testbit n_low index.
  Proof.
    unfold merge, U128.Valid.t in *.
    apply Z.b2z_inj.
    repeat rewrite Z.testbit_spec' by lia.
  Admitted.

  Lemma high_merge_eq (n_low n_high : U128.t)
      (H_n_low : U128.Valid.t n_low)
      (H_n_high : U128.Valid.t n_high) :
    high (merge n_high n_low) = n_high.
  Proof.
    unfold high, merge, U128.Valid.t in *.
    lia.
  Qed.

  Definition raw_get_selector (u v mask : U256.t) : U256.t :=
    Pure.add
      (Pure.add
        (Pure.sub 1 (Pure.iszero (Pure.and u mask)))
        (Pure.shl 1 (Pure.sub 1 (Pure.iszero (Pure.and (Pure.shr 128 u) mask))))
      )
      (Pure.add
        (Pure.shl 2 (Pure.sub 1 (Pure.iszero (Pure.and v mask))))
        (Pure.shl 3 (Pure.sub 1 (Pure.iszero (Pure.and (Pure.shr 128 v) mask))))
      ).

  Definition get_selector (u_low u_high v_low v_high : U256.t) (index : Z) : PointsSelector.t :=
    {|
      PointsSelector.u_low := Z.testbit u_low index;
      PointsSelector.u_high := Z.testbit u_high index;
      PointsSelector.v_low := Z.testbit v_low index;
      PointsSelector.v_high := Z.testbit v_high index
    |}.

  Lemma get_selector_eq (u_low u_high v_low v_high : U256.t) (index : Z)
      (H_u_low : U128.Valid.t u_low)
      (H_u_high : U128.Valid.t u_high)
      (H_v_low : U128.Valid.t v_low)
      (H_v_high : U128.Valid.t v_high)
      (H_index : 0 <= index < 128) :
    PointsSelector.to_Z (get_selector u_low u_high v_low v_high index) =
    raw_get_selector (merge u_high u_low) (merge v_high v_low) (2 ^ index).
  Proof.
    unfold PointsSelector.to_Z, raw_get_selector, get_selector, merge.
    unfold Pure.and, Pure.add, Pure.sub, Pure.iszero, Pure.shl, Pure.shr.
    unfold U128.Valid.t in *.
    assert (H_land_eq :
        forall (a b : Z),
        0 <= a ->
        (Z.land a b =? 0) =
        negb (0 <? Z.land a b)
      ). {
      intros.
      destruct (Z.land_nonneg a b).
      lia.
    }
    repeat rewrite H_land_eq by lia.
    repeat rewrite <- Arith2.Z_testbit_alt by lia.
    pose proof (H_test_low := test_bit_merge_low_eq).
    unfold merge in H_test_low.
    repeat rewrite H_test_low by (unfold U128.Valid.t; lia).
    pose proof (H_test_high := high_merge_eq).
    unfold high, merge in H_test_high.
    repeat rewrite H_test_high by (unfold U128.Valid.t; lia).
    repeat destruct (Z.testbit _ _); reflexivity.
  Qed.
End HighLow.

Module MostSignificantBit.
  Fixpoint get (u_low u_high v_low v_high : U256.t) (index : nat) :
      option PointsSelector.t :=
    let selector := HighLow.get_selector u_low u_high v_low v_high (Z.of_nat index) in
    if PointsSelector.is_zero selector then
      match index with
      | O => None (* we should never reach this case is the values are not all zero *)
      | S index' => get u_low u_high v_low v_high index'
      end
    else
      Some selector.

  Fixpoint get_until (u_low u_high v_low v_high : U256.t) (index until_index : nat) :
      option PointsSelector.t :=
    if (index <? until_index)%nat then
      None
    else
    let selector := HighLow.get_selector u_low u_high v_low v_high (Z.of_nat index) in
    if PointsSelector.is_zero selector then
      match index with
      | O => None
      | S index' => get_until u_low u_high v_low v_high index' until_index
      end
    else
      Some selector.

  Lemma get_until_eq (u_low u_high v_low v_high : U256.t) (index : nat) :
    get_until u_low u_high v_low v_high index 0 =
    get u_low u_high v_low v_high index.
  Proof.
    induction index; cbn; best.
  Qed.

  (* Module Loop.
    Module State.
      Record t : Set := {
        ZZZ : U256.t;
        log_mask : Z;
      }.

      Definition to_output (fuel : nat) (state : t) : BlockUnit.t :=
        get_until
    End State.
  End Loop. *)
End MostSignificantBit.

Ltac load_store_line :=
  with_strategy opaque [ecAddn2] (
    (* We do that to avoid an exponential increase of the output *)
    try set (ecAddn2 _ _ _) in |- *;
    l; [repeat (
      c; [
        p ||
        apply_run_mload ||
        apply_run_mstore ||
        apply run_usr'dollar'ecAddn2 ||
        apply run_usr'dollar'ecAddn2_2189
      |];
      CanonizeState.execute;
      s
    ); p|];
    s;
    try p
  ).

(* Lemma run_loop_most_significant_bit codes environment state
    (
      mem0 mem1 mem2 mem3 mem4 mem5 mem6 mem7 mem8 mem9
      mem10 mem11 mem12 mem13 mem14 :
      U256.t
    )
    (Q Q' G G' : PA.t) (p a : Z) (u_low u_high v_low v_high : U256.t) :
  let u := HighLow.merge u_high u_low in
  let v := HighLow.merge v_high v_low in
  let params := {|
    Params.Qx := Q.(PA.X);
    Params.Qy := Q.(PA.Y);
    Params.Q'x := Q'.(PA.X);
    Params.Q'y := Q'.(PA.Y);
    Params.p := p;
    Params.a := a;
    Params.Gx := G.(PA.X);
    Params.Gy := G.(PA.Y);
    Params.G'x := G'.(PA.X);
    Params.G'y := G'.(PA.Y)
  |} in
  let memoryguard : U256.t := 0 in
  let params_offset : U256.t := 32 * 15 in
  let memory_start : list U256.t :=
    [
      mem0; mem1; memoryguard; mem3; mem4; mem5; mem6; mem7; mem8; u;
      params_offset; v; mem12; mem13; mem14;
      params.(Params.Qx);
      params.(Params.Qy);
      params.(Params.Q'x);
      params.(Params.Q'y);
      params.(Params.p);
      params.(Params.a);
      params.(Params.Gx);
      params.(Params.Gy);
      params.(Params.G'x);
      params.(Params.G'y)
    ] ++ List.repeat 0 200 in
  let state_start :=
      make_state environment state memory_start [] in
  let memory_end : list U256.t :=
    [1; 2; 3; 4; 5; 6] in
  let state_end :=
    make_state environment state memory_end [] in
  (* let output := sim_fun_ecGenMulmuladdX_store_2814_beginning Q scalar_u scalar_v in *)
  let output := Result.Ok tt in
  {{? codes, environment, Some state_start |
    Contract_91.Contract_91_deployed.fun_ecGenMulmuladdX_store_2814 ⇓
    output
  | Some state_end ?}}.
Proof. *)



Lemma run_fun_ecGenMulmuladdX_store_2814 codes environment state
    (
      mem0 mem1 mem3 mem4 mem5 mem6 mem7 mem8
      mem12 mem13 mem14 :
      U256.t
    )
    (Q Q' G G' : PA.t) (p a : Z) (u_low u_high v_low v_high : U256.t) :
  let u := HighLow.merge u_high u_low in
  let v := HighLow.merge v_high v_low in
  let params := {|
    Params.Qx := Q.(PA.X);
    Params.Qy := Q.(PA.Y);
    Params.Q'x := Q'.(PA.X);
    Params.Q'y := Q'.(PA.Y);
    Params.p := p;
    Params.a := a;
    Params.Gx := G.(PA.X);
    Params.Gy := G.(PA.Y);
    Params.G'x := G'.(PA.X);
    Params.G'y := G'.(PA.Y)
  |} in
  let memoryguard : U256.t := 0 in
  let params_offset : U256.t := 32 * 15 in
  let memory_start : list U256.t :=
    [
      mem0; mem1; memoryguard; mem3; mem4; mem5; mem6; mem7; mem8; u;
      params_offset; v; mem12; mem13; mem14;
      params.(Params.Qx);
      params.(Params.Qy);
      params.(Params.Q'x);
      params.(Params.Q'y);
      params.(Params.p);
      params.(Params.a);
      params.(Params.Gx);
      params.(Params.Gy);
      params.(Params.G'x);
      params.(Params.G'y)
    ] ++ List.repeat 0 200 in
  let state_start :=
      make_state environment state memory_start [] in
  let memory_end : list U256.t :=
    [1; 2; 3; 4; 5; 6] in
  let state_end :=
    make_state environment state memory_end [] in
  (* let output := sim_fun_ecGenMulmuladdX_store_2814_beginning Q scalar_u scalar_v in *)
  let output := Result.Ok tt in
  {{? codes, environment, Some state_start |
    Contract_91.Contract_91_deployed.fun_ecGenMulmuladdX_store_2814 ⇓
    output
  | Some state_end ?}}.
Proof.
  intros u v; simpl.
  unfold Contract_91.Contract_91_deployed.fun_ecGenMulmuladdX_store_2814.
  l. {
    repeat load_store_line.
    l. {
      unfold Shallow.if_, Pure.iszero.
      instantiate (2 := Result.Ok (BlockUnit.Tt, if u =? 0 then _ else _)).
      destruct (u =? 0); s; [|p].
      load_store_line.
    }
    s.
    lu.
    match goal with
    | |- context [Shallow.if_ ?condition] =>
      replace condition with (Z.b2z ((u =? 0) && (v =? 0)))
    end. 2: {
      unfold Pure.iszero.
      now repeat destruct (_ =? 0).
    }
    match goal with
    | |- context [Shallow.if_ (Z.b2z ?condition)] =>
      destruct condition eqn:H_u_v_eq; s
    end.
    { load_store_line. }
    { (* Here we fill the memory with all the possible combinations of additions *)
      repeat load_store_line.
      (* We simplify these additions that are a little bit too unfolded *)
      repeat match goal with
      | t := ecAddn2 _ ?P1 ?P2 : _ |- _ =>
        match P1 with
        | {| PZZ.X := ?P.(PA.X) |} =>
          change P1 with (PZZ.of_PA P) in t
        end ||
        match goal with
        | P := _ : _ |- _ =>
          change P1 with P in t
        end ||
        match P2 with
        | {| PA.X := ?P.(PA.X) |} =>
          change P2 with P in t
        end
      end.
      (* Computation of the most-significant bit *)
      l. {
        change (Pure.shl 127 1) with (2 ^ Z.of_nat (128 - 1)).
        set (fuel := 128%nat).
        (* assert (H_index_le : (index <= 128)%nat) by lia. *)
        (* assert (H_get_s :
          forall i, Z.of_nat index <= i <= 127 -> get_selector u v i = 0
        ) by lia. *)
        (* Ltac foo index word2 :=
          let index := eval cbv in (Z.to_nat (index / 32)) in
          eapply (Memory.update_at index word2);
            try apply get_memory_make_state_eq;
            [|reflexivity|];
            unfold List.replace_nth;
            CanonizeState.execute. *)
        set (get_mask := fun (fuel : nat) => 2 ^ Z.of_nat (128 - 1)).
        set (mask_address := 0x01a0).
        set (get_result := fun (fuel : nat) =>
          MostSignificantBit.get_until u_low u_high v_low v_high 127 fuel
        ).
        set (get_ZZZ := fun (fuel : nat) =>
          match get_result fuel with
          | Some selector => PointsSelector.to_Z selector
          | None => 0
          end
        ).
        set (ZZZ_address := 0xe0).
        apply_memory_update_at mask_address (get_mask fuel). {
          reflexivity.
        }
        apply_memory_update_at ZZZ_address (get_ZZZ fuel). {
          reflexivity.
        }
        induction fuel.
        { (* The base case, corresponding to a bit position of `-1`, is impossible to reach. *)
          (* assert (H_u_v_zero :
            (forall i, 0 <= i <= 127 -> get_selector u scalar_v i = 0) ->
            u = 0 /\ scalar_v = 0
          ) by admit.
          exfalso; lia. *)
          admit.
        }
        { 
          eapply LoopStep with
            (output_inter :=
              match get_result fuel with
              | None => Result.Ok (BlockUnit.Tt, tt)
              | Some _ => Result.Ok (BlockUnit.Break, tt)
              end
            )
            (state_inter :=

            ).
          { (* for body *)
            with_strategy opaque [MostSignificantBit.get_until] load_store_line.
            set (_127 := 127%nat).
            unfold MostSignificantBit.get_until at 2.
            with_strategy opaque [MostSignificantBit.get_until] s.
            unfold _127; clear _127.
            destruct (index =? 127)%nat eqn:?; s.
            { l. {
                load_store_line.
              }
              load_store_line.
            }
            { 

            }
            l. {
              load_store_line.
            }
            load_store_line.
          }
          {
        }
        eapply LoopStep.
        { Compute (224 / 32). load_store_line.
          l. {
            load_store_line.
          }
          load_store_line.
        }
        { induction index.
          { assert (H_u_v_zero :
              (forall i, 0 <= i <= 127 -> get_selector scalar_u scalar_v i = 0) ->
              scalar_u = 0 /\ scalar_v = 0
            ) by admit.
            exfalso; lia.
          }
          { apply IHindex; clear IHindex.
            { lia. }
            { 

            }
          }
        }
          l. {
            c. {
              apply_run_mload.
            }
            cu; p.
          }
          s.
          l. {
            l. {
              repeat (c; [
                p ||
                apply_run_mload
              |]).
              s.
              c. {
                match goal with
                | |- context[mstore _ ?value] =>
                  change value with (get_selector scalar_u scalar_v (2 ^ 127))
                end.
                apply_run_mstore.
              }
              CanonizeState.execute.
              p.
            }
            p.
          }
          s.
          l. {
              c. {
                apply_run_mload.
              }
              s.
              cu; s.
              c. {
                match goal with
                | |- context[mstore _ ?value] =>
                  change value with (2 ^ 126)
                end.
                apply_run_mstore.
              }
              CanonizeState.execute.
              p.
            }
            s.
            p.
          }
          { 

          }

        }
        simpl in (Pure.shl _ _).
        match goal with
        | |- context[Pure.shl _ _] =>
        end.
        cbv in (Pure.shl _ _).
        match goal with
        end.
        unfold Pure.shl.
        generalize 127.
        Compute 416 / 32.
      }





      Time with_strategy opaque [ecAddn2] do 10 load_store_line.
      Time with_strategy opaque [ecAddn2] do 10 load_store_line.
      Time with_strategy opaque [ecAddn2] do 10 load_store_line.
      Time with_strategy opaque [ecAddn2] do 10 load_store_line.
      Time with_strategy opaque [ecAddn2] do 10 load_store_line.
      Time with_strategy opaque [ecAddn2] do 10 load_store_line.
      Time with_strategy opaque [ecAddn2] repeat load_store_line.
      repeat set (ecAddn2 _ _) in |- *.
      set (ecAddn2 _ _) in |- *.
      
    repeat load_store_line.
      l. {
        c. {
          apply_run_mload.
        }
        c. {
          apply_run_mload.
        }
        c. {
          apply run_usr'dollar'ecAddn2_2189.
        }
        p.
      }
      with_strategy opaque [ecAddn2] repeat load_store_line.
      set (GA := {| PA.X := Q.(Q.gx); PA.Y := Q.(Q.gy) |}).
      set (G'ZZ :=
        {| PZZ.X := Q.(Q.gx2pow128); PZZ.Y := Q.(Q.gy2pow128); PZZ.ZZ := 1; PZZ.ZZZ := 1 |}).
      l. {
        c. {
          apply_run_mload.
        }
        c. {
          apply_run_mload.
        }
        c. {
          apply_run_mload.
        }
        c. {
          apply run_usr'dollar'ecAddn2_2189.
        }
        p.
      }
      with_strategy opaque [ecAddn2] repeat load_store_line.
        }
      }
      Compute 
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
      load_store_line.
    
    
    
      l. {
        c. {
          apply_run_mstore.
        }
        p.
      }
      CanonizeState.execute.
      l. {
        c. {
          apply_run_mstore.
        }
        p.
      }
      CanonizeState.execute.
      l. {
        c. {
          apply_run_mstore.
        }
        p.
      }
      CanonizeState.execute.
      l. {
        c. {
          apply_run_mload.
        }
        p.
      }
      l. {
        cu; s.
        change (Pure.add 0 2048) with 2048.
        c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        p.
      }
      l. {
        cu; s.
        c. {
          apply_run_mload.
        }
        s.
        cu; s.
        c. {
          apply_run_mload.
        }
        s.
        c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        p.
      }
      l. {
        c. {
          apply_run_mload.
        }
        p.
      }
      (* let~ usr_modulusp := [[ mload ~(| add ~(| _2, 2080 |) |) ]] in *)
      l. {
        cu; s.
        c. {
          apply_run_mload.
        }
        p.
      }
      (* let~ _3 := [[ add ~(| mload ~(| 0x0140 |), 224 |) ]] in *)
      l. {
        c. {
          apply_run_mload.
        }
        cu; p.
      }
      (* let~ _4 := [[ mload ~(| _3 |) ]] in *)
      l. {
        c. {
          apply_run_mload.
        }
        p.
      }
      (* let~ _5 := [[ add ~(| mload ~(| 0x0140 |), 192 |) ]] in *)
      l. {
        c. {
          apply_run_mload.
        }
        cu; p.
      }
      (* do~ [[ mstore ~(| add ~(| 128, _2 |), mload ~(| _5 |) |) ]] in *)
        l; [repeat (
          c; [
            p ||
            apply_run_mload ||
            apply_run_mstore
          |];
          CanonizeState.execute;
          s
        )|].
        match goal with
        | |- context[]
        end.
        cu; s.
        c. {
          apply_run_mload.
        }
        c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        p.
      }
      (* let~ _6 := [[ add ~(| mload ~(| 0x0140 |), 160 |) ]] in *)
    }
    repeat (l; [repeat cu; p|]).
    p.
  }
Qed.

(*
function ecGenMulmuladdX_store(
    uint256 [10] memory Q,//store Qx, Qy, Q'x, Q'y p, a, gx, gy, gx2pow128, gy2pow128 
    uint256 scalar_u,
    uint256 scalar_v
)   public view returns (uint256 X) {
*)
Definition ecGenMulmuladdX_store (Q : Q.t) (scalar_u scalar_v : U256.t) : U256.t :=
  (*
    uint256 mask=1<<127;
    /* I. precomputations phase */

    if(scalar_u==0&&scalar_v==0){
        return 0;
    }
  *)
  let mask := 2 ^ 127 in
  if (scalar_u =? 0) && (scalar_v =? 0) then
    0
  else
  let _modulusp := Q.(Q.p) in
  let Ts := get_Ts Q in
  (*
  ZZZ:=0
  for {} iszero(ZZZ) { mask := shr(1, mask) }{
  ZZZ:=
    add(
      add(
        sub(1,iszero(and(scalar_u, mask))),
        shl(1,sub(1,iszero(and(shr(128, scalar_u), mask))))
      ),
      add(
        shl(2,sub(1,iszero(and(scalar_v, mask)))),
        shl(3,sub(1,iszero(and(shr(128, scalar_v), mask))))
      )
    )
  }
  *)
  (* [(mask, ZZZ)] *)
  let State : Set := (U256.t * U256.t)%type in
  let condition (state : State) : bool :=
    let '(mask, s) := state in
    s =? 0 in
  let next (state : State) : State :=
    let '(mask, s) := state in
    let mask := Pure.shr 1 mask in
    let state := (mask, s) in
    state in
  let body (state : State) : State :=
    let '(mask, s) := state in
    let s := Pure.add
      (Pure.add
        (Pure.sub 1 (Pure.iszero (Pure.and scalar_u mask)))
        (Pure.shl 1 (Pure.sub 1 (Pure.iszero (Pure.and (Pure.shr 128 scalar_u) mask))))
      )
      (Pure.add
        (Pure.shl 2 (Pure.sub 1 (Pure.iszero (Pure.and scalar_v mask))))
        (Pure.shl 3 (Pure.sub 1 (Pure.iszero (Pure.and (Pure.shr 128 scalar_v) mask))))
      ) in
    let state := (mask, s) in
    state in
  let s := 0 in
  let state : State := (mask, s) in
  let state := for_loop 128 state condition next body in
  let '(mask, s) := state in

  (*
  X:=mload(add(mload(0x40),shl(7,s)))//X
  Y:=mload(add(mload(0x40),add(32, shl(7,s))))//Y
  ZZ:=mload(add(mload(0x40),add(64, shl(7,s))))//ZZ
  ZZZ:=mload(add(mload(0x40),add(96, shl(7,ZZZ))))//ZZZ
  *)

  let '{| PZZ.X := X; PZZ.Y := Y; PZZ.ZZ := ZZ; PZZ.ZZZ := ZZZ |} := Ts.get Ts (Z.to_nat s) in
  (* for {} gt(mask, 0) { mask := shr(1, mask) } { *)
  let State : Set := (U256.t * U256.t * U256.t * U256.t * U256.t * U256.t * U256.t)%type in
  let condition (state : State) : bool :=
    let '(mask, X, Y, ZZ, ZZZ, _y2, _zzz2) := state in
    mask >? 0 in
  let next (state : State) : State :=
    let '(mask, X, Y, ZZ, ZZZ, _y2, _zzz2) := state in
    let mask := Pure.shr 1 mask in
    let state := (mask, X, Y, ZZ, ZZZ, _y2, _zzz2) in
    state in
  let body (state : State) : State :=
    let '(mask, X, Y, ZZ, ZZZ, _y2, _zzz2) := state in
    (*
    {
    //X,Y,ZZ,ZZZ:=ecDblNeg(X,Y,ZZ,ZZZ), not having it inplace increase by 12K the cost of the function

    ...

    }
    *)
    let '{|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ
    |} := ecDblNeg Q.(Q.a) _modulusp {| PZZ.X := X; PZZ.Y := Y; PZZ.ZZ := ZZ; PZZ.ZZZ := ZZZ |} in
    (*
    let T1:=add(add(sub(1,iszero(and(scalar_u, mask))), shl(1,sub(1,iszero(and(shr(128, scalar_u), mask))))),
      add(shl(2,sub(1,iszero(and(scalar_v, mask)))), shl(3,sub(1,iszero(and(shr(128, scalar_v), mask))))))
    *)
    let s := Pure.add
      (Pure.add
        (Pure.sub 1 (Pure.iszero (Pure.and scalar_u mask)))
        (Pure.shl 1 (Pure.sub 1 (Pure.iszero (Pure.and (Pure.shr 128 scalar_u) mask))))
      )
      (Pure.add
        (Pure.shl 2 (Pure.sub 1 (Pure.iszero (Pure.and scalar_v mask))))
        (Pure.shl 3 (Pure.sub 1 (Pure.iszero (Pure.and (Pure.shr 128 scalar_v) mask))))
      ) in
    (*
    if iszero(T1) {
                  Y := sub(_p, Y)
                  continue
    }
    *)
    if s =? 0 then
      let Y := Pure.sub _modulusp Y in
      let state := (mask, X, Y, ZZ, ZZZ, _y2, _zzz2) in
      state
    else
    (*
    T1:=shl(7, T1)//Commputed value address offset
    let T4:=mload(add(Mem,T1))//X2
    *)
    let T4 := (Ts.get Ts (Z.to_nat s)).(PZZ.X) in
    (* mstore(add(Mem,_y2), addmod(mulmod( mload(add(Mem,add(32,T1))), ZZZ, _p), mulmod(Y,mload(add(Mem, _zzz2)), _p), _p))//R=S2-S1, sub avoided *)
    let _y2 := Pure.addmod
      (Pure.mulmod (Ts.get Ts (Z.to_nat s)).(PZZ.Y) ZZZ _modulusp)
      (Pure.mulmod Y _zzz2 _modulusp)
      _modulusp in
    (* T1:=mload(add(Mem,add(64,T1)))//zz2 *)
    let T1 := (Ts.get Ts (Z.to_nat s)).(PZZ.ZZ) in
    (* let T2 := addmod(mulmod(T4, ZZ, _p), sub(_p, mulmod(X,T1,_p)), _p)//P=U2-U1 *)
    let T2 := Pure.addmod
      (Pure.mulmod T4 ZZ _modulusp)
      (Pure.sub _modulusp (Pure.mulmod X T1 _modulusp))
      _modulusp in
    (*
    //special case ecAdd(P,P)=EcDbl
    if iszero(mload(add(Mem,_y2))) {
        if iszero(T2) {
            T1 := mulmod(sub(_p,2), Y, _p) //U = 2*Y1, y free
            T2 := mulmod(T1, T1, _p) // V=U^2
            mstore(add(Mem,_y2), mulmod(X, T2, _p)) // S = X1*V

            T1 := mulmod(T1, T2, _p) // W=UV
            T4:=mulmod(mload(add(Q,_a)),mulmod(ZZ,ZZ,_p),_p)
            T4 := addmod(mulmod(3, mulmod(X,X,_p),_p),T4,_p)//M=3*X12+aZZ12   //M

            ZZZ := mulmod(T1, ZZZ, _p) //zzz3=W*zzz1
            ZZ := mulmod(T2, ZZ, _p) //zz3=V*ZZ1, V free

            X := addmod(mulmod(T4, T4, _p), mulmod(sub(_p,2), mload(add(Mem, _y2)), _p), _p) //X3=M^2-2S
            T2 := mulmod(T4, addmod(mload(add(Mem, _y2)), sub(_p, X), _p), _p) //M(S-X3)

            Y := addmod(T2, mulmod(T1, Y, _p), _p) //Y3= M(S-X3)-W*Y1

            continue
        }
    }
    *)
    if (_y2 =? 0) && (T2 =? 0) then
      let T1 := Pure.mulmod (Pure.sub _modulusp 2) Y _modulusp in
      let T2 := Pure.mulmod T1 T1 _modulusp in
      let _y2 := Pure.mulmod X T2 _modulusp in
      let T1 := Pure.mulmod T1 T2 _modulusp in
      let T4 := Pure.mulmod Q.(Q.a) (Pure.mulmod ZZ ZZ _modulusp) _modulusp in
      let T4 := Pure.addmod (Pure.mulmod 3 (Pure.mulmod X X _modulusp) _modulusp) T4 _modulusp in
      let ZZZ := Pure.mulmod T1 ZZZ _modulusp in
      let ZZ := Pure.mulmod T2 ZZ _modulusp in
      let X := Pure.addmod
        (Pure.mulmod T4 T4 _modulusp)
        (Pure.mulmod (Pure.sub _modulusp 2) _y2 _modulusp)
        _modulusp in
      let T2 := Pure.mulmod T4 (Pure.addmod _y2 (Pure.sub _modulusp X) _modulusp) _modulusp in
      let Y := Pure.addmod T2 (Pure.mulmod T1 Y _modulusp) _modulusp in
      let state := (mask, X, Y, ZZ, ZZZ, _y2, _zzz2) in
      state
    else
    (*
    T4 := mulmod(T2, T2, _p) //PP
    T2 := mulmod(T4, T2, _p) //PPP
    ZZ := mulmod(mulmod(ZZ, T4,_p), T1 ,_p)//zz3=zz1*zz2*PP
    T1:= mulmod(X,T1, _p)
    ZZZ := mulmod(mulmod(ZZZ, T2, _p), mload(add(Mem, _zzz2)),_p) // zzz3=zzz1*zzz2*PPP
    X := addmod(addmod(mulmod(mload(add(Mem, _y2)), mload(add(Mem, _y2)), _p), sub(_p, T2), _p), mulmod( T1 ,mulmod(sub(_p,2), T4, _p),_p ), _p)// R2-PPP-2*U1*PP
    T4 := mulmod(T1, T4, _p)///Q=U1*PP
    Y := addmod(mulmod(addmod(T4, sub(_p, X), _p), mload(add(Mem, _y2)), _p), mulmod(mulmod(Y,mload(add(Mem, _zzz2)), _p), T2, _p), _p)// R*(Q-X3)-S1*PPP
    *)
    let T4 := Pure.mulmod T2 T2 _modulusp in
    let T2 := Pure.mulmod T4 T2 _modulusp in
    let ZZ := Pure.mulmod (Pure.mulmod ZZ T4 _modulusp) T1 _modulusp in
    let T1 := Pure.mulmod X T1 _modulusp in
    let ZZZ := Pure.mulmod (Pure.mulmod ZZZ T2 _modulusp) _zzz2 _modulusp in
    let X := Pure.addmod
      (Pure.addmod (Pure.mulmod _y2 _y2 _modulusp) (Pure.sub _modulusp T2) _modulusp)
      (Pure.mulmod T1 (Pure.mulmod (Pure.sub _modulusp 2) T4 _modulusp) _modulusp)
      _modulusp in
    let T4 := Pure.mulmod T1 T4 _modulusp in
    let Y := Pure.addmod
      (Pure.mulmod (Pure.addmod T4 (Pure.sub _modulusp X) _modulusp) _y2 _modulusp)
      (Pure.mulmod (Pure.mulmod Y _zzz2 _modulusp) T2 _modulusp)
      _modulusp in
    let state := (mask, X, Y, ZZ, ZZZ, _y2, _zzz2) in
    state in
  (*  }//endloop *)
  let _y2 := 0 in
  let _zzz2 := 0 in
  let state := {|
    MainLoop.State.mask := mask;
    MainLoop.State.P1 := {|
      PZZ.X := X;
      PZZ.Y := Y;
      PZZ.ZZ := ZZ;
      PZZ.ZZZ := ZZZ
    |};
    MainLoop.State._y2 := _y2;
    MainLoop.State._zzz2 := _zzz2
  |} in
  let 'MainLoop.State.Build_t mask P1 _y2 _zzz2 :=
    for_loop
      128
      state
      MainLoop.condition
      MainLoop.next
      (MainLoop.body Q.(Q.a) _modulusp scalar_u scalar_v Ts) in
  let '{| PZZ.X := X; PZZ.Y := Y; PZZ.ZZ := ZZ; PZZ.ZZZ := ZZZ |} := P1 in

  (*
  mstore(0x40, _free)
  let T := mload(0x40)
  mstore(add(T, 0x60), ZZ)
  //(X,Y)=ecZZ_SetAff(X,Y,zz, zzz);
  //T[0] = inverseModp_Hard(T[0], p); //1/zzz, inline modular inversion using Memmpile:
  // Define length of base, exponent and modulus. 0x20 == 32 bytes
  mstore(T, 0x20)
  mstore(add(T, 0x20), 0x20)
  mstore(add(T, 0x40), 0x20)
  // Define variables base, exponent and modulus
  //mstore(add(pointer, 0x60), u)
  mstore(add(T, 0x80), sub(_p,2))
  mstore(add(T, 0xa0), _p)
  *)
  let T := (* expmod ~(| ZZ, _modulusp, sub ~(| _modulusp, 2 |) |) *) 0 in
  let X := Pure.mulmod X T _modulusp in

  X.
