Require Import CoqOfSolidity.CoqOfSolidity.
Require Import CoqOfSolidity.simulations.CoqOfSolidity.
Import Stdlib.
Require Import CoqOfSolidity.contracts.scl.mulmuladdX_fullgen_b4.contract_shallow_by_hand.

(** Normalized addition of two point, must not be neutral input. *)
Definition high_ecAddn2 (P1 : PZZ.t) (P2 : PA.t) (_p : U256.t) : PZZ.t :=
  let x1 := P1.(PZZ.X) in
  let y1 := P1.(PZZ.Y) in
  let zz1 := P1.(PZZ.ZZ) in
  let zzz1 := P1.(PZZ.ZZZ) in

  let y1 := _p - y1 in
  let y2 := (P2.(PA.Y) * zzz1) + y1 in
  let x2 := (P2.(PA.X) * zz1) + (_p - x1) in
  let _x := x2 * x2 in
  let _y := _x * x2 in
  let _zz := zz1 * _x in

  let _zzz := zzz1 * _y in
  let zz1 := x1 * _x in
  let _x :=
    ((y2 * y2) + (_p - _y)) +
    ((_p - 2) * zz1) in

  let x1 := (zz1 + (_p - _x)) * y2 in
  let _y := x1 + (y1 * _y) in

  {|
    PZZ.X := _x mod _p;
    PZZ.Y := _y mod _p;
    PZZ.ZZ := _zz mod _p;
    PZZ.ZZZ := _zzz mod _p
  |}.

Ltac show_equality_modulo :=
  repeat (
    (
      (
        apply Zplus_eqm ||
        apply Zmult_eqm ||
        apply Zopp_eqm
      );
      unfold eqm
    ) ||
    rewrite Zmod_eqm ||
    reflexivity
  ).

Lemma ecAddn2_eq (P1 : PZZ.t) (P2 : PA.t) (p : Z)
    (H_P1 : PZZ.Valid.t p P1)
    (H_P2 : PA.Valid.t p P2)
    (H__p : 2 <= p < 2^256) :
  ecAddn2 P1 P2 p =
  high_ecAddn2 P1 P2 p.
Proof.
  destruct H_P1, H_P2.
  unfold Zp.Valid.t in *.
  destruct P1, P2; simpl in *.
  unfold
    ecAddn2,
    high_ecAddn2,
    Pure.addmod,
    Pure.mulmod,
    Pure.sub;
    simpl in *.
  assert (H_elim_sub_modulo :
      forall (a : Z),
      0 <= a <= p ->
      (p - a) mod Z.pow_pos 2 256 = p - a
    )
    by lia.
  repeat rewrite H_elim_sub_modulo by lia.
  f_equal.
  all: show_equality_modulo.
Qed.

Lemma high_ecAddn2_is_valid (P1 : PZZ.t) (P2 : PA.t) (_p : U256.t)
    (H_P1 : PZZ.Valid.t _p P1)
    (H_P2 : PA.Valid.t _p P2)
    (H__p : 2 <= _p < 2^256) :
  PZZ.Valid.t _p (high_ecAddn2 P1 P2 _p).
Proof.
  simpl.
  destruct H_P1; constructor.
  all: unfold Zp.Valid.t in *; simpl; try lia.
  admit.
Admitted.

Ltac cbn_without_arithmetic :=
  cbn - [Z.eqb Z.add Z.sub Z.mul Z.div Z.modulo Z.pow].

Lemma high_ecAddn2_is_add (a : Z) (P1 : PZZ.t) (P2 : PA.t) (p : U256.t)
    (H_P1 : PZZ.Valid.t p P1)
    (H_P2 : PA.Valid.t p P2)
    (H__p : 2 <= p < 2^256)
    (H_P1_not_zero : PZZ.is_zero P1 = false) :
  PZZ.to_P p (high_ecAddn2 P1 P2 p) =
  P.add a p (PZZ.to_P p P1) (Some P2).
Proof.
  simpl.
  destruct H_P1.
  unfold high_ecAddn2, PZZ.to_P, P.add, PA.add;
    cbn_without_arithmetic.
  unfold Zp.Valid.t, PZZ.is_zero in *.
  destruct (P1.(PZZ.ZZ) =? 0) eqn:H_P1_ZZ_eq; cbn_without_arithmetic. {
    replace P1.(PZZ.ZZ) with 0 by lia; simpl in *.
    congruence.
  }
  admit.
Admitted.

