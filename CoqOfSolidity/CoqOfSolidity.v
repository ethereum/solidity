Require Export Coq.Strings.Ascii.
Require Coq.Strings.HexString.
Require Export Coq.Strings.String.
Require Export Coq.ZArith.ZArith.
From Ltac2 Require Export Ltac2.
Require Export RecordUpdate.

Require Export Lia.
From Hammer Require Export Tactics.

Global Set Primitive Projections.
Global Set Printing Projections.
Global Open Scope char_scope.
Global Open Scope string_scope.
Global Open Scope list_scope.
Global Open Scope type_scope.
Global Open Scope Z_scope.

Export List.ListNotations.

Inductive sigS {A : Type} (P : A -> Set) : Set :=
| existS : forall (x : A), P x -> sigS P.
Arguments existS {_ _}.

Reserved Notation "{ x @ P }" (at level 0, x at level 99).
Reserved Notation "{ x : A @ P }" (at level 0, x at level 99).
Reserved Notation "{ ' pat : A @ P }"
  (at level 0, pat strict pattern, format "{ ' pat : A @ P }").

Notation "{ x @ P }" := (sigS (fun x => P)) : type_scope.
Notation "{ x : A @ P }" := (sigS (A := A) (fun x => P)) : type_scope.
Notation "{ ' pat : A @ P }" := (sigS (A := A) (fun pat => P)) : type_scope.

Module U256.
  Definition t := Z.
End U256.

Module Environment.
  Record t : Set := {
    caller : U256.t;
    (** Amount of wei sent to the current contract *)
    callvalue : U256.t;
    calldata : list Z;
    (** The address of the contract. *)
    address : U256.t;
  }.
End Environment.

Module BlockUnit.
  (** The return value of a code block. *)
  Inductive t : Set :=
  (** The default value in case of success *)
  | Tt
  (** The instruction `break` was called *)
  | Break
  (** The instruction `continue` was called *)
  | Continue
  (** The instruction `leave` was called *)
  | Leave.
End BlockUnit.

Module Result.
  (** A wrapper for the result of an expression or a code block. We can either return a normal value
      with [Ok], or a special instruction [Return] that will stop the execution of the contract. *)
  Inductive t (A : Set) : Set :=
  | Ok (output : A)
  | Return (p s : U256.t)
  | Revert (p s : U256.t).
  Arguments Ok {_}.
  Arguments Return {_}.
  Arguments Revert {_}.
End Result.

Module Primitive.
  (** We group together primitives that share being impure functions operating over the state. *)
  Inductive t : Set -> Set :=
  | OpenScope : t unit
  | CloseScope : t unit
  | GetVar (name : string) : t U256.t
  | DeclareVars (names : list string) (values : list U256.t) : t unit
  | AssignVars (names : list string) (values : list U256.t) : t unit
  | MLoad (address length : U256.t) : t (list Z)
  | MStore (address : U256.t) (bytes : list Z) : t unit
  | SLoad (address : U256.t) : t U256.t
  | SStore (address value : U256.t) : t unit
  | RLoad : t (list Z)
  | TLoad (address : U256.t) : t U256.t
  | TStore (address value : U256.t) : t unit
  | Log (topics : list U256.t) (payload : list Z) : t unit
  | GetEnvironment : t Environment.t
  | GetNonce : t U256.t
  | GetCodedata (address : U256.t) : t (list Z)
  | CreateAccount (address code : U256.t) (codedata : list Z) : t unit
  | UpdateCodeForDeploy (address code : U256.t) : t unit
  | LoadImmutable (name : U256.t) : t U256.t
  | SetImmutable (name value : U256.t) : t unit
  (** The call stack is there to debug the semantics of Yul. *)
  | CallStackPush (name : string) (arguments : list (string * U256.t)) : t unit
  | CallStackPop : t unit.
End Primitive.

Module LowM.
  Inductive t (A : Set) : Set :=
  | Pure (output : A)
  | Primitive {B : Set}
      (primitive : Primitive.t B)
      (k : B -> t A)
  | DeclareFunction
      (name : string)
      (body : list U256.t -> t (Result.t (list U256.t)))
      (k : t A)
  | CallFunction
      (name : string)
      (arguments : list U256.t)
      (k : Result.t (list U256.t) -> t A)
  | Loop {B : Set}
      (body : t B)
      (** The final value to return if we decide to break of the loop. *)
      (break_with : B -> option B)
      (k : B -> t A)
  | CallContract
      (address : U256.t)
      (value : U256.t)
      (input : list Z)
      (k : U256.t -> t A)
  (** Explicit cut in the monadic expressions, to provide better composition for the proofs. *)
  | Let {B : Set} (e1 : t B) (k : B -> t A)
  | Impossible (message : string).
  Arguments Pure {_}.
  Arguments Primitive {_ _}.
  Arguments DeclareFunction {_}.
  Arguments CallFunction {_}.
  Arguments Loop {_ _}.
  Arguments CallContract {_}.
  Arguments Let {_ _}.
  Arguments Impossible {_}.

  Fixpoint let_ {A B : Set} (e1 : t A) (e2 : A -> t B) : t B :=
    match e1 with
    | Pure (output) =>
      e2 output
    | Primitive primitive k =>
      Primitive primitive (fun result => let_ (k result) e2)
    | DeclareFunction name body k =>
      DeclareFunction name body (let_ k e2)
    | CallFunction name arguments k =>
      CallFunction name arguments (fun result => let_ (k result) e2)
    | Loop body break_with k =>
      Loop body break_with (fun result => let_ (k result) e2)
    | CallContract contract value input k =>
      CallContract contract value input (fun result => let_ (k result) e2)
    | Let e1 k =>
      Let e1 (fun result => let_ (k result) e2)
    | Impossible message => Impossible message
    end.
