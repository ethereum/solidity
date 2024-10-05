Require Import CoqOfSolidity.CoqOfSolidity.
Require Import CoqOfSolidity.proofs.CoqOfSolidity.
Require Import CoqOfSolidity.simulations.CoqOfSolidity.
Require Import CoqOfSolidity.contracts.scl.mulmuladdX_fullgen_b4.contract_shallow.
Import Stdlib.
Import RunO.

Ltac Zify.zify_post_hook ::= Z.to_euclidean_division_equations.

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

Module Curve.
  Record t : Set := {
    a : Z;
    b : Z;
  }.
End Curve.

Module Zp.
  Module Valid.
    Definition t (p n : Z) : Prop :=
      0 <= n < p.
  End Valid.
End Zp.

(** Affine points, excluding zero *)
Module PA.
  Record t : Set := {
    X : U256.t;
    Y : U256.t;
  }.

  Module Valid.
    Record t (p : U256.t) (P : PA.t) : Prop := {
      X : Zp.Valid.t p P.(PA.X);
      Y : Zp.Valid.t p P.(PA.Y);
    }.
  End Valid.

  Definition add (a p : Z) (P1 P2 : t) : option t :=
    (* opposite *)
    if (P1.(X) =? P2.(X)) && (P1.(Y) =? - P2.(Y)) then
      None
    (* double *)
    else if (P1.(X) =? P2.(X)) && (P1.(Y) =? P2.(Y)) then
      let lambda := (3 * P1.(X) ^ 2 + a) / (2 * P1.(Y)) in
      let x := (lambda ^ 2 - 2 * P1.(X)) mod p in
      Some {|
        X := x;
        Y := (lambda * (P1.(X) - x) - P1.(Y)) mod p;
      |}
    (* add *)
    else
      let lambda := (P2.(Y) - P1.(Y)) / (P2.(X) - P1.(X)) in
      let x := (lambda ^ 2 - P1.(X) - P2.(X)) mod p in
      Some {|
        X := x;
        Y := (lambda * (P1.(X) - x) - P1.(Y)) mod p;
      |}.
End PA.

(** Points, with a dummy value for the zero *)
Module P.
  Definition t : Set :=
    option PA.t.

  Module Valid.
    Definition t (p : U256.t) (P : P.t) : Prop :=
      match P with
      | None => True
      | Some P => PA.Valid.t p P
      end.
  End Valid.

  Definition zero : P.t :=
    None.

  Definition add (a p : Z) (P1 P2 : t) : t :=
    match P1, P2 with
    | None, P | P, None => P
    | Some P1, Some P2 => PA.add a p P1 P2
    end.

  Lemma add_zero_l (a p : U256.t) (P : t) :
    add a p zero P = P.
  Proof.
    reflexivity.
  Qed.

  Lemma add_zero_r (a p : U256.t) (P : t) :
    add a p P zero = P.
  Proof.
    destruct P; reflexivity.
  Qed.

  Axiom add_commut :
    forall (a p : U256.t) (P1 P2 : t),
    add a p P1 P2 =
    add a p P2 P1.

  Axiom add_commut_nested :
    forall (a p : U256.t) (P1 P2 P3 : t),
    add a p (add a p P1 P2) P3 =
    add a p (add a p P1 P3) P2.
End P.

(** Points in the ZZ, ZZZ representation *)
Module PZZ.
  Record t : Set := {
    X : U256.t;
    Y : U256.t;
    ZZ : U256.t;
    ZZZ : U256.t;
  }.

  Module Valid.
    Record t (p : U256.t) (P : PZZ.t) : Prop := {
      X : Zp.Valid.t p P.(PZZ.X);
      Y : Zp.Valid.t p P.(PZZ.Y);
      ZZ : Zp.Valid.t p P.(PZZ.ZZ);
      ZZZ : Zp.Valid.t p P.(PZZ.ZZZ);
      zero : P.(PZZ.ZZ) = 0 <-> P.(PZZ.ZZZ) = 0;
    }.
  End Valid.

  Definition zero : PZZ.t := {|
    PZZ.X := 0;
    PZZ.Y := 0;
    PZZ.ZZ := 0;
    PZZ.ZZZ := 0;
  |}.

  Lemma zero_is_valid (p : U256.t)
      (H_p : 2 <= p) :
    Valid.t p zero.
  Proof.
    constructor; unfold Zp.Valid.t; simpl; lia.
  Qed.

  Definition of_PA (P : PA.t) : PZZ.t := {|
    PZZ.X := P.(PA.X);
    PZZ.Y := P.(PA.Y);
    PZZ.ZZ := 1;
    PZZ.ZZZ := 1;
  |}.

  Lemma of_PA_is_valid (p : U256.t) (P : PA.t)
      (H_p : 2 <= p)
      (H_P : PA.Valid.t p P) :
    Valid.t p (of_PA P).
  Proof.
    destruct H_P; unfold Zp.Valid.t in *.
    unfold of_PA.
    constructor; unfold Zp.Valid.t; simpl; lia.
  Qed.

  Definition of_P (P : P.t) : PZZ.t :=
    match P with
    | None => zero
    | Some P => of_PA P
    end.

  Lemma of_P_is_valid (p : U256.t) (P : P.t)
      (H_p : 2 <= p < 2^256)
      (H_P : P.Valid.t p P) :
    Valid.t p (of_P P).
  Proof.
    destruct P as [P|].
    { now apply of_PA_is_valid. }
    { now apply zero_is_valid. }
  Qed.

  (** We do not need to check for [ZZZ] as both should be zero at the same time. *)
  Definition is_zero (P : PZZ.t) : bool :=
    P.(ZZ) =? 0.

  Definition to_P (p : Z) (P : PZZ.t) : P.t :=
    if is_zero P then
      None
    else
      Some {|
        PA.X := (P.(X) / P.(ZZ)) mod p;
        PA.Y := (P.(Y) / P.(ZZZ)) mod p;
      |}.

  Lemma to_P_is_valid (p : U256.t) (P : PZZ.t)
      (H_p : 2 <= p) :
    P.Valid.t p (to_P p P).
  Proof.
    unfold P.Valid.t, to_P, is_zero; simpl.
    destruct (ZZ P =? 0) eqn:H_ZZ_eq; simpl; [exact I|].
    constructor; unfold Zp.Valid.t; simpl; lia.
  Qed.

  Lemma to_P_of_PA_eq (p : U256.t) (P : PA.t)
      (H_P : PA.Valid.t p P) :
    to_P p (of_PA P) = Some P.
  Proof.
    unfold to_P, of_PA; simpl.
    destruct H_P; unfold Zp.Valid.t in *.
    destruct P; simpl in *.
    repeat f_equal.
    all: rewrite Z.mod_small; lia.
  Qed.

  Lemma to_P_of_P_eq (p : U256.t) (P : P.t)
      (H_P : P.Valid.t p P) :
    to_P p (of_P P) = P.
  Proof.
    destruct P as [P|].
    { apply to_P_of_PA_eq, H_P. }
    { reflexivity. }
  Qed.