Axiom P_add_eq_high_ecAddn2 :
  forall (a p : Z) (P1 : PZZ.t) (P2 : PA.t),
  P.add a p (PZZ.to_P p P1) (Some P2) =
  PZZ.to_P p (high_ecAddn2 P1 P2 p).

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
End PointsSelector.

Definition Tss (p : U256.t) (P P128 Q Q128 : P.t) (selector : PointsSelector.t) : list P.t :=
  (if selector.(PointsSelector.u_low) then [P] else []) ++
  (if selector.(PointsSelector.u_high) then [P128] else []) ++
  (if selector.(PointsSelector.v_low) then [Q] else []) ++
  (if selector.(PointsSelector.v_high) then [Q128] else []).

Definition high_get_T (a p : U256.t) (P P128 Q Q128 : P.t) :
    PointsSelector.t -> P.t :=
  fun selector =>
  List.fold_left
    (fun P Q => P.add a p P Q)
    (Tss p P P128 Q Q128 selector)
    P.zero.

Lemma high_get_T_eq (a p : U256.t) (P P128 Q Q128 : PA.t)
    (H_P : PA.Valid.t p P)
    (H_P128 : PA.Valid.t p P128)
    (H_Q : PA.Valid.t p Q)
    (H_Q128 : PA.Valid.t p Q128)
    (selector : PointsSelector.t) :
  let params : Q.t := {|
    Q.Qx := Q.(PA.X);
    Q.Qy := Q.(PA.Y);
    Q.Q'x := Q128.(PA.X);
    Q.Q'y := Q128.(PA.Y);
    Q.p := p;
    Q.a := a;
    Q.gx := P.(PA.X);
    Q.gy := P.(PA.Y);
    Q.gx2pow128 := P128.(PA.X);
    Q.gy2pow128 := P128.(PA.Y);
  |} in
  let Ts := List.map (PZZ.to_P p) (get_Ts params) in
  let T := List.nth_error Ts (Z.to_nat (PointsSelector.to_Z selector)) in
  let high_T := high_get_T a p (Some P) (Some P128) (Some Q) (Some Q128) selector in
  T = Some high_T.
Proof.
  unfold PointsSelector.to_Z, Z.b2z.
  unfold high_get_T, Tss.
  unfold get_Ts; cbn - [ecAddn2 PZZ.to_P].
  repeat rewrite ecAddn2_eq by admit.
  repeat match goal with
  | |- context [?e] =>
    match e with
    | {| PZZ.X := ?P.(PA.X) |} =>
      change e with (PZZ.of_PA P)
    | {| PA.X := ?P.(PA.X) |} =>
      change e with P
    end
  end.
  repeat rewrite <- P_add_eq_high_ecAddn2 with (a := a).
  repeat rewrite PZZ.to_P_of_PA_eq by assumption.

  destruct selector; Tactics.destruct_all bool.
  all: unfold Z.to_nat, Pos.to_nat, List.nth_error; cbn - [P.add].
  all: f_equal; try reflexivity.
  all: try rewrite P.add_zero_l.

  all: repeat (
    rewrite P.add_commut with (P1 := Some P128) (P2 := Some P) ||
    rewrite P.add_commut with (P1 := Some Q)    (P2 := Some P) ||
    rewrite P.add_commut with (P1 := Some Q128) (P2 := Some P) ||
    rewrite P.add_commut with (P1 := Some Q)    (P2 := Some P128) ||
    rewrite P.add_commut with (P1 := Some Q128) (P2 := Some P128) ||
    rewrite P.add_commut with (P1 := Some Q128) (P2 := Some Q) ||
    rewrite P.add_commut_nested with (P2 := Some P128) (P3 := Some P) ||
    rewrite P.add_commut_nested with (P2 := Some Q)    (P3 := Some P) ||
    rewrite P.add_commut_nested with (P2 := Some Q128) (P3 := Some P) ||
    rewrite P.add_commut_nested with (P2 := Some Q)    (P3 := Some P128) ||
    rewrite P.add_commut_nested with (P2 := Some Q128) (P3 := Some P128) ||
    rewrite P.add_commut_nested with (P2 := Some Q128) (P3 := Some Q)
  ).
  all: reflexivity.
Admitted.

Definition high_get_s (u v : U256.t) (log_mask : Z) : PointsSelector.t := {|
  PointsSelector.u_low := Z.testbit u log_mask;
  PointsSelector.u_high := Z.testbit (u / (2 ^ 128)) log_mask;
  PointsSelector.v_low := Z.testbit v log_mask;
  PointsSelector.v_high := Z.testbit (v / (2 ^ 128)) log_mask;
|}.

