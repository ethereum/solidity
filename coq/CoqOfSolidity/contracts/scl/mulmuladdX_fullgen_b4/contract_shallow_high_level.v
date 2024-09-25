Require Import CoqOfSolidity.CoqOfSolidity.
Require Import CoqOfSolidity.simulations.CoqOfSolidity.
Import Stdlib.

Ltac Zify.zify_post_hook ::= Z.to_euclidean_division_equations.

Module Zp.
  Module Valid.
    Definition t (p n : Z) : Prop :=
      0 <= n < p.
  End Valid.
End Zp.

Module PZZ.
  Record t : Set := {
    X : U256.t;
    Y : U256.t;
    ZZ : U256.t;
    ZZZ : U256.t;
  }.

  Module Valid.
    Record t (p : U256.t) (P : PZZ.t) : Prop := {
      X_valid : Zp.Valid.t p P.(X);
      Y_valid : Zp.Valid.t p P.(Y);
      ZZ_valid : Zp.Valid.t p P.(ZZ);
      ZZZ_valid : Zp.Valid.t p P.(ZZZ);
    }.
  End Valid.
End PZZ.

Definition ecAddn2_original (x1 y1 zz1 zzz1 x2 y2 _p : U256.t) :
    U256.t * U256.t * U256.t * U256.t :=
  let y1 := Pure.sub _p y1 in
  let y2 := Pure.addmod (Pure.mulmod y2 zzz1 _p) y1 _p in
  let x2 := Pure.addmod (Pure.mulmod x2 zz1 _p) (Pure.sub _p x1) _p in
  let _x := Pure.mulmod x2 x2 _p in
  let _y := Pure.mulmod _x x2 _p in
  let _zz := Pure.mulmod zz1 _x _p in

  let _zzz := Pure.mulmod zzz1 _y _p in
  let zz1 := Pure.mulmod x1 _x _p in
  let _x :=
    Pure.addmod
      (Pure.addmod (Pure.mulmod y2 y2 _p) (Pure.sub _p _y) _p)
      (Pure.mulmod (Pure.sub _p 2) zz1 _p)
      _p in

  let x1 := Pure.mulmod (Pure.addmod zz1 (Pure.sub _p _x) _p) y2 _p in
  let _y := Pure.addmod x1 (Pure.mulmod y1 _y _p) _p in

  (_x, _y, _zz, _zzz).

(** Normalized addition of two point, must not be neutral input. *)
Definition ecAddn2 (P1 : PZZ.t) (x2 y2 _p : U256.t) : PZZ.t :=
  let x1 := P1.(PZZ.X) in
  let y1 := P1.(PZZ.Y) in
  let zz1 := P1.(PZZ.ZZ) in
  let zzz1 := P1.(PZZ.ZZZ) in

  let y1 := _p - y1 in
  let y2 := (y2 * zzz1) + y1 in
  let x2 := (x2 * zz1) + (_p - x1) in
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

Lemma sub_le_eq (n m : U256.t) :
  U256.Valid.t n ->
  U256.Valid.t m ->
  m <= n ->
  Pure.sub n m = n - m.
Proof.
  unfold U256.Valid.t, Pure.sub.
  lia.
Qed.

(* Lemma add_mod_left_without_mod (a b c : U256.t) :
  Pure.addmod (a mod c) b c =
  Pure.addmod a b c.
Proof.
  apply Zplus_mod_idemp_l.
Qed. *)

(* Lemma add_mod_second_without_mod (a b c : U256.t) :
  Pure.addmod a (b mod c) c =
  Pure.addmod a b c. *)

Lemma ecAddn2_original_eq_ecAddn2 (x1 y1 zz1 zzz1 x2 y2 _p : U256.t)
    (H_x1 : Zp.Valid.t _p x1)
    (H_y1 : Zp.Valid.t _p y1)
    (H_zz1 : Zp.Valid.t _p zz1)
    (H_zzz1 : Zp.Valid.t _p zzz1)
    (H_x2 : Zp.Valid.t _p x2)
    (H_y2 : Zp.Valid.t _p y2)
    (H__p : 2 <= _p < 2^256) :
  let P1 := {|
    PZZ.X := x1;
    PZZ.Y := y1;
    PZZ.ZZ := zz1;
    PZZ.ZZZ := zzz1;
  |} in
  let P := ecAddn2 P1 x2 y2 _p in
  ecAddn2_original x1 y1 zz1 zzz1 x2 y2 _p =
  (P.(PZZ.X), P.(PZZ.Y), P.(PZZ.ZZ), P.(PZZ.ZZZ)).
Proof.
  unfold Zp.Valid.t in *.
  unfold
    ecAddn2_original,
    Pure.addmod,
    Pure.mulmod,
    Pure.sub;
    simpl in *.
  (* rewrite sub_le_eq by (unfold U256.Valid.t; lia).
  rewrite sub_le_eq by (unfold U256.Valid.t; lia). *)
  (* set (two_pow_256 := Z.pow_pos 2 256) in *. *)
  (* replace ((_p - y1) mod two_pow_256) with (_p - y1) by lia.
  replace ((_p - x1) mod two_pow_256) with (_p - x1) by lia.
  replace ((_p - 2) mod two_pow_256) with (_p - 2) by lia. *)
  (* repeat (
    rewrite Zplus_mod_idemp_l ||
    rewrite Zplus_mod_idemp_r ||
    rewrite Zmult_mod_idemp_l ||
    rewrite Zmult_mod_idemp_r
  ). *)
  assert (H_foo : forall a, 0 <= a <= _p -> (_p - a) mod Z.pow_pos 2 256 = _p - a)
    by (intros; lia).
  repeat rewrite H_foo by lia.
  (* assert (H_remove_last_two_pow_256 : forall a, (_p - a mod _p) mod two_pow_256 = _p - a mod _p)
    by lia.
  repeat rewrite H_remove_last_two_pow_256. *)
  repeat match goal with
  | |- (_, _) = (_, _) => f_equal
  end;
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
Qed.
  rewrite H_foo.
  f_equal.
  f_equal.
  f_equal.
  { repeat (
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
  }
  { repeat (
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
  }
  { repeat (
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
  }
Qed.


  repeat f_equal.
  f_equal.
  f_equal.
  injection.

  rewrite H_remove_last_two_pow_256.
  rewrite H_remove_last_two_pow_256.

  intros.
  assert (0 <= a mod _p < _p) by lia.
  assert (0 <= _p - a mod _p < two_pow_256) by lia.
  lia.
  match goal with
  | |- context [(_p - ?x) mod two_pow_256] =>
    replace ((_p - x) mod two_pow_256) with (_p - x) by lia
  end.
  match goal with
  | |- context [(_p - ?x) mod two_pow_256] =>
    replace ((_p - x) mod two_pow_256) with (_p - x)
  end.
  2: {
    lia.
  }
  set (foo :=
    ((x2 * zz1 + (_p - x1)) mod _p * ((x2 * zz1 + (_p - x1)) mod _p) * (x2 * zz1 + (_p - x1))) mod _p
  ).
  nia.
  rewrite Zplus_mod_idemp_r.
  nia.
  2: {
    lia.
  }
  unfold Z.pow_pos.
Qed.