End PZZ.

Definition ecAddn2 (P1 : PZZ.t) (P2 : PA.t) (p : U256.t) : PZZ.t :=
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
    ecAddn2
      {| PZZ.X := P1_X; PZZ.Y := P1_Y; PZZ.ZZ := P1_ZZ; PZZ.ZZZ := P1_ZZZ |}
      {| PA.X := P2_X; PA.Y := P2_Y |}
      p in
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
    ecAddn2
      {| PZZ.X := P1_X; PZZ.Y := P1_Y; PZZ.ZZ := 1; PZZ.ZZZ := 1 |}
      {| PA.X := P2_X; PA.Y := P2_Y |}
      p in
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

  Fixpoint get (Ts : t) (index : nat) : PZZ.t :=
    match Ts with
    | nil => PZZ.zero
    | T :: Ts =>
      match index with
      | O => T
      | S index => get Ts index
      end
    end.
End Ts.

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

Definition get_s u v mask :=
  Pure.add
    (Pure.add
      (Pure.sub 1 (Pure.iszero (Pure.and u mask)))
      (Pure.shl 1 (Pure.sub 1 (Pure.iszero (Pure.and (Pure.shr 128 u) mask))))
    )
    (Pure.add
      (Pure.shl 2 (Pure.sub 1 (Pure.iszero (Pure.and v mask))))
      (Pure.shl 3 (Pure.sub 1 (Pure.iszero (Pure.and (Pure.shr 128 v) mask))))
    ).

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
    let s := get_s u v mask in
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

Lemma run_fun_ecGenMulmuladdX_store_2814_beginning codes environment state
    (
      mem0 mem1 mem3 mem4 mem5 mem6 mem7 mem8
      mem12 mem13 mem14 :
      U256.t
    )
    (Q : Q.t) (scalar_u scalar_v : U256.t) :
  let memoryguard : U256.t := 0 in
  let location_Q : U256.t := 32 * 15 in
  let memory_start : list U256.t :=
    [
      mem0; mem1; memoryguard; mem3; mem4; mem5; mem6; mem7; mem8; scalar_u;
      location_Q; scalar_v; mem12; mem13; mem14;
      Q.(Q.Qx);
      Q.(Q.Qy);
      Q.(Q.Q'x);
      Q.(Q.Q'y);
      Q.(Q.p);
      Q.(Q.a);
      Q.(Q.gx);
      Q.(Q.gy);
      Q.(Q.gx2pow128);
      Q.(Q.gy2pow128)
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
  simpl.
  unfold Contract_91.Contract_91_deployed.fun_ecGenMulmuladdX_store_2814_beginning.
  l. {
    repeat load_store_line.
    (* l. {
      c. {
        apply_run_mstore.
      }
      CanonizeState.execute.
      p.
    }
    l. {
      cu; s.
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
      s.
      cu; p.
    } *)
    l. {
      unfold Shallow.if_, Pure.iszero.
      instantiate (2 := Result.Ok (BlockUnit.Tt, if scalar_u =? 0 then _ else _)).
      destruct (scalar_u =? 0); s; [|p].
      load_store_line.
      (* l. {
        c. {
          apply_run_mload.
        }
        s.
        cu; p.
      }
      s.
      p. *)
    }
    s.
    lu.
    match goal with
    | |- context [Shallow.if_ ?condition] =>
      replace condition with (Z.b2z ((scalar_u =? 0) && (scalar_v =? 0)))
    end.
    2: {
      unfold Pure.iszero.
      now repeat destruct (_ =? 0).
    }
    match goal with
    | |- context [Shallow.if_ (Z.b2z ?condition)] =>
      destruct condition; s
    end.
    { load_store_line. }
      (* p.
      lu; c. {
        apply_run_mstore.
      }
      p. *)
    (* } *)
    { repeat load_store_line.
      





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
