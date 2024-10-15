Require Import CoqOfSolidity.CoqOfSolidity.
Require Import simulations.CoqOfSolidity.
Require coqutil.Datatypes.List.

Import RunO.

Module Memory.
  Definition of_u256_list (words : list U256.t) : Memory.t.
  Admitted.

  Lemma run_mload codes environment state
      (words : list U256.t) (index : nat) (word : U256.t) :
    List.nth_error words index = Some word ->
    state.(State.memory) = of_u256_list words ->
    {{? codes, environment, Some state |
      Stdlib.mload (32 * Z.of_nat index) ⇓
      Result.Ok word
    | Some state ?}}.
  Proof.
  Admitted.

  Lemma run_mstore codes environment state
      (words : list U256.t) (index : nat) (word : U256.t) :
    match List.update_nth words index word with
    | Some words' =>
      let state' := state <| State.memory := of_u256_list words' |> in
      {{? codes, environment, Some state |
        Stdlib.mstore (32 * Z.of_nat index) word ⇓
        Result.Ok tt
      | Some state' ?}}
    | None => True
    end.
  Proof.
  Admitted.

  Lemma update_at (index : nat) (word2 : U256.t)
      {A : Set} codes environment state1 e (output : A) state'
      (words1 : list U256.t) (word1 : U256.t)
      (H_word : word2 = word1)
      (H_nth : List.nth_error words1 index = Some word1)
      (H_memory : state1.(State.memory) = of_u256_list words1) :
    let state2 :=
      state1 <| State.memory := of_u256_list (List.replace_nth index words1 word2) |> in
    {{? codes, environment, Some state2 |
      e ⇓ output
    | state' ?}} ->
    {{? codes, environment, Some state1 |
      e ⇓ output
    | state' ?}}.
  Proof.
    intros.
    assert (state1 = state2). {
      unfold state2.
      replace (of_u256_list _) with state1.(State.memory). 2: {
        rewrite H_memory, H_word.
        f_equal.
        revert H_nth; clear; intros.
        revert index H_nth.
        induction words1; hauto lq: on.
      }
      sfirstorder.
    }
    congruence.
  Qed.
End Memory.

Module SimulatedMemory.
  Definition t : Set :=
    list U256.t.

  Definition init : t :=
    [0; 0; 0; 0; 0].
End SimulatedMemory.

Module Address.
  Lemma implies_and_mask address :
    Address.Valid.t address ->
    address = Z.land address (2 ^ 160 - 1).
  Proof.
  Admitted.
End Address.

(** A value that admits a direct representation on the storage. *)
Module StorableValue.
  (** For now the maps can only contain integers. *)
  Inductive t : Set :=
  | U256 (value : U256.t)
  | Map (value : Dict.t U256.t U256.t)
  | Map2 (value : Dict.t (U256.t * U256.t) U256.t).

  (** The default value is zero when a key is not yet assigned. *)
  Definition map_get_u256 {K : Set} `{Dict.Eq.C K}
      (map : Dict.t K U256.t) (key : K) : U256.t :=
    match Dict.get map key with
    | Some value => value
    | None => 0
    end.

  Lemma map_get_u256_is_valid {K : Set} `{Dict.Eq.C K} (P_K : K -> Prop)
      (map : Dict.t K U256.t) (key : K)
      (H_map : Dict.Valid.t P_K U256.Valid.t map) :
    U256.Valid.t (map_get_u256 map key).
  Proof.
    unfold map_get_u256.
    pose proof (Dict.get_is_valid _ _ _ key H_map).
    destruct Dict.get; unfold U256.Valid.t in *; lia.
  Qed.
End StorableValue.

Module IsStorable.
  Class C (A : Set) : Set := {
    to_storable_value : A -> StorableValue.t;
  }.

  Global Instance IU256 : C U256.t := {
    to_storable_value := StorableValue.U256;
  }.

  Global Instance IMap : C (Dict.t U256.t U256.t) := {
    to_storable_value := StorableValue.Map;
  }.

  Global Instance IMap2 : C (Dict.t (U256.t * U256.t) U256.t) := {
    to_storable_value := StorableValue.Map2;
  }.
End IsStorable.

Module State.
  Definition get_current_storage
      (environment : Environment.t) (state : State.t) :
      option Storage.t :=
    let address := environment.(Environment.address) in
    let account := Dict.get state.(State.accounts) address in
    match account with
    | None => None
    | Some account => Some account.(Account.storage)
    end.

  Definition with_current_storage
      (environment : Environment.t) (state : State.t) (storage : Storage.t) :
      State.t :=
    let address := environment.(Environment.address) in
    let accounts :=
      Dict.assign_function state.(State.accounts) address (fun account =>
        account <| Account.storage := storage |>
      ) in
    match accounts with
    | None => state
    | Some accounts => state <| State.accounts := accounts |>
    end.

  Lemma get_current_storage_with_current_storage_eq
      environment state storage :
    get_current_storage environment (with_current_storage environment state storage) =
    Some storage.
  Proof.
  Admitted.
End State.
Global Opaque State.get_current_storage State.with_current_storage.

Lemma run_keccak256_tuple2 codes environment state
    (memory : list U256.t) (index : nat) (a b : U256.t) :
  state.(State.memory) = Memory.of_u256_list memory ->
  List.nth_error memory index = Some a ->
  List.nth_error memory (S index) = Some b ->
  {{? codes, environment, Some state |
    Stdlib.keccak256 (32 * (Z.of_nat index)) (32 * 2) ⇓
    Result.Ok (keccak256_tuple2 a b)
  | Some state ?}}.
Proof.
Admitted.

Module Storage.
  Definition of_storable_values (values : list StorableValue.t) : Storage.t.
  Admitted.

  Lemma run_sload_u256
      (values : list StorableValue.t)
      (index : nat)
      (value : U256.t)
      codes environment state :
    State.get_current_storage environment state = Some (of_storable_values values) ->
    List.nth_error values index = Some (StorableValue.U256 value) ->
    {{? codes, environment, Some state |
      Stdlib.sload (Z.of_nat index) ⇓
      Result.Ok value
    | Some state ?}}.
  Proof.
  Admitted.

  Lemma run_sstore_u256
      (values : list StorableValue.t)
      (index : nat)
      (value : U256.t)
      codes environment state :
    let state := State.with_current_storage environment state (of_storable_values values) in
    match List.update_nth values index (StorableValue.U256 value) with
    | Some values' =>
      let state' := State.with_current_storage environment state (of_storable_values values') in
      {{? codes, environment, Some state |
        Stdlib.sstore (Z.of_nat index) value ⇓
        Result.Ok tt
      | Some state' ?}}
    | None => True
    end.
  Proof.
  Admitted.

  Lemma run_sload_map_u256
      (values : list StorableValue.t)
      (index : nat)
      (map : Dict.t U256.t U256.t)
      (key : U256.t)
      codes environment state :
    List.nth_error values index = Some (StorableValue.Map map) ->
    {{? codes, environment, state |
      Stdlib.sload (keccak256_tuple2 key (Z.of_nat index)) ⇓
      Result.Ok (StorableValue.map_get_u256 map key)
    | state ?}}.
  Proof.
  Admitted.

  Lemma run_sstore_map_u256
      (values : list StorableValue.t)
      (index : nat)
      (key : U256.t) (value : U256.t)
      codes environment state :
    State.get_current_storage environment state = Some (of_storable_values values) ->
    match List.nth_error values index with
    | Some (StorableValue.Map map) =>
      let map' := Dict.declare_or_assign map key value in
      match List.update_nth values index (StorableValue.Map map') with
      | Some values' =>
        let state' := State.with_current_storage environment state (of_storable_values values') in
        {{? codes, environment, Some state |
          Stdlib.sstore (keccak256_tuple2 key (Z.of_nat index)) value ⇓
          Result.Ok tt
        | Some state' ?}}
      | None => True
      end
    | _ => True
    end.
  Proof.
  Admitted.

  Lemma run_sload_map2_u256
      (values : list StorableValue.t)
      (index : nat)
      (map : Dict.t (U256.t * U256.t) U256.t)
      (key1 key2 : U256.t)
      codes environment state :
    List.nth_error values index = Some (StorableValue.Map2 map) ->
    {{? codes, environment, state |
      Stdlib.sload (keccak256_tuple2 key2 (keccak256_tuple2 key1 (Z.of_nat index))) ⇓
      Result.Ok (StorableValue.map_get_u256 map (key1, key2))
    | state ?}}.
  Proof.
  Admitted.

  Lemma run_sstore_map2_u256
      (values : list StorableValue.t)
      (index : nat)
      (key1 key2 : U256.t) (value : U256.t)
      codes environment state :
    State.get_current_storage environment state = Some (of_storable_values values) ->
    match List.nth_error values index with
    | Some (StorableValue.Map2 map) =>
      let map' := Dict.declare_or_assign map (key1, key2) value in
      match List.update_nth values index (StorableValue.Map2 map') with
      | Some values' =>
        let state' := State.with_current_storage environment state (of_storable_values values') in
        {{? codes, environment, Some state |
          Stdlib.sstore
            (keccak256_tuple2 key2 (keccak256_tuple2 key1 (Z.of_nat index))) value ⇓
          Result.Ok tt
        | Some state' ?}}
      | None => True
      end
    | _ => True
    end.
  Proof.
  Admitted.
End Storage.

Module SimulatedStorage.
  Definition t : Set :=
    list StorableValue.t.

  Definition init : t := [
    StorableValue.Map [];
    StorableValue.Map [];
    StorableValue.U256 0
  ].
End SimulatedStorage.

Definition make_state environment state
    (memory : SimulatedMemory.t) (storage : SimulatedStorage.t) :
    State.t :=
  State.with_current_storage environment
    (state <| State.memory := Memory.of_u256_list memory |>)
    (Storage.of_storable_values storage).

Lemma get_memory_make_state_eq environment state
    memory storage :
  (make_state environment state memory storage).(State.memory) =
  Memory.of_u256_list memory.
Proof.
Admitted.

(** Lemma to put the state always in the same form. *)
Module CanonizeState.
  Lemma update_memory_eq environment state
      memory storage new_memory :
    (make_state environment state memory storage) <|
      State.memory := Memory.of_u256_list new_memory
    |> =
    make_state environment state new_memory storage.
  Proof.
  Admitted.

  Lemma update_storage_eq environment state
        memory storage new_storage :
    State.with_current_storage environment
      (make_state environment state memory storage)
      (Storage.of_storable_values new_storage) =
    make_state environment state memory new_storage.
  Proof.
  Admitted.

  Lemma with_current_storage_twice_eq environment state
      storage1 storage2 :
    State.with_current_storage environment
      (State.with_current_storage environment state storage1)
      storage2 =
    State.with_current_storage environment state storage2.
  Proof.
  Admitted.

  Ltac execute := repeat (
    rewrite update_memory_eq ||
    rewrite update_storage_eq ||
    rewrite with_current_storage_twice_eq ||
    match goal with
    | |- context[
      State.with_current_storage ?environment
        (?state <|State.memory:= Memory.of_u256_list ?memory |>)
        (Storage.of_storable_values ?storage)
      ] => fold (make_state environment state memory storage)
    end
  ).
End CanonizeState.

Ltac apply_run_mload :=
  match goal with
  | |- {{? _, _, Some (make_state _ _ ?memory _) | Stdlib.mload ?offset ⇓ _ | _ ?}} =>
    eapply (Memory.run_mload _ _ _ memory (Z.to_nat (offset / 32)));
    try reflexivity;
    try apply get_memory_make_state_eq
  end.

Ltac apply_run_mstore :=
  match goal with
  | |- {{? _, _, Some (make_state _ _ ?memory _) | Stdlib.mstore ?offset ?value ⇓ _ | _ ?}} =>
    apply (Memory.run_mstore _ _ _ memory (Z.to_nat (offset / 32)) value)
  end.

Ltac apply_memory_update_at index word2 :=
  let index := eval cbv in (Z.to_nat (index / 32)) in
  eapply (Memory.update_at index word2);
    try apply get_memory_make_state_eq;
    [|reflexivity|];
    unfold List.replace_nth;
    CanonizeState.execute.

Ltac apply_run_sload_u256 :=
  match goal with
  | |- {{? _, _, Some (make_state _ _ _ ?storage) | Stdlib.sload ?slot ⇓ _ | _ ?}} =>
    apply (Storage.run_sload_u256 storage (Z.to_nat slot));
    try reflexivity;
    try apply State.get_current_storage_with_current_storage_eq
  end.

Ltac apply_run_sstore_u256 :=
  match goal with
  | |- {{? _, _, Some (make_state _ _ _ ?storage) | Stdlib.sstore ?slot ?value ⇓ _ | _ ?}} =>
    apply (Storage.run_sstore_u256 storage (Z.to_nat slot) value)
  end.

Ltac apply_run_sload_map_u256 :=
  match goal with
  | |- {{? _, _, Some (make_state _ _ _ ?storage) |
      Stdlib.sload (keccak256_tuple2 ?key ?index) ⇓ _
    | _ ?}} =>
    eapply (Storage.run_sload_map_u256 storage (Z.to_nat index) _ key);
    try reflexivity
  end.

Ltac apply_run_sstore_map_u256 :=
  match goal with
  | |- {{? _, _, Some (make_state _ _ _ ?storage) |
      Stdlib.sstore (keccak256_tuple2 ?key ?index) ?value ⇓ _
    | _ ?}} =>
    eapply (Storage.run_sstore_map_u256 storage (Z.to_nat index) key value);
    try reflexivity;
    try apply State.get_current_storage_with_current_storage_eq
  end.

Ltac apply_run_sload_map2_u256 :=
  match goal with
  | |- {{? _, _, Some (make_state _ _ _ ?storage) |
      Stdlib.sload (keccak256_tuple2 ?key2 (keccak256_tuple2 ?key1 ?index)) ⇓ _
    | _ ?}} =>
    eapply (Storage.run_sload_map2_u256 storage (Z.to_nat index) _ key1 key2);
    try reflexivity
  end.

Ltac apply_run_sstore_map2_u256 :=
  match goal with
  | |- {{? _, _, Some (make_state _ _ _ ?storage) |
      Stdlib.sstore (keccak256_tuple2 ?key2 (keccak256_tuple2 ?key1 ?index)) ?value ⇓ _
    | _ ?}} =>
    eapply (Storage.run_sstore_map2_u256 storage (Z.to_nat index) key1 key2 value);
    try reflexivity;
    try apply State.get_current_storage_with_current_storage_eq
  end.

Ltac apply_run_keccak256_tuple2 :=
  match goal with
  | |- {{? _, _, Some (make_state _ _ ?memory _) | Stdlib.keccak256 ?pointer 64 ⇓ _ | _ ?}} =>
    apply (run_keccak256_tuple2 _ _ _ memory (Z.to_nat (pointer / 32)));
    try reflexivity;
    try apply get_memory_make_state_eq
  end.