End LowM.

Module M.
  Definition t (A : Set) := LowM.t (Result.t A).

  (** This axiom is only used as a marker, we eliminate it later. *)
  Parameter run : forall {A : Set}, t A -> A.

  Definition pure {A : Set} (output : A) : t A :=
    LowM.Pure (Result.Ok output).

  Definition generic_let {A B : Set}
      (let_ : forall {A B : Set}, LowM.t A -> (A -> LowM.t B) -> LowM.t B)
      (e1 : t A)
      (e2 : A -> t B) :
      t B :=
    let_ e1 (fun result =>
    match result with
    | Result.Ok value => e2 value
    | Result.Return p s => LowM.Pure (Result.Return p s)
    | Result.Revert p s => LowM.Pure (Result.Revert p s)
    end).
  Arguments generic_let /.

  Definition let_ {A B : Set} : t A -> (A -> t B) -> t B :=
    generic_let (@LowM.let_).

  Definition strong_let_ {A B : Set} : t A -> (A -> t B) -> t B :=
    generic_let (fun A B => @LowM.Let B A).

  Definition do (e1 : t BlockUnit.t) (e2 : t BlockUnit.t) : t BlockUnit.t :=
    strong_let_ e1 (fun output =>
    match output with
    | BlockUnit.Tt => e2
    | _ => pure output
    end).

  (** We sequence the scoping with the [let_] operator rather than with [do], so that we always
      unscope on [leave] but still keep the scope stack on [return]. *)
  Definition scope {A : Set} (e : t A) : t A :=
    let_ (LowM.Primitive Primitive.OpenScope pure) (fun _ =>
    let_ e (fun output =>
    let_ (LowM.Primitive Primitive.CloseScope pure) (fun _ =>
    pure output))).

  (** Same as for [scope], we use [let_] for sequencing. *)
  Definition log_call_stack {A : Set}
      (name : string)
      (arguments : list string)
      (argument_values : list U256.t)
      (e : t A)
      : t A :=
    let_ (
      LowM.Primitive (Primitive.CallStackPush name (List.combine arguments argument_values)) pure
    ) (fun _ =>
    let_ e (fun output =>
    let_ (LowM.Primitive Primitive.CallStackPop pure) (fun _ =>
    pure output))).

  Definition expr_stmt (_ : list U256.t) : t BlockUnit.t :=
    pure BlockUnit.Tt.

  Definition call (name : string) (arguments : list (list U256.t)) : t (list U256.t) :=
    let arguments := List.map (fun argument => List.hd 0 argument) arguments in
    LowM.CallFunction name arguments LowM.Pure.

  Definition if_ (condition : list U256.t) (success : t BlockUnit.t) : t BlockUnit.t :=
    match condition with
    | [0] => pure BlockUnit.Tt
    | [_] => success
    | _ => LowM.Impossible "if: expected a single value as condition"
    end.

  Definition declare (names : list string) (values : option (list U256.t)) : t BlockUnit.t :=
    let values_with_default :=
      match values with
      | None => List.map (fun _ => 0) names
      | Some values => values
      end in
    LowM.Primitive (Primitive.DeclareVars names values_with_default) (fun _ => pure BlockUnit.Tt).

  Definition assign (names : list string) (values : option (list U256.t)) : t BlockUnit.t :=
    let values_with_default :=
      match values with
      | None => List.map (fun _ => 0) names
      | Some values => values
      end in
    LowM.Primitive (Primitive.AssignVars names values_with_default) (fun _ => pure BlockUnit.Tt).

  Definition get_var (name : string) : t (list U256.t) :=
    LowM.Primitive (Primitive.GetVar name) (fun value => pure [value]).

  Fixpoint get_vars (names : list string) : t (list U256.t) :=
    match names with
    | [] => pure []
    | name :: names =>
      LowM.Primitive (Primitive.GetVar name) (fun value =>
      let_ (get_vars names) (fun values =>
      pure (value :: values)))
    end.

  Definition function (name : string) (arguments results : list string) (body : t BlockUnit.t) :
      t BlockUnit.t :=
    let body : list U256.t -> t (list U256.t) :=
      fun argument_values =>
        log_call_stack name arguments argument_values (
          scope (
            let_ (declare arguments (Some argument_values)) (fun _ =>
            let_ (declare results None) (fun _ =>
            let_ body (fun _ =>
            get_vars results)))
          )
        ) in
    LowM.DeclareFunction name body (pure BlockUnit.Tt).

  Fixpoint switch_aux (value : U256.t) (cases : list (option U256.t * t BlockUnit.t)) :
      t BlockUnit.t :=
    match cases with
    | [] => pure BlockUnit.Tt
    | (None, body) :: _ => body
    | (Some current_value, body) :: cases =>
      if Z.eqb current_value value then
        body
      else
        switch_aux value cases
    end.

  Definition switch (values : list U256.t) (cases : list (option U256.t * t BlockUnit.t)) :
      t BlockUnit.t :=
    let_ (
      match values with
      | [value] => pure value
      | _ => LowM.Impossible "switch value must be a single value"
      end
    ) (fun value =>
      switch_aux value cases
    ).

  Definition for_ (condition : t (list U256.t)) (after body : t BlockUnit.t) : t BlockUnit.t :=
    let loop_body : t BlockUnit.t :=
      strong_let_ condition (fun condition =>
      match condition with
      | [0] => pure BlockUnit.Break
      | [_] =>
        strong_let_ body (fun output =>
          match output with
          | BlockUnit.Tt | BlockUnit.Continue => after
          | BlockUnit.Break | BlockUnit.Leave => pure output
          end
        )
      | _ => LowM.Impossible "for: expected a single value as condition"
      end) in
    let break_with (result : Result.t BlockUnit.t) : option (Result.t BlockUnit.t) :=
      match result with
      | Result.Ok output =>
        match output with
        | BlockUnit.Break => Some (Result.Ok BlockUnit.Tt)
        | BlockUnit.Leave => Some (Result.Ok BlockUnit.Leave)
        | BlockUnit.Tt | BlockUnit.Continue => None
        end
      | _ => Some result
      end in
    LowM.Loop loop_body break_with LowM.Pure.

  Definition break : t BlockUnit.t :=
    pure BlockUnit.Break.

  Definition continue : t BlockUnit.t :=
    pure BlockUnit.Continue.

  Definition leave : t BlockUnit.t :=
    pure BlockUnit.Leave.

  (** A tactic that replaces all [run] markers with a bind operation.
      This allows to represent Rust programs without introducing
      explicit names for all intermediate computation results. *)
  Ltac monadic e :=
    lazymatch e with
    | context ctxt [let v := ?x in @?f v] =>
      refine (let_ _ _);
        [ monadic x
        | let v' := fresh v in
          intro v';
          let y := (eval cbn beta in (f v')) in
          lazymatch context ctxt [let v := x in y] with
          | let _ := x in y => monadic y
          | _ =>
            refine (let_ _ _);
              [ monadic y
              | let w := fresh "v" in
                intro w;
                let z := context ctxt [w] in
                monadic z
              ]
          end
        ]
    | context ctxt [run ?x] =>
      lazymatch context ctxt [run x] with
      | run x => monadic x
      | _ =>
        refine (let_ _ _);
          [ monadic x
          | let v := fresh "v" in
            intro v;
            let y := context ctxt [v] in
            monadic y
          ]
      end
    | _ =>
      lazymatch type of e with
      | t _ => exact e
      | _ => exact (pure e)
      end
    end.
End M.

Module Notations.
  Notation "'let*' x ':=' e 'in' k" :=
    (M.let_ e (fun x => k))
    (at level 200, x ident, e at level 200, k at level 200).

  Notation "'do*' a 'in' b" :=
    (M.do a b)
    (at level 200).

  Notation "e (| e1 , .. , en |)" :=
    (M.run ((.. (e e1) ..) en))
    (at level 100).

  Notation "e (||)" :=
    (M.run e)
    (at level 100).
End Notations.

Export M.
Export Notations.

Module Literal.
  Definition number (z : Z) : U256.t :=
    z.

  Definition bool (b : bool) : U256.t :=
    if b then 1 else 0.

  Definition string (z : Z) : U256.t :=
    z.
End Literal.

Module Code.
  Record t : Set := {
    name : string;
    hex_name : U256.t;
    code : M.t BlockUnit.t;
  }.

  Definition flatten (codes : list t) : list (U256.t * M.t BlockUnit.t) :=
    List.map (fun x => (x.(hex_name), x.(code))) codes.

  Ltac2 rec constr_of_list (l : constr list) : constr :=
    match l with
    | [] => '[]
    | x :: xs =>
      let rest := constr_of_list xs in
      '($x :: $rest)
    end.

  Ltac2 get_codes () : constr :=
    let codes := Env.expand [@code] in
    let codes := List.map Env.instantiate codes in
    let codes_of_right_type :=
      List.filter (fun x =>
        Constr.equal (Constr.type x) 't
      ) codes in
    let codes := constr_of_list codes_of_right_type in
    '(flatten $codes).
End Code.