Lemma get_s_eq (u v : U256.t) (log_mask : Z)
    (H_u : U256.Valid.t u)
    (H_v : U256.Valid.t v)
    (H_log_mask : 0 <= log_mask < 128) :
  get_s u v (2 ^ log_mask) =
  PointsSelector.to_Z (high_get_s u v log_mask).
Proof.
  unfold U256.Valid.t in *.
  unfold
    get_s,
    high_get_s,
    Pure.add,
    Pure.sub,
    Pure.iszero,
    Pure.and,
    Pure.shl,
    Pure.shr,
    PointsSelector.to_Z.
  repeat rewrite Arith2.Z_testbit_alt by lia.
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
  repeat destruct (0 <? _); reflexivity.
Qed.

Definition high_ecDblNeg (a p : Z) (P : PZZ.t) : PZZ.t :=
  let '{|
    PZZ.X := X;
    PZZ.Y := Y;
    PZZ.ZZ := ZZ;
    PZZ.ZZZ := ZZZ
  |} := P in

  let T1 := 2 * Y in
  let T2 := T1 * T1 in
  let T3 := X * T2 in
  let T1 := T1 * T2 in
  let T4 := a * (ZZ * ZZ) in
  let T4 := (3 * (X * X)) + T4 in
  let ZZZ := T1 * ZZZ in
  let ZZ := T2 * ZZ in
  let X := p - 2 in
  let X := (T4 * T4) + (X * T3) in
  let T2 := T4 * (X + (p - T3)) in
  let Y := (T1 * Y) + T2 in

  {|
    PZZ.X := X mod p;
    PZZ.Y := Y mod p;
    PZZ.ZZ := ZZ mod p;
    PZZ.ZZZ := ZZZ mod p
  |}.

Lemma high_ecDblNeg_eq (a p : Z) (P : PZZ.t)
    (H_p : 2 <= p < 2^256)
    (H_P : PZZ.Valid.t p P) :
  ecDblNeg a p P = high_ecDblNeg a p P.
Proof.
  destruct H_P.
  unfold ecDblNeg, high_ecDblNeg, Pure.addmod, Pure.mulmod, Pure.sub.
  unfold Zp.Valid.t in *.
  destruct P; cbn_without_arithmetic.
  simpl in * |-.
  assert (H_elim_sub_modulo :
      forall (a : Z),
      0 <= a <= p ->
      (p - a) mod (2 ^ 256) = p - a
    )
    by lia.
  repeat rewrite H_elim_sub_modulo by lia.
  f_equal.
  all: show_equality_modulo.
Qed.

Module MainLoop.
  Module State.
    Module Valid.
      Record t (p : Z) (state : MainLoop.State.t) : Prop := {
        mask_valid : 0 <= state.(MainLoop.State.mask) <= 127;
        P1_valid : PZZ.Valid.t p state.(MainLoop.State.P1);
        _y2_valid : U256.Valid.t state.(MainLoop.State._y2);
        _zzz2_valid : U256.Valid.t state.(MainLoop.State._zzz2);
      }.
    End Valid.
  End State.

  Definition high_next (state : MainLoop.State.t) : MainLoop.State.t :=
    let mask := state.(MainLoop.State.mask) / 2 in
    state <| MainLoop.State.mask := mask |>.

  Lemma high_next_eq (state : MainLoop.State.t) :
    MainLoop.next state = high_next state.
  Proof.
    reflexivity.
  Qed.

  Definition high_body (a p : Z) (u v : U256.t) (get_T : PointsSelector.t -> P.t)
      (state : MainLoop.State.t) : MainLoop.State.t :=
    let '{|
      MainLoop.State.mask := mask;
      MainLoop.State.P1 := P1;
      MainLoop.State._y2 := _y2;
      MainLoop.State._zzz2 := _zzz2
    |} := state in
    let P1 := high_ecDblNeg a p P1 in
    let s := high_get_s u v mask in
    if PointsSelector.is_zero s then
      let Y := p - P1.(PZZ.Y) in
      state <| MainLoop.State.P1 := state.(MainLoop.State.P1) <| PZZ.Y := Y |> |>
    else
    (* let T4 := (Ts.get Ts (Z.to_nat s)).(PZZ.X) in *)
    let T4 := (get_T s).(PZZ.X) in
    state.
End MainLoop.
