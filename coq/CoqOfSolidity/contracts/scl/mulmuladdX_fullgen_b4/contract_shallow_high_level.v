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
  cbn - [Z.eqb Z.add Z.sub Z.mul Z.div Z.modulo].

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

  Definition to_Z (selector : t) : Z :=
    let 'Build_t u_low u_high v_low v_high := selector in
    Z.b2z v_low + 2 * Z.b2z v_high + 4 * Z.b2z u_low + 8 * Z.b2z u_high.
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

(* Ltac zero_head a p P :=
  match P with
  | P.add _ _ ?P _ => zero_head a p P
  | ?P => rewrite <- (P.add_zero_l a p P) at 1
  end. *)

Lemma high_get_T_eq (a p : U256.t) (P P128 Q Q128 : PA.t)
    (H_P : PA.Valid.t p P)
    (H_P128 : PA.Valid.t p P128)
    (H_Q : PA.Valid.t p Q)
    (H_Q128 : PA.Valid.t p Q128)
    (selector : PointsSelector.t) :
  let params : Q.t := {|
    Q.Qx := P.(PA.X);
    Q.Qy := P.(PA.Y);
    Q.Q'x := P128.(PA.X);
    Q.Q'y := P128.(PA.Y);
    Q.p := p;
    Q.a := a;
    Q.gx := Q.(PA.X);
    Q.gy := Q.(PA.Y);
    Q.gx2pow128 := Q128.(PA.X);
    Q.gy2pow128 := Q128.(PA.Y);
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
