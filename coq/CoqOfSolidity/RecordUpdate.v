(** This file adds syntax for update of record fields, in the presence of primitive projections. *)
(* Modifed from: https://github.com/mit-plv/riscv-coq/blob/3a4ddc56fce50c0167fbe887987086fcc157153c/src/riscv/Utility/RecordSetters.v *)

Require Import Coq.Program.Basics.
Require Import Ltac2.Ltac2.
Require Ltac2.Option.
Require Import Ltac2.Bool.
Set Default Proof Mode "Classic".

Ltac2 rec strip_foralls (t : constr) :=
  match Constr.Unsafe.kind t with
  | Constr.Unsafe.Prod b u => let (bs, body) := strip_foralls u in (b :: bs, body)
  | _ => ([], t)
  end.

Ltac2 app_arg_count (t : constr) :=
  match Constr.Unsafe.kind t with
  | Constr.Unsafe.App f args => Array.length args
  | _ => 0
  end.

Ltac2 binder_to_field (qualification : ident list) (b : binder) :=
   Option.get (Env.get (List.append qualification [Option.get (Constr.Binder.name b)])).

Ltac2 field_names (ctor_ref : Std.reference) :=
  let ctor_type := Constr.type (Env.instantiate ctor_ref) in
  let (binders, result) := strip_foralls ctor_type in
  let n_type_args := app_arg_count result in
  let field_name_binders := List.skipn n_type_args binders in
  List.map (binder_to_field (List.removelast (Env.path ctor_ref))) field_name_binders.

Ltac2 constructor_of_record (t : constr) :=
  match Constr.Unsafe.kind t with
  | Constr.Unsafe.Ind ind inst =>
    Std.ConstructRef (Constr.Unsafe.constructor ind 0)
  | _ => Control.throw (Invalid_argument (Some (Message.of_constr t)))
  end.

Ltac2 mkApp (f : constr) (args : constr array) :=
  Constr.Unsafe.make (Constr.Unsafe.App f args).

Ltac2 record_with_set_val (ty : constr) (record : constr)
  (field : constr) (val : constr) : constr :=
  let (h, args) :=
    match Constr.Unsafe.kind ty with
    | Constr.Unsafe.App h args => (h, args)
    | _ => (ty, Array.empty ())
    end in
  let ctor := constructor_of_record h in
  let getters := List.map (fun getterRef =>
    mkApp (Env.instantiate getterRef) args) (field_names ctor) in
  let fields := List.map (fun getter =>
    if Constr.equal getter field then val else '($getter $record)
    ) getters in
  let res :=
    mkApp (mkApp (Env.instantiate ctor) args) (Array.of_list fields) in
  res.

Ltac2 setter (ty : constr) (field : constr) : constr := '(
  fun v r => ltac2:(
    let res := record_with_set_val ty &r field &v in exact $res)
  ).

Ltac exact_setter :=
  ltac2:(ty field |-
    let t := Option.get (Ltac1.to_constr ty) in
    let f := Option.get (Ltac1.to_constr field) in
    let res := setter t f
    in exact $res).

Class Setter{R E: Type}(getter: R -> E): Type :=
  set : E -> R -> R.

Arguments set {R E} (getter) {Setter} (fieldUpdater) (r).

Global Hint Extern 1 (@Setter ?R ?E ?getter) =>
  exact_setter R getter : typeclass_instances.

Module Export RecordSetNotations1.
  Declare Scope record_set.
  Delimit Scope record_set with rs.
  Open Scope rs.
  Notation "record <| field := v |>" := (set field v record)
     (at level 8, v at level 99, left associativity,
      format "record <| field :=  v |>")
     : record_set.
End RecordSetNotations1.

Module RecordSetterTests.
  Module PrimProjTests.
    Local Set Primitive Projections.

    Record Test (X : Type) := {
      f1 : X;
      f2 : X * X;
    }.

    Arguments f1 {_}.
    Arguments f2 {_}.

    Goal forall X (t : Test X) (x : X),
      t <| f1 := x |>.(f1) = x.
    Proof.
      intros; reflexivity.
    Qed.

    Goal forall X (t : Test X) (x : X),
      t <| f1 := x |>.(f2) = t.(f2).
    Proof.
      intros; reflexivity.
    Qed.
  End PrimProjTests.

  Module NonPrimProjTests.
    Record Test (X : Type) := {
      f1 : X;
      f2 : X * X;
    }.

    Arguments f1 {_}.
    Arguments f2 {_}.

    Goal forall X (t : Test X) (x : X),
      t <| f1 := x |>.(f1) = x.
    Proof.
      intros; reflexivity.
    Qed.

    Goal forall X (t : Test X) (x : X),
      t <| f1 := x |>.(f2) = t.(f2).
    Proof.
      intros; reflexivity.
    Qed.
  End NonPrimProjTests.
End RecordSetterTests.
