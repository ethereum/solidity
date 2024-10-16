Require Import CoqOfSolidity.CoqOfSolidity.

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
