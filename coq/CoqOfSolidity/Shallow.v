Require Import CoqOfSolidity.CoqOfSolidity.

Module Mode.
  Inductive t : Set :=
  | Default
  | Break
  | Continue
  | Leave.
End Mode.

Module M.
  Definition t (State A : Set) : Set :=
    State -> Mode.t -> (A * State * Mode.t).
End M.

Module Control.
  Inductive t : Set :=
  | Break
  | Continue
  | Leave.
End Control.

Module Exception.
  Inductive t : Set :=
  | Return
  | Revert
  | Break
  | Continue
  | Leave.
End Exception.

Module Result.
  Inductive t (A : Set) : Set :=
  | Ok (output : A)
  | Exception (exception : Exception.t).
  Arguments Ok {_}.
  Arguments Exception {_}.

  (* Definition let_ {A B : Set} (x : t A) (f : A -> t B) : t B :=
    match x with
    | Ok output => f output
    | Exception exception => Exception exception
    end.

  Definition try_with {A B : Set} (x : t A) (body : A -> B) (catch : Exception.t -> B) : B :=
    match x with
    | Ok output => body output
    | Exception exception => catch exception
    end. *)
End Result.

Module M.
  Definition t (State A : Set) : Set :=
    State -> LowM.t (Result.t A * State).

  (* Definition return_ {State A : Set} (output : A) : t State A :=
    fun state => LowM.return_ (Result.Ok output, state).

  Definition let_ {State A B : Set} (x : t State A) (f : A -> t State B) : t State B :=
    fun state =>
      let (result, state) := x state in
      match result with
      | Result.Ok output => f output state
      | Result.Return p s => (Result.Return p s, state)
      | Result.Revert p s => (Result.Revert p s, state)
      | Result.Break => (Result.Break, state)
      | Result.Continue => (Result.Continue, state)
      | Result.Leave => (Result.Leave, state)
      end. *)
End M.

Axiom run : forall {State A : Set}, M.t State A -> A.

Definition pure {State A : Set} (output : A) : M.t State A :=
  fun state => LowM.Pure (Result.Ok output, state).
Arguments pure {_ _} /.

Definition generic_let {State A B : Set}
    (let_ : forall {A B : Set}, LowM.t A -> (A -> LowM.t B) -> LowM.t B)
    (e1 : M.t State A)
    (e2 : A -> M.t State B) :
    M.t State B :=
  fun state =>
  LowM.let_ (e1 state) (fun '(result, state) =>
  match result with
  | Result.Ok output => e2 output state
  | Result.Exception exception => LowM.Pure (Result.Exception exception, state)
  end).
Arguments generic_let {_ _ _} /.

Definition let_ {State A B : Set} : M.t State A -> (A -> M.t State B) -> M.t State B :=
  generic_let (@LowM.let_).

Definition strong_let_ {State A B : Set} : M.t State A -> (A -> M.t State B) -> M.t State B :=
  generic_let (fun A B => @LowM.Let B A).

(* Definition call {A : Set} (e : t A) : t A :=
    LowM.Call e LowM.Pure. *)

Definition call {State A : Set} (e : M.t State A) : M.t State A :=
  fun state => LowM.Call (e state) (fun '(result, state) => (result, state)).

Definition if_ {State : Set} (condition : U256.t) (success : M.t State unit) : M.t State unit :=
  fun state =>
    if condition =? 0
    then (Result.Ok tt, state)
    else success state.

Module Notations.
  Notation "'let~' x ':=' e 'in' k" :=
    (strong_let_ e (fun x => k))
    (at level 200, x ident, e at level 200, k at level 200).

  Notation "'let~' ' p ':=' e 'in' k" :=
    (strong_let_ e (fun p => k))
    (at level 200, p pattern, e at level 200, k at level 200).

  Notation "'do~' a 'in' b" :=
    (strong_let_ a (fun _ => b))
    (at level 200).

  Notation "e ~(| e1 , .. , en |)" :=
    (run (call ((.. (e e1) ..) en)))
    (at level 100).

  Notation "e ~(||)" :=
    (run (call e))
    (at level 100).

  Notation "[[ e ]]" :=
    (ltac:(M.monadic e))
    (only parsing).
End Notations.
