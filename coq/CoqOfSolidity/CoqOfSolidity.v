Require Export Coq.Strings.Ascii.
Require Coq.Strings.HexString.
Require Export Coq.Strings.String.
Require Export Coq.ZArith.ZArith.
From Ltac2 Require Ltac2.
Require Export RecordUpdate.

Require Export Lia.
From Hammer Require Export Tactics.

(* Activate the modulo arithmetic in [lia] *)
Ltac Zify.zify_post_hook ::= Z.to_euclidean_division_equations.

Global Set Primitive Projections.
Global Set Printing Projections.
Global Open Scope char_scope.
Global Open Scope string_scope.
Global Open Scope list_scope.
Global Open Scope type_scope.
Global Open Scope Z_scope.
Global Open Scope bool_scope.

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

  Module Valid.
    Definition t (value : U256.t) : Prop :=
      0 <= value < 2 ^ 256.
  End Valid.

  Definition of_bool (b : bool) : U256.t :=
    if b then 1 else 0.
End U256.

Module Address.
  (** This type is a synonym and acts mainly as documentation purpose. *)
  Definition t : Set := U256.t.

  Module Valid.
    Definition t (address : Address.t) : Prop :=
      0 <= address < 2 ^ 160.
  End Valid.
End Address.

Module Environment.
  Record t : Set := {
    caller : U256.t;
    (** Amount of wei sent to the current contract *)
    callvalue : U256.t;
    calldata : list Z;
    (** The address of the contract. *)
    address : U256.t;
    (** The name of the current code that is being executed. *)
    code_name : U256.t;
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

  Definition map {A B : Set} (f : A -> B) (value : t A) : t B :=
    match value with
    | Ok output => Ok (f output)
    | Return p s => Return p s
    | Revert p s => Revert p s
    end.
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
  | Pure
      (output : A)
  | Primitive {B : Set}
      (primitive : Primitive.t B)
      (k : B -> t A)
  | CallFunction
      (name : string)
      (arguments : list U256.t)
      (k : Result.t (list U256.t) -> t A)
  | Loop {In Out : Set}
      (init : In)
      (body : In -> t Out)
      (** The final value to return if we decide to break of the loop, otherwise what to continue
          with. *)
      (break_with : Out -> In + Out)
      (k : Out -> t A)
  | CallContract
      (address : U256.t)
      (value : U256.t)
      (input : list Z)
      (k : U256.t -> t A)
  (** Explicit cut in the monadic expressions, to provide better composition for the proofs. *)
  | Let {B : Set} (e1 : t B) (k : B -> t A)
  (** Similar to [Let], a marker to help for the proofs. *)
  | Call {B : Set} (e : t B) (k : B -> t A)
  | Impossible (message : string).
  Arguments Pure {_}.
  Arguments Primitive {_ _}.
  Arguments CallFunction {_}.
  Arguments Loop {_ _ _}.
  Arguments CallContract {_}.
  Arguments Let {_ _}.
  Arguments Call {_ _}.
  Arguments Impossible {_}.

  Fixpoint let_ {A B : Set} (e1 : t A) (e2 : A -> t B) : t B :=
    match e1 with
    | Pure (output) =>
      e2 output
    | Primitive primitive k =>
      Primitive primitive (fun result => let_ (k result) e2)
    | CallFunction name arguments k =>
      CallFunction name arguments (fun result => let_ (k result) e2)
    | Loop input body break_with k =>
      Loop input body break_with (fun result => let_ (k result) e2)
    | CallContract contract value input k =>
      CallContract contract value input (fun result => let_ (k result) e2)
    | Let e1 k =>
      Let e1 (fun result => let_ (k result) e2)
    | Call e k =>
      Call e (fun result => let_ (k result) e2)
    | Impossible message => Impossible message
    end.
End LowM.

Module M.
  Definition t (A : Set) := LowM.t (Result.t A).

  (** This axiom is only used as a marker, we eliminate it later. *)
  Parameter run : forall {A : Set}, t A -> A.

  Definition pure {A : Set} (output : A) : t A :=
    LowM.Pure (Result.Ok output).
  Arguments pure /.

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

  Definition call {A : Set} (e : t A) : t A :=
    LowM.Call e LowM.Pure.

  (** Whether we are in debug mode for the execution. It simplifies the understanding of the code,
      but also makes the formal verification more complex. *)
  Definition IS_DEBUG_EXEC : bool := false.

  (** In debug mode, we do not unscope in case of [Return] or [Revert]. *)
  Definition scope {A : Set} (e : t A) : t A :=
    if IS_DEBUG_EXEC then
      LowM.Primitive Primitive.OpenScope (fun _ =>
      let_ e (fun output =>
      LowM.Primitive Primitive.CloseScope (fun _ =>
      pure output)))
    else
      LowM.Primitive Primitive.OpenScope (fun _ =>
      LowM.let_ e (fun result =>
      LowM.Primitive Primitive.CloseScope (fun _ =>
      LowM.Pure result))).

  (** We sequence the scoping with the [let_] operator rather than with [do], so that we always
      pop on [leave] but still keep the call stack on [Return] or [Revert]. *)
  Definition log_call_stack {A : Set}
      (name : string)
      (arguments : list string)
      (argument_values : list U256.t)
      (e : t A)
      : t A :=
    if IS_DEBUG_EXEC then
      let_ (
        LowM.Primitive (Primitive.CallStackPush name (List.combine arguments argument_values)) pure
      ) (fun _ =>
      let_ e (fun output =>
      let_ (LowM.Primitive Primitive.CallStackPop pure) (fun _ =>
      pure output)))
    else
      e.

  Definition expr_stmt (_ : list U256.t) : t BlockUnit.t :=
    pure BlockUnit.Tt.

  Definition call_function (name : string) (arguments : list (list U256.t)) : t (list U256.t) :=
    let arguments := List.map (fun argument => List.hd 0 argument) arguments in
    LowM.CallFunction name arguments LowM.Pure.

  Definition if_ (conditions : list U256.t) (success : t BlockUnit.t) : t BlockUnit.t :=
    match conditions with
    | [condition] =>
      (* We use an `if` instead of a pattern so that we can descruct on the boolean condition. *)
      if condition =? 0 then
        pure BlockUnit.Tt
      else
        success
    | _ => LowM.Impossible "if: expected a single value as condition"
    end.

  Definition if_unit (condition : U256.t) (success : t unit) : t unit :=
    if condition =? 0 then
      pure tt
    else
      success.

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

  Definition make_function
      (name : string) (arguments results : list string) (body : t BlockUnit.t) :
      list U256.t -> t (list U256.t) :=
    fun argument_values =>
      log_call_stack name arguments argument_values (
        scope (
          let_ (declare arguments (Some argument_values)) (fun _ =>
          let_ (declare results None) (fun _ =>
          let_ body (fun _ =>
          get_vars results)))
        )
      ).

  Fixpoint switch_aux (value : U256.t) (cases : list (option U256.t * t BlockUnit.t)) :
      t BlockUnit.t :=
    match cases with
    | [] => pure BlockUnit.Tt
    | (None, body) :: _ => body
    | (Some current_value, body) :: cases =>
      if Z.eqb value current_value then
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
    let loop_body (_ : unit) : t BlockUnit.t :=
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
    let break_with (result : Result.t BlockUnit.t) : unit + Result.t BlockUnit.t :=
      match result with
      | Result.Ok output =>
        match output with
        | BlockUnit.Break => inr (Result.Ok BlockUnit.Tt)
        | BlockUnit.Leave => inr (Result.Ok BlockUnit.Leave)
        | BlockUnit.Tt | BlockUnit.Continue => inl tt
        end
      | _ => inr result
      end in
    LowM.Loop tt loop_body break_with LowM.Pure.

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

(* TODO: move this module in a separated file? *)
Module Shallow.
  Definition t (State : Set) : Set :=
    M.t (BlockUnit.t * State).

  Definition let_state {State1 State2 : Set}
      (expression : t State1) (body : State1 -> State2 * t State2) :
      t State2 :=
    M.strong_let_ expression (fun value =>
    let '(mode, state1) := value in
    match mode with
    | BlockUnit.Tt => snd (body state1)
    | _ => M.pure (mode, fst (body state1))
    end).

  Definition lift_state_update {State1 State2 : Set}
      (f : State1 -> State2)
      (e : M.t (BlockUnit.t * State1)) :
      M.t (BlockUnit.t * State2) :=
    M.let_ e (fun '(output, state) =>
    M.pure (output, f state)).

  Definition if_ {State : Set}
      (condition : U256.t)
      (success : M.t (BlockUnit.t * State))
      (failure : State) :
      M.t (BlockUnit.t * State) :=
    if condition =? 0 then
      M.pure (BlockUnit.Tt, failure)
    else
      success.

  Definition for_ {State : Set}
      (init_state : State)
      (condition : State -> M.t U256.t)
      (body : State -> M.t (BlockUnit.t * State))
      (post : State -> M.t (BlockUnit.t * State)) :
      M.t (BlockUnit.t * State) :=
    let loop_body (state : State) : M.t (BlockUnit.t * State) :=
      M.strong_let_ (condition state) (fun condition =>
      if condition =? 0 then
        M.pure (BlockUnit.Break, state)
      else
        M.strong_let_ (body state) (fun '(output, state) =>
        match output with
        | BlockUnit.Tt | BlockUnit.Continue => post state
        | BlockUnit.Break | BlockUnit.Leave => M.pure (output, state)
        end)) in
    let break_with (result : Result.t (BlockUnit.t * State)) :
        State + Result.t (BlockUnit.t * State) :=
      match result with
      | Result.Ok (output, state) =>
        match output with
        | BlockUnit.Break => inr (Result.Ok (BlockUnit.Tt, state))
        | BlockUnit.Leave => inr (Result.Ok (BlockUnit.Leave, state))
        | BlockUnit.Tt | BlockUnit.Continue => inl state
        end
      | _ => inr result
      end in
    LowM.Loop init_state loop_body break_with LowM.Pure.
End Shallow.

Module Notations.
  Notation "'let*' x ':=' e 'in' k" :=
    (M.let_ e (fun x => k))
    (at level 200, x ident, e at level 200, k at level 200).

  Notation "'let~' x ':=' e 'in' k" :=
    (M.strong_let_ e (fun x => k))
    (at level 200, x ident, e at level 200, k at level 200).

  Notation "'let~' ' p ':=' e 'in' k" :=
    (M.strong_let_ e (fun p => k))
    (at level 200, p pattern, e at level 200, k at level 200).

  Notation "'let_state~' x ':=' e 'default~' state 'in' k" :=
    (Shallow.let_state e (fun x => (state, k)))
    (at level 200, x ident, e at level 200, state at level 200, k at level 200).

  Notation "'let_state~' ' p ':=' e 'default~' state 'in' k" :=
    (Shallow.let_state e (fun p => (state, k)))
    (at level 200, p pattern, e at level 200, state at level 200, k at level 200).

  Notation "'do*' a 'in' b" :=
    (M.let_ a (fun _ => b))
    (at level 200).

  Notation "'do~' a 'in' b" :=
    (M.strong_let_ a (fun _ => b))
    (at level 200).

  Notation "'do!' a 'in' b" :=
    (M.do a b)
    (at level 200).

  Notation "e (| e1 , .. , en |)" :=
    (M.run ((.. (e e1) ..) en))
    (at level 100).

  Notation "e (||)" :=
    (M.run e)
    (at level 100).

  Notation "e ~(| e1 , .. , en |)" :=
    (M.run (M.call ((.. (e e1) ..) en)))
    (at level 100).

  Notation "e ~(||)" :=
    (M.run (M.call e))
    (at level 100).

  Notation "[[ e ]]" :=
    (ltac:(M.monadic e))
    (only parsing).
End Notations.

Export M.
Export Notations.

Module Literal.
  Definition number (z : Z) : U256.t :=
    z.
  Arguments number /.

  Definition bool (b : bool) : U256.t :=
    if b then 1 else 0.
  Arguments bool /.

  Definition string (z : Z) : U256.t :=
    z.
  Arguments string /.
End Literal.

Module Code.
  Module Function.
    Record t : Set := {
      arguments : list string;
      results : list string;
      body : M.t BlockUnit.t;
    }.

    Definition make (parameters : string * list string * list string * M.t BlockUnit.t) :
        string * t :=
      let '(name, arguments, results, body) := parameters in
      (
        name,
        {|
          arguments := arguments;
          results := results;
          body := body;
        |}
      ).
  End Function.

  Record t : Set := {
    name : string;
    hex_name : U256.t;
    functions : list (string * Function.t);
    body : M.t BlockUnit.t;
  }.

  (* This is important not to import Ltac2 before to still have access to Ltac1 before. *)
  Import Ltac2.

  Ltac2 rec constr_of_list (l : constr list) : constr :=
    match l with
    | [] => '[]
    | x :: xs =>
      let rest := constr_of_list xs in
      '($x :: $rest)
    end.

  (** Tactic to get all the code definitions in the current file. This is useful to know all the
      codes of the contracts to load for when making a contract call or deploy. *)
  Ltac2 get_codes () : constr :=
    let codes := Env.expand [@code] in
    let codes := List.map Env.instantiate codes in
    let codes_of_right_type :=
      List.filter (fun x =>
        Constr.equal (Constr.type x) 't
      ) codes in
    constr_of_list codes_of_right_type.
End Code.
