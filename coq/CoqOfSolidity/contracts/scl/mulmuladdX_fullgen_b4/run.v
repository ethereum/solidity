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

  (** We make explicit the list of points that we add to be sure to be in the same order as in the
    source code. *)
  Definition get_points (p : U256.t) (Q Q' G G' : PA.t) (selector : PointsSelector.t) :
      list PA.t :=
    let '{|
      u_low := u_low;
      u_high := u_high;
      v_low := v_low;
      v_high := v_high
    |} := selector in
    match u_low, u_high, v_low, v_high with
    | false, false, false, false => []
    | true, false, false, false => [G]
    | false, true, false, false => [G']
    | true, true, false, false => [G'; G]
    | false, false, true, false => [Q]
    | true, false, true, false => [Q; G]
    | false, true, true, false => [G'; Q]
    | true, true, true, false => [G'; Q; G]
    | false, false, false, true => [Q']
    | true, false, false, true => [Q'; G]
    | false, true, false, true => [Q'; G']
    | true, true, false, true => [Q'; G'; G]
    | false, false, true, true => [Q; Q']
    | true, false, true, true => [Q; Q'; G]
    | false, true, true, true => [G'; Q; Q']
    | true, true, true, true => [G'; Q; Q'; G]
    end.

  (** Add all the elements in the list or return zero, without ever adding zero itself as the
      operation is not defined for it. *)
  Fixpoint add_points (p : U256.t) (Qs : list PA.t) : PZZ.t :=
    match Qs with
    | [] => PZZ.zero
    | [Q] => PZZ.of_PA Q
    | Q :: Qs => ecAddn2 p (add_points p Qs) Q
    end.

  Definition get_point (p : U256.t) (Q Q' G G' : PA.t) (selector : PointsSelector.t) : PZZ.t :=
    add_points p (List.rev (get_points p Q Q' G G' selector)).
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
    induction index; cbn; hauto lq: on.
  Qed.
End MostSignificantBit.

Ltac load_store_line :=
  with_strategy opaque [ecAddn2 Z.pow PointsSelector.get_point HighLow.get_selector] (
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

Lemma run_get_point_coordinate codes environment state
    (
      mem0 mem1 mem2 mem3 mem4 mem5 mem6 mem7 mem8 mem9
      mem10 mem11 mem12 mem13 mem14 mem15 mem16 mem17 mem18 mem19
      mem20 mem21 mem22 mem23 mem24 mem25 mem26 mem27 mem28 mem29
      mem30 mem31 mem32 mem33 mem34 mem35 mem36 mem37 mem38 mem39
      mem40 mem41 mem42 mem43 mem44 mem45 mem46 mem47 mem48 mem49
      mem50 mem51 mem52 mem53 mem54 mem55 mem56 mem57 mem58 mem59
      mem60 mem61 mem62 mem63
      : U256.t
    )
    (memory_suffix : list U256.t)
    (Q Q' G G' : PA.t) (p : Z)
    (selector : PointsSelector.t)
    (start_state' : State.t) :
  let memory_prefix : list U256.t := [
    mem0; mem1; mem2; mem3; mem4; mem5; mem6; mem7; mem8; mem9;
    mem10; mem11; mem12; mem13; mem14; mem15; mem16; mem17; mem18; mem19;
    mem20; mem21; mem22; mem23; mem24; mem25; mem26; mem27; mem28; mem29;
    mem30; mem31; mem32; mem33; mem34; mem35; mem36; mem37; mem38; mem39;
    mem40; mem41; mem42; mem43; mem44; mem45; mem46; mem47; mem48; mem49;
    mem50; mem51; mem52; mem53; mem54; mem55; mem56; mem57; mem58; mem59;
    mem60; mem61; mem62; mem63
  ] in
  let P0 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false false false false) in
  let P1 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true false false false) in
  let P2 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false true false false) in
  let P3 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true true false false) in
  let P4 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false false true false) in
  let P5 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true false true false) in
  let P6 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false true true false) in
  let P7 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true true true false) in
  let P8 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false false false true) in
  let P9 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true false false true) in
  let P10 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false true false true) in
  let P11 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true true false true) in
  let P12 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false false true true) in
  let P13 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true false true true) in
  let P14 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false true true true) in
  let P15 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true true true true) in
  let memory : list U256.t :=
    memory_prefix ++
    [P0.(PZZ.X); P0.(PZZ.Y); P0.(PZZ.ZZ); P0.(PZZ.ZZZ)] ++
    [P1.(PZZ.X); P1.(PZZ.Y); P1.(PZZ.ZZ); P1.(PZZ.ZZZ)] ++
    [P2.(PZZ.X); P2.(PZZ.Y); P2.(PZZ.ZZ); P2.(PZZ.ZZZ)] ++
    [P3.(PZZ.X); P3.(PZZ.Y); P3.(PZZ.ZZ); P3.(PZZ.ZZZ)] ++
    [P4.(PZZ.X); P4.(PZZ.Y); P4.(PZZ.ZZ); P4.(PZZ.ZZZ)] ++
    [P5.(PZZ.X); P5.(PZZ.Y); P5.(PZZ.ZZ); P5.(PZZ.ZZZ)] ++
    [P6.(PZZ.X); P6.(PZZ.Y); P6.(PZZ.ZZ); P6.(PZZ.ZZZ)] ++
    [P7.(PZZ.X); P7.(PZZ.Y); P7.(PZZ.ZZ); P7.(PZZ.ZZZ)] ++
    [P8.(PZZ.X); P8.(PZZ.Y); P8.(PZZ.ZZ); P8.(PZZ.ZZZ)] ++
    [P9.(PZZ.X); P9.(PZZ.Y); P9.(PZZ.ZZ); P9.(PZZ.ZZZ)] ++
    [P10.(PZZ.X); P10.(PZZ.Y); P10.(PZZ.ZZ); P10.(PZZ.ZZZ)] ++
    [P11.(PZZ.X); P11.(PZZ.Y); P11.(PZZ.ZZ); P11.(PZZ.ZZZ)] ++
    [P12.(PZZ.X); P12.(PZZ.Y); P12.(PZZ.ZZ); P12.(PZZ.ZZZ)] ++
    [P13.(PZZ.X); P13.(PZZ.Y); P13.(PZZ.ZZ); P13.(PZZ.ZZZ)] ++
    [P14.(PZZ.X); P14.(PZZ.Y); P14.(PZZ.ZZ); P14.(PZZ.ZZZ)] ++
    [P15.(PZZ.X); P15.(PZZ.Y); P15.(PZZ.ZZ); P15.(PZZ.ZZZ)] ++
    memory_suffix in
  let start_state := make_state environment state memory [] in
  start_state' = start_state ->
  {{? codes, environment, Some start_state' |
    mload (Pure.add (Pure.add 0 2048) (Pure.shl 7 (PointsSelector.to_Z selector))) ⇓
    Result.Ok (PointsSelector.get_point p Q Q' G G' selector).(PZZ.X)
  | Some start_state' ?}} /\
  {{? codes, environment, Some start_state' |
    mload (Pure.add (Pure.add (Pure.add 0 2048) (Pure.shl 7 (PointsSelector.to_Z selector))) 32) ⇓
    Result.Ok (PointsSelector.get_point p Q Q' G G' selector).(PZZ.Y)
  | Some start_state' ?}} /\
  {{? codes, environment, Some start_state' |
    mload (Pure.add (Pure.add (Pure.add 0 2048) (Pure.shl 7 (PointsSelector.to_Z selector))) 64) ⇓
    Result.Ok (PointsSelector.get_point p Q Q' G G' selector).(PZZ.ZZ)
  | Some start_state' ?}} /\
  {{? codes, environment, Some start_state' |
    mload (Pure.add (Pure.add (Pure.add 0 2048) (Pure.shl 7 (PointsSelector.to_Z selector))) 96) ⇓
    Result.Ok (PointsSelector.get_point p Q Q' G G' selector).(PZZ.ZZZ)
  | Some start_state' ?}}.
Proof.
  intros * H_state.
  rewrite H_state.
  unfold start_state.
  repeat split;
    destruct selector as [u_low u_high v_low v_high];
    destruct u_low, u_high, v_low, v_high;
    apply_run_mload.
Qed.

Lemma run_fun_ecGenMulmuladdX_store_2814 codes environment state
    (
      mem0 mem1 mem3 mem4 mem5 mem6 mem7 mem8
      mem12 mem13 mem14 :
      U256.t
    )
    (Q Q' G G' : PA.t) (p a : Z) (u_low u_high v_low v_high : U256.t)
    (H_u_low : U128.Valid.t u_low)
    (H_u_high : U128.Valid.t u_high)
    (H_v_low : U128.Valid.t v_low)
    (H_v_high : U128.Valid.t v_high) :
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
      (* We flatten the additions *)
      repeat match goal with
      | _ := ?t : PZZ.t |- _ =>
        let t' := eval cbv - [ecAddn2 PZZ.of_PA] in t in
        progress change t with t' in * |-
      end.
      (* We show that the memory is equal to the description based on selectors *)
      set (P0 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false false false false)).
      set (P1 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true false false false)).
      set (P2 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false true false false)).
      set (P3 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true true false false)).
      set (P4 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false false true false)).
      set (P5 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true false true false)).
      set (P6 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false true true false)).
      set (P7 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true true true false)).
      set (P8 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false false false true)).
      set (P9 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true false false true)).
      set (P10 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false true false true)).
      set (P11 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true true false true)).
      set (P12 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false false true true)).
      set (P13 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true false true true)).
      set (P14 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t false true true true)).
      set (P15 := PointsSelector.get_point p Q Q' G G' (PointsSelector.Build_t true true true true)).
      apply_memory_update_at (2048 + 0 * 32) P0.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 1 * 32) P0.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 2 * 32) P0.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 3 * 32) P0.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 4 * 32) P1.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 5 * 32) P1.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 6 * 32) P1.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 7 * 32) P1.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 8 * 32) P2.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 9 * 32) P2.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 10 * 32) P2.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 11 * 32) P2.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 12 * 32) P3.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 13 * 32) P3.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 14 * 32) P3.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 15 * 32) P3.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 16 * 32) P4.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 17 * 32) P4.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 18 * 32) P4.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 19 * 32) P4.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 20 * 32) P5.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 21 * 32) P5.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 22 * 32) P5.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 23 * 32) P5.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 24 * 32) P6.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 25 * 32) P6.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 26 * 32) P6.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 27 * 32) P6.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 28 * 32) P7.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 29 * 32) P7.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 30 * 32) P7.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 31 * 32) P7.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 32 * 32) P8.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 33 * 32) P8.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 34 * 32) P8.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 35 * 32) P8.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 36 * 32) P9.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 37 * 32) P9.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 38 * 32) P9.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 39 * 32) P9.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 40 * 32) P10.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 41 * 32) P10.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 42 * 32) P10.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 43 * 32) P10.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 44 * 32) P11.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 45 * 32) P11.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 46 * 32) P11.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 47 * 32) P11.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 48 * 32) P12.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 49 * 32) P12.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 50 * 32) P12.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 51 * 32) P12.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 52 * 32) P13.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 53 * 32) P13.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 54 * 32) P13.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 55 * 32) P13.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 56 * 32) P14.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 57 * 32) P14.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 58 * 32) P14.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 59 * 32) P14.(PZZ.ZZZ); [reflexivity|].
      apply_memory_update_at (2048 + 60 * 32) P15.(PZZ.X); [reflexivity|].
      apply_memory_update_at (2048 + 61 * 32) P15.(PZZ.Y); [reflexivity|].
      apply_memory_update_at (2048 + 62 * 32) P15.(PZZ.ZZ); [reflexivity|].
      apply_memory_update_at (2048 + 63 * 32) P15.(PZZ.ZZZ); [reflexivity|].
      (* Remove unused definitions *)
      repeat match reverse goal with
      | t := _ : PZZ.t |- _ => clear t
      end.
      set (mask_address := 0x01a0).
      set (ZZZ_address := 0xe0).
      (* Computation of the most-significant bit *)
      l. {
        eapply LoopOneStepUnsafe with (default_output := Result.Ok (BlockUnit.Tt, tt)). {
          load_store_line.
          l. {
            load_store_line.
          }
          load_store_line.
        }
        apply_memory_update_at mask_address (2 ^ 126). {
          reflexivity.
        }
        apply_memory_update_at ZZZ_address (HighLow.raw_get_selector u v (2 ^ 127)). {
          reflexivity.
        }
        p.
      }
      repeat load_store_line.
      unfold u, v.
      Ltac run_load_coordinate :=
        l; [
          try (c; [p|]; with_strategy opaque [Z.pow] s);
          c; [
            rewrite <- HighLow.get_selector_eq by (try assumption; lia);
            eapply run_get_point_coordinate; reflexivity
          |];
          c; [
            apply_run_mstore
          |];
          CanonizeState.execute;
          p
        |].
      do 4 run_load_coordinate.
      load_store_line.
      set (P_127 := PointsSelector.get_point p Q Q' G G' _) in |- *.
      (* Main for loop *)
      l. {
        eapply LoopOneStepUnsafe with (default_output := Result.Ok (BlockUnit.Tt, tt)). {
          load_store_line.
          l. {
            set (P_127_X_address := 5 * 32).
            set (P_127_Y_address := 8 * 32).
            set (P_127_ZZ_address := 14 * 32).
            set (P_127_ZZZ_address := 7 * 32).
            load_store_line.
            load_store_line.
            load_store_line.
            load_store_line.
            load_store_line.
            load_store_line.
            set (dbl_neg_P_127 := ecDblNeg a p P_127).
            (* This special optimization is made by the Solidity compiler *)
            replace (Pure.add p (Pure.not 1)) with (Pure.sub p 2). 2: {
              unfold Pure.add, Pure.sub, Pure.not.
              lia.
            }
            apply_memory_update_at P_127_X_address dbl_neg_P_127.(PZZ.X); [reflexivity|].
            apply_memory_update_at P_127_Y_address dbl_neg_P_127.(PZZ.Y); [reflexivity|].
            apply_memory_update_at P_127_ZZ_address dbl_neg_P_127.(PZZ.ZZ); [reflexivity|].
            apply_memory_update_at P_127_ZZZ_address dbl_neg_P_127.(PZZ.ZZZ); [reflexivity|].
            l. {
              repeat (c; [
                apply_run_mload ||
                p
              |]).
              change (Result.Ok _) with (Result.Ok (HighLow.raw_get_selector u v (2 ^ 126))).
              unfold u, v.
              rewrite <- HighLow.get_selector_eq by (try assumption; lia).
              p.
            }
            (* We flatten the control flow because there is going to be an `if` with a `continue` *)
            lu.
            c; [p|].
            with_strategy opaque [Z.pow] s.
            match goal with
            | |- context[Shallow.if_ ?condition] =>
              let condition' := eval unfold Pure.iszero in condition in
              change condition with condition'
            end.
            destruct (PointsSelector.to_Z _ =? 0) eqn:H_is_selector_zero.
            { (* The selector is zero *)
              load_store_line.
            }
            { (* The selector is not zero *)
              l. {
                repeat (c; [
                  apply_run_mload ||
                  p
                |]).
                with_strategy opaque [Z.pow] s.
                c. {
                  eapply run_get_point_coordinate; reflexivity.
                }
                p.
              }
              set (P_126 := PointsSelector.get_point p Q Q' G G' _) in |- *.
              load_store_line.
              l. {
                c. {
                  eapply run_get_point_coordinate; reflexivity.
                }
                p.
              }
              fold P_126.
              load_store_line.
              (* Flattening the control flow to anticipate an `if` *)
              lu.
              repeat (c; [
                apply_run_mload ||
                p
              |]).
              fold @LowM.let_.
              unfold Pure.iszero at 1.
              destruct (dbl_neg_P_127.(PZZ.ZZ) =? 0).
              { (* Case is zero *)
                load_store_line.
                l. {
                  repeat (c; [
                    apply_run_mload ||
                    p
                  |]).
                  s.
                  c. {
                    eapply run_get_point_coordinate; reflexivity.
                  }
                  c. {
                    apply_run_mstore.
                  }
                  CanonizeState.execute.
                  p.
                }
                l. {
                  repeat (c; [
                    apply_run_mload ||
                    p
                  |]).
                  s.
                  c. {
                    eapply run_get_point_coordinate; reflexivity.
                  }
                  c. {
                    apply_run_mstore.
                  }
                  CanonizeState.execute.
                  p.
                }
                l. {
                  repeat (c; [
                    apply_run_mload ||
                    p
                  |]).
                  s.
                  c. {
                    eapply run_get_point_coordinate; reflexivity.
                  }
                  c. {
                    apply_run_mstore.
                  }
                  CanonizeState.execute.
                  p.
                }
                fold @LowM.let_.
                (* This is the end of a control-flow branch *)
                admit.
              }
              { (* Case is not zero *)
                l. {
                  repeat (c; [
                    apply_run_mload ||
                    p
                  |]).
                  s.
                  c. {
                    eapply run_get_point_coordinate; reflexivity.
                  }
                  fold P_126.
                  repeat (c; [
                    apply_run_mload ||
                    p
                  |]).
                  p.
                }
                l. {
                  repeat (c; [
                    apply_run_mload ||
                    p
                  |]).
                  c. {
                    apply_run_mstore.
                  }
                  CanonizeState.execute.
                  p.
                }
                l. {
                  repeat (c; [
                    apply_run_mload ||
                    p
                  |]).
                  c. {
                    eapply run_get_point_coordinate; reflexivity.
                  }
                  fold P_126.
                  p.
                }
                load_store_line.
                (* `if continue` *)
                lu.
                c. {
                  p.
                }
                fold @LowM.let_.
                unfold Pure.iszero at 1.
                match goal with
                | |- context[if ?condition then _ else _] => destruct condition
                end.
                { lu.
                  c. {
                    p.
                  }
                  fold @LowM.let_.
                  unfold Pure.iszero at 1.
                  match goal with
                  | |- context[if ?condition then _ else _] => destruct condition
                  end.
                  { load_store_line.
                    load_store_line.
                    load_store_line.
                    (* Too slow, we should factorize the expressions *)
                    admit.
                  }
                  { load_store_line.
                    load_store_line.
                    load_store_line.
                    (* Too slow, we should factorize the expressions *)
                    admit.

                  }
                }
              }

              Ltac cbn_goal :=
                match goal with
                | |- {{? _, _, _ | ?expr1 ⇓ ?expr2 | ?state_end ?}} =>
                  let expr1' := eval cbn in expr1 in
                  change expr1 with expr1';
                  let expr2' := eval cbn in expr2 in
                  change expr2 with expr2';
                  let state_end' := eval cbn in state_end in
                  change state_end with state_end'
                end.
              cbn in P_126.
              
              
              (* with_strategy opaque [Z.pow] s. *)
                run_load_coordinate.
                match goal with
                | |- context[shl 7 ?selector] =>
                  replace selector with (PointsSelector.to_Z (HighLow.get_selector u_low u_high v_low v_high 126))
                end. 2: {
                  apply HighLow.get_selector_eq; try assumption; lia.
                }
                end.
              }
              
            unfold Pure.iszero at 1.
            match goal with
            | |- context[if ?condition then _ else _] => destruct condition eqn:H_is_selector_zero
            end.
            { (* The selector is zero *)
            }
            fold HighLow.raw_get_selector.
            rewrite <- HighLow.get_selector_eq.
            (* Next line: let selector *)
            load_store_line.
            load_store_line.
            load_store_line.
          }
        }
      }
Qed.
