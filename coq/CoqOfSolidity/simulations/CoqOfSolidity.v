Require Import CoqOfSolidity.CoqOfSolidity.
Require EVM.Crypto.Keccak.

(** We should probably use an existing library for the [Dict.t] definition. We have mainly two kinds
    of keys in our code:
    - [string] for variable/function names
    - [U256.t] for addresses coming from the code
*)
Module Dict.
  Definition t (K V : Set) : Set :=
    list (K * V).

  Module Valid.
    Definition t {K V : Set} (P_K : K -> Prop) (P_V : V -> Prop) (dict : Dict.t K V) : Prop :=
      List.Forall (fun '(k, v) => P_K k /\ P_V v) dict.
  End Valid.

  Module Eq.
    Class C (A : Set) : Set :=
      eqb : A -> A -> bool.

    Global Instance IZ : C Z :=
      Z.eqb.

    Global Instance IString : C string :=
      String.eqb.

    Global Instance ITuple2 {A B : Set} `{C A} `{C B} : C (A * B) :=
      fun '(a1, b1) '(a2, b2) =>
        andb (eqb a1 a2) (eqb b1 b2).
  End Eq.

  Fixpoint get {K V : Set} `{Eq.C K} (dict : t K V) (key : K) : option V :=
    match dict with
    | [] => None
    | (current_key, current_value) :: dict =>
      if Eq.eqb key current_key then
        Some current_value
      else
        get dict key
    end.

  Lemma get_is_valid {K V : Set} `{Eq.C K} P_K P_V (dict : t K V) (key : K)
      (H_dict : Valid.t P_K P_V dict) :
    match get dict key with
    | None => True
    | Some value => P_V value
    end.
  Proof.
    induction dict as [|(current_key, current_value) dict IH]; cbn; trivial.
    sauto q: on.
  Qed.

  Lemma get_map_commut {K V1 V2 : Set} `{Eq.C K}
      (dict : t K V1) (key : K) (f : V1 -> V2) :
    match get dict key with
    | None => None
    | Some value => Some (f value)
    end =
    get (List.map (fun '(k, v) => (k, f v)) dict) key.
  Proof.
    induction dict as [|(current_key, current_value) dict IH]; cbn; trivial.
    hauto q: on.
  Qed.

  Definition declare {K V : Set} (dict : t K V) (key : K) (value : V) : t K V :=
    (key, value) :: dict.

  Fixpoint assign_function {K V : Set} `{Eq.C K}
      (dict : t K V) (key : K) (f : V -> V) :
      option (t K V) :=
    match dict with
    | [] => None
    | ((current_key, current_value) as entry) :: dict =>
      if Eq.eqb current_key key then
        Some ((current_key, f current_value) :: dict)
      else
        match assign_function dict key f with
        | None => None
        | Some dict => Some (entry :: dict)
        end
    end.

  Definition assign {K V : Set} `{Eq.C K} (dict : t K V) (key : K) (value : V) :
      option (t K V) :=
    assign_function dict key (fun _ => value).

  Fixpoint declare_or_assign_function {K V : Set} `{Eq.C K}
      (dict : t K V) (key : K) (f : option V -> V) :
      t K V :=
    match dict with
    | [] => [(key, f None)]
    | ((current_key, current_value) as entry) :: dict =>
      if Eq.eqb current_key key then
        (key, f (Some current_value)) :: dict
      else
        entry :: declare_or_assign_function dict key f
    end.

  Definition declare_or_assign {K V : Set} `{Eq.C K}
      (dict : t K V) (key : K) (value : V) :
      t K V :=
    declare_or_assign_function dict key (fun _ => value).

  Lemma declare_or_assign_is_valid {K V : Set} `{Eq.C K} P_K P_V
      (dict : t K V) (key : K) (value : V)
      (H_dict : Valid.t P_K P_V dict)
      (H_key : P_K key)
      (H_value : P_V value) :
    Valid.t P_K P_V (declare_or_assign dict key value).
  Proof.
    induction dict as [|(current_key, current_value) dict IH]; cbn; sauto q: on.
  Qed.

  Lemma declare_or_assign_map_commut {K V1 V2 : Set} `{Eq.C K}
      (dict : t K V1) (key : K) (value : V1) (f : V1 -> V2) :
    List.map (fun '(k, v) => (k, f v)) (declare_or_assign dict key value) =
    declare_or_assign (List.map (fun '(k, v) => (k, f v)) dict) key (f value).
  Proof.
    induction dict as [|(current_key, current_value) dict IH]; cbn; trivial.
    hauto q: on.
  Qed.
End Dict.

(** Some additional list primitives *)
Module List.
  Fixpoint update_nth {A : Set} (l : list A) (index : nat) (x : A) : option (list A) :=
    match index, l with
    | O, h :: t => Some (x :: t)
    | S index', h :: t => 
        match update_nth t index' x with
        | Some l' => Some (h :: l')
        | None => None
        end
    | _, [] => None
    end.

  (** Like [firstn] but we guarantee to always return [n] values, filling up with [default] if the
      list is empty. *)
  Fixpoint firstn_or_default {A : Set} (n : nat) (default : A) (l : list A) : list A :=
    match n with
    | O => []
    | S n' =>
      match l with
      | [] => default :: firstn_or_default n' default []
      | x :: xs => x :: firstn_or_default n' default xs
      end
    end.
End List.

Module Locals.
  Definition t : Set :=
    list (string * U256.t).
End Locals.

Module Stack.
  Definition t : Set :=
    list Locals.t.

  Definition open_scope (stack : t) : t :=
    [] :: stack.

  Definition close_scope (stack : t) : t + string :=
    match stack with
    | [] => inr "cannot close the last scope as there are none left"
    | _ :: stack => inl stack
    end.

  Fixpoint get_var (stack : t) (name : string) : U256.t + string :=
    match stack with
    | [] => inr ("variable '" ++ name ++ "' not found")%string
    | locals :: stack =>
      match Dict.get locals name with
      | None => get_var stack name
      | Some value => inl value
      end
    end.

  Definition declare_var (stack : t) (name : string) (value : U256.t) : t + string :=
    match stack with
    | [] => inr "no scope to declare the variable"
    | locals :: stack => inl (Dict.declare locals name value :: stack)
    end.

  Fixpoint declare_vars_aux (stack : t) (names : list string) (values : list U256.t) :
      option (t + string) :=
    match names, values with
    | [], [] => Some (inl stack)
    | name :: names, value :: values =>
      match declare_var stack name value with
      | inl stack => declare_vars_aux stack names values
      | inr error => Some (inr error)
      end
    | _, _ => None
    end.

  Definition declare_vars (stack : t) (names : list string) (values : list U256.t) : t + string :=
    match declare_vars_aux stack names values with
    | Some result => result
    | None =>
      inr (
        "declare: names and values have different lengths for names: " ++
        String.concat ", " names
      )%string
    end.

  Fixpoint assign_var (stack : t) (name : string) (value : U256.t) : t + string :=
    match stack with
    | [] => inr ("variable '" ++ name ++ "' not found")%string
    | locals :: stack =>
      match Dict.assign locals name value with
      | None =>
        match assign_var stack name value with
        | inl stack => inl (locals :: stack)
        | inr error => inr error
        end
      | Some locals => inl (locals :: stack)
      end
    end.

  Fixpoint assign_vars_aux (stack : t) (names : list string) (values : list U256.t) :
      option (t + string) :=
    match names, values with
    | [], [] => Some (inl stack)
    | name :: names, value :: values =>
      match assign_var stack name value with
      | inl stack => assign_vars_aux stack names values
      | inr error => Some (inr error)
      end
    | _, _ => None
    end.

  Definition assign_vars (stack : t) (names : list string) (values : list U256.t) : t + string :=
    match assign_vars_aux stack names values with
    | Some result => result
    | None =>
      inr (
        "assign: names and values have different lengths for names: " ++
        String.concat ", " names
      )%string
    end.
End Stack.

Module Memory.
  (** We define the memory as a function instead of an explicit list as there can be holes in it. It
      goes from addresses in [U256.t] to bytes represented as [Z]. *)
  Definition t : Set :=
    U256.t -> Z.

  Definition empty : t :=
    fun _ => 0.

  (** Get the bytes from some memory from a start adress and for a certain length. *)
  Definition get_bytes (memory : Memory.t) (start length : U256.t) : list Z :=
    List.map
      (fun (i : nat) =>
        let address : U256.t := start + Z.of_nat i in
        memory address
      )
      (List.seq 0 (Z.to_nat length)).

  Definition update_bytes (memory : Memory.t) (start : U256.t) (bytes : list Z) : Memory.t :=
    fun address =>
      let i : Z := address - start in
      if andb (0 <=? i) (i <? Z.of_nat (List.length bytes)) then
        List.nth_default 0 bytes (Z.to_nat i)
      else
        memory address.

  Definition u256_as_bytes (value : U256.t) : list Z :=
    List.map
      (fun (i : nat) => Z.shiftr value (8 * (31 - Z.of_nat i)) mod 256)
      (List.seq 0 32).

  Fixpoint bytes_as_u256_aux (acc : Z) (bytes : list Z) : U256.t :=
    match bytes with
    | [] => acc
    | byte :: bytes =>
      bytes_as_u256_aux
        (acc * 256 + byte)
        bytes
    end.

  Definition bytes_as_u256 (bytes : list Z) : U256.t :=
    bytes_as_u256_aux 0 bytes.

  Lemma bytes_as_u256_bounds (bytes : list Z)
      (H_bytes : List.Forall (fun byte => 0 <= byte < 256) bytes) :
    0 <= bytes_as_u256 bytes < 2 ^ (8 * Z.of_nat (List.length bytes)).
  Proof.
  Admitted.

  Definition bytes_as_bytes (bytes : list Z) : list Nibble.byte :=
    List.map
      (fun (byte : Z) => Nibble.byte_of_N (Z.to_N byte))
      bytes.

  Fixpoint hex_string_as_bytes (hex_string : string) : list Z :=
    match hex_string with
    | "" => []
    | String.String a "" => [] (* this case is unexpected *)
    | String.String a (String.String b rest) =>
      match HexString.ascii_to_digit a, HexString.ascii_to_digit b with
      | Some a, Some b =>
        let byte := 16 * Z.of_N a + Z.of_N b in
        byte :: hex_string_as_bytes rest
      | _, _ => [] (* this case is unexpected *)
      end
    end.
End Memory.

Module Storage.
  (** Each slot in the storage is a word. This is different from [Memory.t] where it is only
      bytes. *)
  Definition t : Set :=
    U256.t -> U256.t.

  Definition empty : t :=
    fun _ => 0.

  Definition update (storage : Storage.t) (address value : U256.t) : Storage.t :=
    fun current_address =>
      if address =? current_address then
        value
      else
        storage current_address.
End Storage.

Module CallStack.
  (** The list of functions that were called with their corresponding parameters. This is for
      debugging purpose only, and does not exist in the semantics of Yul. *)
  Definition t : Set :=
    list (string * list (string * U256.t)).
End CallStack.

Module Account.
  Record t : Set := {
    balance : U256.t;
    nonce: Z;
    code : U256.t;
    (** When calling a constructor the parameters are concatenated to the code. We represent them
        here. *)
    codedata : list Z;
    storage : Storage.t;
    immutables : Dict.t U256.t U256.t;
  }.
End Account.

Module State.
  (** The state contains the various kinds of memory that we use in a smart contract. *)
  Record t : Set := {
    stack : Stack.t;
    memory : Memory.t;
    return_data : list Z;
    transient_storage : Storage.t;
    accounts : list (U256.t * Account.t);
    logs : list (list U256.t * list Z);
    (** This is only for debugging *)
    call_stack : CallStack.t;
  }.

  Definition init : State.t :=
    {|
      State.stack := [];
      State.memory := Memory.empty;
      State.return_data := [];
      State.transient_storage := Memory.empty;
      State.accounts := [];
      State.logs := [];
      State.call_stack := [];
    |}.
End State.

(** Compute a selector from the signature of an entrypoint *)
Definition get_selector (entrypoint_name : string) : U256.t :=
  let hash : list Nibble.byte := EVM.Crypto.Keccak.keccak_256_of_string entrypoint_name in
  let hash : list Z := (List.map (fun byte => Z.of_N (Nibble.N_of_byte byte))) hash in
  Memory.bytes_as_u256 (List.firstn 4 hash).

Goal get_selector "transfer()" = 0x8a4068dd.
Proof. reflexivity. Qed.

Module StdlibAux.
  Definition get_signed_value (value : U256.t) : Z :=
    ((value + (2 ^ 255)) mod (2 ^ 256)) - (2 ^ 255).

  Goal List.map get_signed_value [0; 10; (2 ^ 256) - 3] = [0; 10; -3].
  Proof. reflexivity. Qed.

  Definition keccak256 (bytes : list U256.t) : U256.t :=
    let bytes := Memory.bytes_as_bytes bytes in
    let hash : list Nibble.byte := EVM.Crypto.Keccak.keccak_256 bytes in
    let hash : list Z := List.map (fun byte => Z.of_N (Nibble.N_of_byte byte)) hash in
    Memory.bytes_as_u256 hash.

  Definition get_calldata_u256 (calldata : list Z) (index : U256.t) : U256.t :=
    let index := Z.to_nat index in
    let bytes := List.firstn_or_default 32 0 (List.skipn index calldata) in
    Memory.bytes_as_u256 bytes.

  Lemma get_calldata_u256_is_valid (calldata : list Z) (index : U256.t)
      (H_calldata : List.Forall (fun byte => 0 <= byte < 256) calldata) :
    U256.Valid.t (get_calldata_u256 calldata index).
  Proof.
  Admitted.
End StdlibAux.

Module Stdlib.
  (** The pure functions from the stdlib. *)
  Module Pure.
    Definition add (x y : U256.t) : U256.t :=
      (x + y) mod (2 ^ 256).

    Definition sub (x y : U256.t) : U256.t :=
      (x - y) mod (2 ^ 256).

    Definition mul (x y : U256.t) : U256.t :=
      (x * y) mod (2 ^ 256).

    Definition div (x y : U256.t) : U256.t :=
      if y =? 0 then
        0
      else
        x / y.

    Definition sdiv (x y : U256.t) : U256.t :=
      if y =? 0 then
        0
      else
        let x := StdlibAux.get_signed_value x in
        let y := StdlibAux.get_signed_value y in
        let result := x / y in
        result mod (2 ^ 256).

    Definition mod_ (x y : U256.t) : U256.t :=
      if y =? 0 then
        0
      else
        x mod y.

    Definition smod (a b : Z) : U256.t :=
      let a := StdlibAux.get_signed_value a in
      let b := StdlibAux.get_signed_value b in
      if b =? 0 then
        0
      else
        (Z.rem a b) mod (2 ^ 256).

    Goal List.map (fun '(x, y) => smod x y)
      [(10, 3); (10, 0); (10, 10); (2^256 -8, 2^256 -3); (7, 5); (7, 2^256 -5)] =
      [1; 0; 0; 2^256 -2; 2; 2].
    Proof. vm_compute. reflexivity. Qed.

    Definition exp (x y : U256.t) : U256.t :=
      x ^ y.

    Definition not (x : U256.t) : U256.t :=
      2^256 - x - 1.

    Definition lt (x y : U256.t) : U256.t :=
      if x <? y then
        1
      else
        0.

    Definition gt (x y : U256.t) : U256.t :=
      if x >? y then
        1
      else
        0.

    (* Signed version of [lt] *)
    Definition slt (x y : U256.t) : U256.t :=
      let x := (x + 2 ^ 255) mod (2 ^ 256) in
      let y := (y + 2 ^ 255) mod (2 ^ 256) in
      lt x y.

    Definition sgt (x y : U256.t) : U256.t :=
      let x := (x + 2 ^ 255) mod (2 ^ 256) in
      let y := (y + 2 ^ 255) mod (2 ^ 256) in
      gt x y.

    Definition eq (x y : U256.t) : U256.t :=
      if x =? y then
        1
      else
        0.

    Definition iszero (x : U256.t) : U256.t :=
      if x =? 0 then
        1
      else
        0.

    Definition and (x y : U256.t) : U256.t :=
      Z.land x y.

    Definition or (x y : U256.t) : U256.t :=
      Z.lor x y.

    Definition xor (x y : U256.t) : U256.t :=
      Z.lxor x y.

    Definition byte (n x : U256.t) : U256.t :=
      (x / (256 ^ (31 - n))) mod 256.

    Definition shl (x y : U256.t) : U256.t :=
      (y * (2 ^ x)) mod (2 ^ 256).

    Definition shr (x y : U256.t) : U256.t :=
      y / (2 ^ x).

    Definition sar (shift : U256.t) (value : U256.t) : U256.t :=
      let signed_value := StdlibAux.get_signed_value value in
      let shifted_value := signed_value / (2 ^ shift) in
      shifted_value mod (2 ^ 256).

    Definition test_sar :=
      let test_cases := [
        (* Test case 1: shift right by 0 *)
        (0, 123456789, 123456789);
        (* Test case 2: shift right by 1 *)
        (1, 123456789, 61728394);
        (* Test case 3: shift right by 8 *)
        (8, 123456789, 482253);
        (* Test case 4: shift right by 16 *)
        (16, 123456789, 1883);
        (* Test case 5: shift right by 31 *)
        (31, 123456789, 0);
        (* Test case 6: shift right by 0 for a negative number *)
        (0, -123456789 mod (2 ^ 256), -123456789 mod (2 ^ 256));
        (* Test case 7: shift right by 1 for a negative number *)
        (1, -123456789 mod (2 ^ 256), -61728395 mod (2 ^ 256));
        (* Test case 8: shift right by 8 for a negative number *)
        (8, -123456789 mod (2 ^ 256), -482254 mod (2 ^ 256));
        (* Test case 9: shift right by 16 for a negative number *)
        (16, -123456789 mod (2 ^ 256), -1884 mod (2 ^ 256));
        (* Test case 10: shift right by 31 for a negative number *)
        (31, -123456789 mod (2 ^ 256), -1 mod (2 ^ 256))
      ] in
      List.forallb (fun '(shift, value, expected) =>
        let result := Stdlib.Pure.sar shift value in
        result =? expected
      ) test_cases.

    Goal test_sar = true.
    Proof. reflexivity. Qed.

    Definition addmod (x y m : U256.t) : U256.t :=
      (x + y) mod m.

    Definition mulmod (x y m : U256.t) : U256.t :=
      (x * y) mod m.

    Definition signextend (i x : Z) : U256.t :=
      if i >=? 31 then
        x
      else
        let size := 8 * (i + 1) in
        let byte := (x / 2 ^ (8 * i)) mod 256 in
        let sign_bit := byte / 128 in
        let extend_bit (bit size : Z) : Z :=
          if bit =? 1 then
            (2 ^ 256) - (2 ^ size)
          else
            0 in
        (x mod (2 ^ size)) + extend_bit sign_bit size.
  End Pure.

  Definition stop : M.t unit :=
    LowM.Pure (Result.Return 0 0).

  Definition add (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.add x y).

  Definition sub (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.sub x y).

  Definition mul (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.mul x y).

  Definition div (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.div x y).

  Definition sdiv (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.sdiv x y).

  Definition mod_ (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.mod_ x y).

  Definition smod (a b : Z) : M.t U256.t :=
    M.pure (Pure.smod a b).

  Definition exp (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.exp x y).

  Definition not (x : U256.t) : M.t U256.t :=
    M.pure (Pure.not x).

  Definition lt (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.lt x y).

  Definition gt (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.gt x y).

  (* Signed version of [lt] *)
  Definition slt (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.slt x y).

  Definition sgt (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.sgt x y).

  Definition eq (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.eq x y).

  Definition iszero (x : U256.t) : M.t U256.t :=
    M.pure (Pure.iszero x).

  Definition and (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.and x y).

  Definition or (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.or x y).

  Definition xor (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.xor x y).

  Definition byte (n x : U256.t) : M.t U256.t :=
    M.pure (Pure.byte n x).

  Definition shl (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.shl x y).

  Definition shr (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.shr x y).

  Definition sar (x y : U256.t) : M.t U256.t :=
    M.pure (Pure.sar x y).

  Definition addmod (x y m : U256.t) : M.t U256.t :=
    M.pure (Pure.addmod x y m).

  Definition mulmod (x y m : U256.t) : M.t U256.t :=
    M.pure (Pure.mulmod x y m).

  Definition signextend (i x : Z) : M.t U256.t :=
    M.pure (Pure.signextend i x).

  Definition keccak256 (p n : U256.t) : M.t U256.t :=
    let* bytes := LowM.Primitive (Primitive.MLoad p n) M.pure in
    M.pure (StdlibAux.keccak256 bytes).

  Definition pc : M.t U256.t :=
    LowM.Impossible "pc".

  Definition pop (x : U256.t) : M.t unit :=
    M.pure tt.

  Definition mload (address : U256.t) : M.t U256.t :=
    let* bytes := LowM.Primitive (Primitive.MLoad address 32) M.pure in
    M.pure (Memory.bytes_as_u256 bytes).

  Definition mstore (address value : U256.t) : M.t unit :=
    let bytes := Memory.u256_as_bytes value in
    LowM.Primitive (Primitive.MStore address bytes) M.pure.

  Definition mstore8 (address value : U256.t) : M.t unit :=
    let bytes := [value mod 256] in
    LowM.Primitive (Primitive.MStore address bytes) M.pure.

  Definition sload (address : U256.t) : M.t U256.t :=
    LowM.Primitive (Primitive.SLoad address) M.pure.

  Definition sstore (address value : U256.t) : M.t unit :=
    LowM.Primitive (Primitive.SStore address value) M.pure.

  Definition tload (address : U256.t) : M.t U256.t :=
    LowM.Primitive (Primitive.TLoad address) M.pure.

  Definition tstore (address value : U256.t) : M.t unit :=
    LowM.Primitive (Primitive.TStore address value) M.pure.

  Definition msize : M.t U256.t :=
    LowM.Impossible "msize".

  Definition gas : M.t U256.t :=
    M.pure 1000.

  Definition address : M.t U256.t :=
    let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
    M.pure environment.(Environment.address).

  Definition balance (a : U256.t) : M.t U256.t :=
    LowM.Impossible "balance".

  Definition selfbalance : M.t U256.t :=
    LowM.Impossible "selfbalance".

  Definition caller : M.t U256.t :=
    LowM.Primitive Primitive.GetEnvironment (fun env => M.pure env.(Environment.caller)).

  Definition callvalue : M.t U256.t :=
    LowM.Primitive Primitive.GetEnvironment (fun env => M.pure env.(Environment.callvalue)).

  Definition calldataload (p : U256.t) : M.t U256.t :=
    let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
    let calldata := environment.(Environment.calldata) in
    M.pure (StdlibAux.get_calldata_u256 calldata p).

  Definition calldatasize : M.t U256.t :=
    let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
    M.pure (Z.of_nat (List.length environment.(Environment.calldata))).

  Definition calldatacopy (t f s : U256.t) : M.t unit :=
    let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
    let calldata := environment.(Environment.calldata) in
    let bytes : list Z :=
      List.map
        (fun (i : nat) =>
          let address : nat := (Z.to_nat f + i)%nat in
          List.nth_default 0 calldata address
        )
        (List.seq 0 (Z.to_nat s)) in
    LowM.Primitive (Primitive.MStore t bytes) M.pure.

  Definition codesize : M.t U256.t :=
    let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
    let address := environment.(Environment.address) in
    let* codedata := LowM.Primitive (Primitive.GetCodedata address) M.pure in
    M.pure (32 + Z.of_nat (List.length codedata)).

  (** There are two kinds of code copy that we handle: either to copy actual code, or
      to copy the constructor's parameters that are stored just after the code of the
      constructor. *)
  Definition codecopy (t f s : U256.t) : M.t unit :=
    (* code case *)
    if f mod (2^256) =? 0 then
      if s =? 32 then
        let name : U256.t := f / (2^256) in
        let bytes := Memory.u256_as_bytes name in
        LowM.Primitive (Primitive.MStore t bytes) M.pure
      else
        LowM.Impossible "codecopy: s must be 32 for the code case"
    (* codedata case *)
    else if f mod (2^256) =? 32 then
      let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
      let* codedata :=
        LowM.Primitive (Primitive.GetCodedata environment.(Environment.address)) M.pure in
      if s =? Z.of_nat (List.length codedata) then
        LowM.Primitive (Primitive.MStore t codedata) M.pure
      else
        LowM.Impossible "codecopy: s must be the length of the codedata for the codedata case"
    else
      LowM.Impossible "codecopy: f mod (2^256) must be 0 or 32".

  Definition extcodesize (a : U256.t) : M.t U256.t :=
    let* codedata := LowM.Primitive (Primitive.GetCodedata a) M.pure in
    M.pure (32 + Z.of_nat (List.length codedata)).

  Definition extcodecopy (a t f s : U256.t) : M.t unit :=
    LowM.Impossible "extcodecopy".

  Definition returndatasize : M.t U256.t :=
    let* return_data := LowM.Primitive Primitive.RLoad M.pure in
    M.pure (Z.of_nat (List.length return_data)).

  Definition returndatacopy (t f s : U256.t) : M.t unit :=
    let* return_data := LowM.Primitive Primitive.RLoad M.pure in
    let bytes : list Z :=
      List.map
        (fun (i : nat) =>
          let index : nat := (Z.to_nat f + i)%nat in
          List.nth_default 0 return_data index
        )
        (List.seq 0 (Z.to_nat s)) in
    LowM.Primitive (Primitive.MStore t bytes) M.pure.

  Definition mcopy (t f s : U256.t) : M.t unit :=
    let* bytes := LowM.Primitive (Primitive.MLoad f s) M.pure in
    LowM.Primitive (Primitive.MStore t bytes) M.pure.

  Definition extcodehash (a : U256.t) : M.t U256.t :=
    LowM.Impossible "extcodehash".

  Definition create (v p n : U256.t) : M.t U256.t :=
    (* TODO: have the exact calculation of the address with RLP *)
    let* created_address :=
      let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
      let address := environment.(Environment.address) in
      let* nonce := LowM.Primitive Primitive.GetNonce M.pure in
      let bytes : list Z :=
        Memory.u256_as_bytes address ++ Memory.u256_as_bytes nonce in
      let bytes : list Nibble.byte := Memory.bytes_as_bytes bytes in
      let hash : list Nibble.byte := EVM.Crypto.Keccak.keccak_256 bytes in
      let hash : list Z := (List.map (fun byte => Z.of_N (Nibble.N_of_byte byte))) hash in
      M.pure (Z.land ((2 ^ 160) - 1) (Memory.bytes_as_u256 hash)) in
    if n <? 32 then
      LowM.Impossible "create with code of size lesser than a word"
    else
      let* code := mload p in
      let* codedata := LowM.Primitive (Primitive.MLoad (p + 32) (n - 32)) M.pure in
      let* tt := LowM.Primitive (Primitive.CreateAccount created_address code codedata) M.pure in
      (* The input during the call is empty as it is in the [codedata]. *)
      let* call_contract_status := LowM.CallContract created_address v [] M.pure in
      (* Failure case *)
      if call_contract_status =? 0 then
        M.pure 0
      (* Success case *)
      else
        let* constructor_output := LowM.Primitive Primitive.RLoad M.pure in
        if negb (Z.of_nat (List.length constructor_output) =? 32) then
          LowM.Impossible "create: constructor_output must be a word"
        else
          let deployed_code := Memory.bytes_as_u256 constructor_output in
          LowM.Primitive (Primitive.UpdateCodeForDeploy created_address deployed_code) (fun _ =>
          M.pure created_address).

  Definition create2 (v p n s : U256.t) : M.t U256.t :=
    LowM.Impossible "create2".

  Definition call (g a v in_ insize out outsize : U256.t) : M.t U256.t :=
    let* input := LowM.Primitive (Primitive.MLoad in_ insize) M.pure in
    let* result := LowM.CallContract a v input M.pure in
    let* output := LowM.Primitive Primitive.RLoad M.pure in
    LowM.Primitive (Primitive.MStore out (List.firstn (Z.to_nat outsize) output)) (fun _ =>
    M.pure result).

  Definition callcode (g a v in_ insize out outsize : U256.t) : M.t U256.t :=
    LowM.Impossible "callcode".

  Definition delegatecall (g a in_ insize out outsize : U256.t) : M.t U256.t :=
    LowM.Impossible "delegatecall".

  (* TODO: have a flag such that the operations that change the state fail. *)
  Definition staticcall (g a in_ insize out outsize : U256.t) : M.t U256.t :=
    call g a 0 in_ insize out outsize.

  Definition return_ (p s : U256.t) : M.t unit :=
    LowM.Pure (Result.Return p s).

  Definition revert (p s : U256.t) : M.t unit :=
    LowM.Pure (Result.Revert p s).

  Definition selfdestruct (a : U256.t) : M.t unit :=
    LowM.Impossible "selfdestruct".

  Definition invalid : M.t unit :=
    LowM.Impossible "invalid".

  (* Definition log0 (p s : U256.t) : M.t unit :=
    let* payload := LowM.Primitive (Primitive.MLoad p s) M.pure in
    LowM.Primitive (Primitive.Log [] payload) M.pure.

  Definition log1 (p s t1 : U256.t) : M.t unit :=
    let* payload := LowM.Primitive (Primitive.MLoad p s) M.pure in
    LowM.Primitive (Primitive.Log [t1] payload) M.pure.

  Definition log2 (p s t1 t2 : U256.t) : M.t unit :=
    let* payload := LowM.Primitive (Primitive.MLoad p s) M.pure in
    LowM.Primitive (Primitive.Log [t1; t2] payload) M.pure.

  Definition log3 (p s t1 t2 t3 : U256.t) : M.t unit :=
    let* payload := LowM.Primitive (Primitive.MLoad p s) M.pure in
    LowM.Primitive (Primitive.Log [t1; t2; t3] payload) M.pure.

  Definition log4 (p s t1 t2 t3 t4 : U256.t) : M.t unit :=
    let* payload := LowM.Primitive (Primitive.MLoad p s) M.pure in
    LowM.Primitive (Primitive.Log [t1; t2; t3; t4] payload) M.pure. *)

  (** ** For now we ignore the log operations. *)

  Definition log0 (p s : U256.t) : M.t unit :=
    M.pure tt.

  Definition log1 (p s t1 : U256.t) : M.t unit :=
    M.pure tt.

  Definition log2 (p s t1 t2 : U256.t) : M.t unit :=
    M.pure tt.

  Definition log3 (p s t1 t2 t3 : U256.t) : M.t unit :=
    M.pure tt.

  Definition log4 (p s t1 t2 t3 t4 : U256.t) : M.t unit :=
    M.pure tt.

  Definition chainid : M.t U256.t :=
    LowM.Impossible "chainid".

  Definition basefee : M.t U256.t :=
    LowM.Impossible "basefee".

  Definition blobbasefee : M.t U256.t :=
    LowM.Impossible "blobbasefee".

  Definition origin : M.t U256.t :=
    LowM.Impossible "origin".

  Definition gasprice : M.t U256.t :=
    LowM.Impossible "gasprice".

  Definition blockhash (b : U256.t) : M.t U256.t :=
    LowM.Impossible "blockhash".

  Definition blobhash (i : U256.t) : M.t U256.t :=
    LowM.Impossible "blobhash".

  Definition coinbase : M.t U256.t :=
    LowM.Impossible "coinbase".

  Definition timestamp : M.t U256.t :=
    LowM.Impossible "timestamp".

  Definition number : M.t U256.t :=
    LowM.Impossible "number".

  Definition difficulty : M.t U256.t :=
    LowM.Impossible "difficulty".

  Definition prevrandao : M.t U256.t :=
    LowM.Impossible "prevrandao".

  Definition gaslimit : M.t U256.t :=
    LowM.Impossible "gaslimit".

  Definition loadimmutable (name : U256.t) : M.t U256.t :=
    LowM.Primitive (Primitive.LoadImmutable name) M.pure.

  Definition setimmutable (_offset name value : U256.t) : M.t unit :=
    LowM.Primitive (Primitive.SetImmutable name value) M.pure.

  (** ** Additional functions for the object mode of Yul. *)
  (** We assume that the optimizer does not use any additional memory. *)
  Definition memoryguard (size : U256.t) : M.t U256.t :=
    M.pure size.

  (** For the [dataoffset] function we use the [name] of the code. We shift this
      address by 256 bits in order to be able to complete it with the address constructor call's
      parameters that are concatenated to the constructor's code at startup.

      Having an address that is more than 256 bits long is not realistic but not an issue for us
      as we store the addresses in [Z].
  *)
  Definition dataoffset (name : U256.t) : M.t U256.t :=
    M.pure (name * (2 ^ 256)).

  (** We suppose that the size of the code is a word's size, and is one word that is the name. *)
  Definition datasize (name : U256.t) : M.t U256.t :=
    M.pure 32.

  Definition datacopy (t f s : U256.t) : M.t unit :=
    codecopy t f s.

  Notation "'fn' p '=>' body" :=
    (fun arguments =>
      match arguments with
      | p => body
      | _ => LowM.Impossible "wrong number of arguments"
      end)
    (at level 200, p pattern).

  Definition return_unit (body : M.t unit) : M.t (list U256.t) :=
    M.let_ body (fun _ => M.pure []).

  Definition return_u256 (body : M.t U256.t) : M.t (list U256.t) :=
    M.let_ body (fun result => M.pure [result]).

  Definition functions : list (string * (list U256.t -> M.t (list U256.t))) := [
    ("stop", fn [] => return_unit stop);
    ("add", fn [x; y] => return_u256 (add x y));
    ("sub", fn [x; y] => return_u256 (sub x y));
    ("mul", fn [x; y] => return_u256 (mul x y));
    ("div", fn [x; y] => return_u256 (div x y));
    ("sdiv", fn [x; y] => return_u256 (sdiv x y));
    ("mod", fn [x; y] => return_u256 (mod_ x y));
    ("smod", fn [x; y] => return_u256 (smod x y));
    ("exp", fn [x; y] => return_u256 (exp x y));
    ("not", fn [x] => return_u256 (not x));
    ("lt", fn [x; y] => return_u256 (lt x y));
    ("gt", fn [x; y] => return_u256 (gt x y));
    ("slt", fn [x; y] => return_u256 (slt x y));
    ("sgt", fn [x; y] => return_u256 (sgt x y));
    ("eq", fn [x; y] => return_u256 (eq x y));
    ("iszero", fn [x] => return_u256 (iszero x));
    ("and", fn [x; y] => return_u256 (and x y));
    ("or", fn [x; y] => return_u256 (or x y));
    ("xor", fn [x; y] => return_u256 (xor x y));
    ("byte", fn [n; x] => return_u256 (byte n x));
    ("shl", fn [x; y] => return_u256 (shl x y));
    ("shr", fn [x; y] => return_u256 (shr x y));
    ("sar", fn [x; y] => return_u256 (sar x y));
    ("addmod", fn [x; y; m] => return_u256 (addmod x y m));
    ("mulmod", fn [x; y; m] => return_u256 (mulmod x y m));
    ("signextend", fn [i; x] => return_u256 (signextend i x));
    ("keccak256", fn [p; n] => return_u256 (keccak256 p n));
    ("pc", fn [] => return_u256 pc);
    ("pop", fn [x] => return_unit (pop x));
    ("mload", fn [p] => return_u256 (mload p));
    ("mstore", fn [p; v] => return_unit (mstore p v));
    ("mstore8", fn [p; v] => return_unit (mstore8 p v));
    ("sload", fn [p] => return_u256 (sload p));
    ("sstore", fn [p; v] => return_unit (sstore p v));
    ("tload", fn [p] => return_u256 (tload p));
    ("tstore", fn [p; v] => return_unit (tstore p v));
    ("msize", fn [] => return_u256 msize);
    ("gas", fn [] => return_u256 gas);
    ("address", fn [] => return_u256 address);
    ("balance", fn [a] => return_u256 (balance a));
    ("selfbalance", fn [] => return_u256 selfbalance);
    ("caller", fn [] => return_u256 caller);
    ("callvalue", fn [] => return_u256 callvalue);
    ("calldataload", fn [p] => return_u256 (calldataload p));
    ("calldatasize", fn [] => return_u256 calldatasize);
    ("calldatacopy", fn [p; s; n] => return_unit (calldatacopy p s n));
    ("codesize", fn [] => return_u256 codesize);
    ("codecopy", fn [p; s; n] => return_unit (codecopy p s n));
    ("extcodesize", fn [a] => return_u256 (extcodesize a));
    ("extcodecopy", fn [a; p; s; n] => return_unit (extcodecopy a p s n));
    ("returndatasize", fn [] => return_u256 returndatasize);
    ("returndatacopy", fn [p; s; n] => return_unit (returndatacopy p s n));
    ("mcopy", fn [t; f; s] => return_unit (mcopy t f s));
    ("extcodehash", fn [a] => return_u256 (extcodehash a));
    ("create", fn [v; p; n] => return_u256 (create v p n));
    ("create2", fn [v; p; n; s] => return_u256 (create2 v p n s));
    ("call", fn [g; a; v; in_; insize; out; outsize] =>
      return_u256 (call g a v in_ insize out outsize));
    ("callcode", fn [g; a; v; in_; insize; out; outsize] =>
      return_u256 (callcode g a v in_ insize out outsize));
    ("delegatecall", fn [g; a; in_; insize; out; outsize] =>
      return_u256 (delegatecall g a in_ insize out outsize));
    ("staticcall", fn [g; a; in_; insize; out; outsize] =>
      return_u256 (staticcall g a in_ insize out outsize));
    ("return", fn [p; s] => return_unit (return_ p s));
    ("revert", fn [p; s] => return_unit (revert p s));
    ("selfdestruct", fn [a] => return_unit (selfdestruct a));
    ("invalid", fn [] => return_unit invalid);
    ("log0", fn [p; s] => return_unit (log0 p s));
    ("log1", fn [p; s; t1] => return_unit (log1 p s t1));
    ("log2", fn [p; s; t1; t2] => return_unit (log2 p s t1 t2));
    ("log3", fn [p; s; t1; t2; t3] => return_unit (log3 p s t1 t2 t3));
    ("log4", fn [p; s; t1; t2; t3; t4] => return_unit (log4 p s t1 t2 t3 t4));
    ("chainid", fn [] => return_u256 chainid);
    ("basefee", fn [] => return_u256 basefee);
    ("blobbasefee", fn [] => return_u256 blobbasefee);
    ("origin", fn [] => return_u256 origin);
    ("gasprice", fn [] => return_u256 gasprice);
    ("blockhash", fn [b] => return_u256 (blockhash b));
    ("blobhash", fn [i] => return_u256 (blobhash i));
    ("coinbase", fn [] => return_u256 coinbase);
    ("timestamp", fn [] => return_u256 timestamp);
    ("number", fn [] => return_u256 number);
    ("difficulty", fn [] => return_u256 difficulty);
    ("prevrandao", fn [] => return_u256 prevrandao);
    ("gaslimit", fn [] => return_u256 gaslimit);
    ("loadimmutable", fn [name] => return_u256 (loadimmutable name));
    ("setimmutable", fn [offset; name; value] => return_unit (setimmutable offset name value));
    ("memoryguard", fn [p] => return_u256 (memoryguard p));
    ("dataoffset", fn [name] => return_u256 (dataoffset name));
    ("datasize", fn [name] => return_u256 (datasize name));
    ("datacopy", fn [p; s; n] => return_unit (datacopy p s n))
  ].
End Stdlib.

Module Codes.
  Definition t : Set :=
    list Code.t.

  Fixpoint get (codes : t) (hex_name : U256.t) : option Code.t :=
    match codes with
    | [] => None
    | code :: codes =>
      if code.(Code.hex_name) =? hex_name then
        Some code
      else
        get codes hex_name
    end.

  Definition get_function
      (codes : t)
      (environment : Environment.t)
      (name : string) :
      list U256.t -> M.t (list U256.t) :=
    match Dict.get Stdlib.functions name with
    | Some function => function
    | None =>
      match get codes environment.(Environment.code_name) with
      | None => fun _ =>
        LowM.Impossible (
          "code '" ++ HexString.of_Z environment.(Environment.code_name) ++ "' not found"
        )
      | Some code =>
        match Dict.get code.(Code.functions) name with
        | None => fun _ => LowM.Impossible ("function '" ++ name ++ "' not found")
        | Some function =>
          M.make_function
            name
            function.(Code.Function.arguments)
            function.(Code.Function.results)
            function.(Code.Function.body)
        end
      end
    end.
End Codes.

(** We consider that all the primitives can be defined as a function over the state. *)
Definition eval_primitive {A : Set}
    (environment : Environment.t)
    (primitive : Primitive.t A) :
    State.t -> (A * State.t) + string :=
  fun state =>
  match primitive with
  | Primitive.OpenScope =>
    inl (
      tt,
      state <| State.stack := Stack.open_scope state.(State.stack) |>
    )
  | Primitive.CloseScope =>
    let stack := Stack.close_scope state.(State.stack) in
    match stack with
    | inr error => inr error
    | inl stack =>
      inl (
        tt,
        state <| State.stack := stack |>
      )
    end
  | Primitive.GetVar name =>
    let value := Stack.get_var state.(State.stack) name in
    match value with
    | inr error => inr error
    | inl value =>
      inl (
        value,
        state
      )
    end
  | Primitive.DeclareVars names values =>
    let stack := Stack.declare_vars state.(State.stack) names values in
    match stack with
    | inr error => inr error
    | inl stack =>
      inl (
        tt,
        state <| State.stack := stack |>
      )
    end
  | Primitive.AssignVars names values =>
    let stack := Stack.assign_vars state.(State.stack) names values in
    match stack with
    | inr error => inr error
    | inl stack =>
      inl (
        tt,
        state <| State.stack := stack |>
      )
    end
  | Primitive.MLoad address length =>
    inl (
      Memory.get_bytes state.(State.memory) address length,
      state
    )
  | Primitive.MStore address bytes =>
    inl (
      tt,
      state <| State.memory := Memory.update_bytes state.(State.memory) address bytes |>
    )
  | Primitive.SLoad slot =>
    let address := environment.(Environment.address) in
    let account := Dict.get state.(State.accounts) address in
    match account with
    | None => inr ("storage not found for the address " ++ HexString.of_Z address)%string
    | Some account =>
      inl (
        account.(Account.storage) slot,
        state
      )
    end
  | Primitive.SStore slot value =>
    let address := environment.(Environment.address) in
    let accounts :=
      Dict.assign_function state.(State.accounts) address (fun account =>
        account <| Account.storage := Storage.update account.(Account.storage) slot value |>
      ) in
    match accounts with
    | None => inr ("storage not found for the address " ++ HexString.of_Z address)%string
    | Some accounts =>
      inl (
        tt,
        state <| State.accounts := accounts |>
      )
    end
  | Primitive.RLoad =>
    inl (
      state.(State.return_data),
      state
    )
  | Primitive.TLoad address =>
    inl (
      state.(State.transient_storage) address,
      state
    )
  | Primitive.TStore address value =>
    inl (
      tt,
      state <| State.transient_storage :=
        Storage.update state.(State.transient_storage) address value
      |>
    )
  | Primitive.Log topics payload =>
    inl (
      tt,
      state <| State.logs := (topics, payload) :: state.(State.logs) |>
    )
  | Primitive.GetEnvironment =>
    inl (
      environment,
      state
    )
  | Primitive.GetNonce =>
    let address := environment.(Environment.address) in
    let accounts := state.(State.accounts) in
    match Dict.get accounts address with
    | None => inr ("nonce not found for the address " ++ HexString.of_Z address)%string
    | Some account =>
      inl (
        account.(Account.nonce),
        state
      )
    end
  | Primitive.GetCodedata address =>
    let accounts := state.(State.accounts) in
    match Dict.get accounts address with
    | None => inr ("codedata not found for the address " ++ HexString.of_Z address)%string
    | Some account =>
      inl (
        account.(Account.codedata),
        state
      )
    end
  | Primitive.CreateAccount address code codedata =>
    let account := {|
      Account.balance := 0;
      Account.nonce := 1;
      Account.code := code;
      Account.codedata := codedata;
      Account.storage := Memory.empty;
      Account.immutables := [];
    |} in
    inl (
      tt,
      state <| State.accounts := Dict.declare state.(State.accounts) address account |>
    )
  | Primitive.UpdateCodeForDeploy address code =>
    let accounts := state.(State.accounts) in
    match Dict.assign_function accounts address (fun account =>
      account <| Account.code := code |>
    ) with
    | None => inr ("code not found for the address " ++ HexString.of_Z address)%string
    | Some accounts =>
      inl (
        tt,
        state <| State.accounts := accounts |>
      )
    end
  | Primitive.LoadImmutable name =>
    let address := environment.(Environment.address) in
    let accounts := state.(State.accounts) in
    match Dict.get accounts address with
    | None => inr ("immutables not found for the address " ++ HexString.of_Z address)%string
    | Some account =>
      match Dict.get account.(Account.immutables) name with
      | None => inr ("immutable not found for the name " ++ HexString.of_Z name)%string
      | Some value =>
        inl (
          value,
          state
        )
      end
    end
  | Primitive.SetImmutable name value =>
    let address := environment.(Environment.address) in
    let accounts := state.(State.accounts) in
    match Dict.assign_function accounts address (fun account =>
      account <| Account.immutables := Dict.declare account.(Account.immutables) name value |>
    ) with
    | None => inr ("immutables not found for the address " ++ HexString.of_Z address)%string
    | Some accounts =>
      inl (
        tt,
        state <| State.accounts := accounts |>
      )
    end
  | Primitive.CallStackPush name arguments =>
    inl (
      tt,
      state <| State.call_stack := (name, arguments) :: state.(State.call_stack) |>
    )
  | Primitive.CallStackPop =>
    match state.(State.call_stack) with
    | [] => inr "cannot pop the last element of the call stack"
    | _ :: call_stack =>
      inl (
        tt,
        state <| State.call_stack := call_stack |>
      )
    end
  end.

Definition state_error_bind {S E A B : Set} (e1 : S -> (A + E) * S) (e2 : A -> S -> (B + E) * S) :
    S -> (B + E) * S :=
  fun state =>
  match e1 state with
  | (inl a, state) => e2 a state
  | (inr error, state) => (inr error, state)
  end.

Notation "'letS?' x ':=' e 'in' body" :=
  (state_error_bind e (fun x => body))
  (at level 200, x ident, e at level 200, body at level 200).

(** We assume that there is engouh value in the contract we are transfering from. We also use this
    opportunity to increase the nonce of the account. *)
Definition decrease_value_of_current_contract
    (environment : Environment.t) (state : State.t) (transferred_value : U256.t) :
    State.t + string :=
  let address := environment.(Environment.address) in
  let accounts :=
    Dict.assign_function state.(State.accounts) address (fun account =>
      account
        <| Account.balance := account.(Account.balance) - transferred_value |>
        <| Account.nonce := account.(Account.nonce) + 1 |>
    ) in
  match accounts with
  | None => inr ("balance not found for the address " ++ HexString.of_Z address)%string
  | Some accounts =>
    inl (
      state <| State.accounts := accounts |>
    )
  end.

(** A function to evaluate an expression assuming that we have enough [fuel]. *)
Fixpoint eval {A : Set}
    (fuel : nat)
    (codes : Codes.t)
    (environment : Environment.t)
    (e : LowM.t A) :
    State.t -> (A + string) * State.t :=
  match fuel with
  | O => fun state => (inr "out of fuel", state)
  | S fuel =>
    match e with
    | LowM.Pure output => fun state => (inl output, state)
    | LowM.Primitive primitive k =>
      fun state =>
      let value_state := eval_primitive environment primitive state in
      match value_state with
      | inl (value, state) => eval fuel codes environment (k value) state
      | inr error => (inr error, state)
      end
    | LowM.CallFunction name arguments k =>
      fun state =>
      let function := Codes.get_function codes environment name in
      (letS? results := eval fuel codes environment (function arguments) in
      eval fuel codes environment (k results)) state
    | LowM.Loop body break_with k =>
      letS? output := eval fuel codes environment body in
      match break_with output with
      | None => eval fuel codes environment (LowM.Loop body break_with k)
      | Some output => eval fuel codes environment (k output)
      end
    | LowM.CallContract address value input k => fun state =>
      if value >? environment.(Environment.callvalue) then
        (* When there is not enough balance, the call fails but we do not revert *)
        eval fuel codes environment (k 0) state
      else
        match decrease_value_of_current_contract environment state value with
        | inr error => (inr error, state)
        | inl state =>
          let callee_account := Dict.get state.(State.accounts) address in
          match callee_account with
          | None =>
            (* If the contract only contains code, we transfer the [value] and succeed immediately *)
            eval fuel codes environment (k 1) state
          | Some callee_account =>
            let callee_code_name : U256.t := callee_account.(Account.code) in
            let callee_environment := {|
              Environment.caller := environment.(Environment.address);
              Environment.callvalue := value;
              Environment.calldata := input;
              Environment.address := address;
              Environment.code_name := callee_code_name;
            |} in
            let callee_contract :=
              match Codes.get codes callee_code_name with
              | None => LowM.Impossible "code not found"
              | Some code => code.(Code.body)
              end in
            let callee_state := State.init <| State.accounts := state.(State.accounts) |> in
            let '(result, callee_state) :=
              eval fuel codes callee_environment callee_contract callee_state in
            match result with
            | inr error => (inr error, state)
            | inl (Result.Ok _) => (inr "call: expected a return or a revert", state)
            | inl (Result.Return p s) =>
              let state :=
                state
                  <| State.accounts := callee_state.(State.accounts) |>
                  <| State.return_data := Memory.get_bytes callee_state.(State.memory) p s |> in
              eval fuel codes environment (k 1) state
            | inl (Result.Revert p s) =>
              let state :=
                state
                  <| State.accounts := callee_state.(State.accounts) |>
                  <| State.return_data := Memory.get_bytes callee_state.(State.memory) p s |> in
              eval fuel codes environment (k 0) state
            end
          end
        end
    | LowM.Let e1 k | LowM.Call e1 k =>
      letS? value := eval fuel codes environment e1 in
      eval fuel codes environment (k value)
    | LowM.Impossible message => fun state => (inr ("Impossible: " ++ message)%string, state)
    end
  end.

Module Run.
  Reserved Notation "{{ codes , environment , state | e ⇓ output | state' }}"
    (at level 70, no associativity).

  Inductive t {A : Set} (codes : Codes.t) (environment : Environment.t)
      (state : State.t) (output : A) :
      LowM.t A -> State.t -> Prop :=
  | Pure : {{ codes, environment, state | LowM.Pure output ⇓ output | state }}
  | Primitive {B : Set} (primitive : Primitive.t B) (k : B -> LowM.t A) value state_inter state' :
    inl (value, state_inter) = eval_primitive environment primitive state ->
    {{ codes, environment, state_inter | k value ⇓ output | state' }} ->
    {{ codes, environment, state | LowM.Primitive primitive k ⇓ output | state' }}
  | CallFunction name arguments k results state_inter state' :
    let function := Codes.get_function codes environment name in
    {{ codes, environment, state | function arguments ⇓ results | state_inter }} ->
    {{ codes, environment, state_inter | k results ⇓ output | state' }} ->
    {{ codes, environment, state | LowM.CallFunction name arguments k ⇓ output | state' }}
  | Let {B : Set} (e1 : LowM.t B) k state_inter output_inter state' :
    {{ codes, environment, state | e1 ⇓ output_inter | state_inter }} ->
    {{ codes, environment, state_inter | k output_inter ⇓ output | state' }} ->
    {{ codes, environment, state | LowM.Let e1 k ⇓ output | state' }}
  | Call {B : Set} (e1 : LowM.t B) k state_inter output_inter state' :
    {{ codes, environment, state | e1 ⇓ output_inter | state_inter }} ->
    {{ codes, environment, state_inter | k output_inter ⇓ output | state' }} ->
    {{ codes, environment, state | LowM.Call e1 k ⇓ output | state' }}

  where "{{ codes , environment , state | e ⇓ output | state' }}" :=
    (t codes environment state output e state').
End Run.

Import Run.

Module RunO.
  Reserved Notation "{{? codes , environment , state | e ⇓ output | state' ?}}"
    (at level 70, no associativity).

  Inductive t {A : Set} (codes : Codes.t) (environment : Environment.t) (output : A) :
      option State.t -> LowM.t A -> option State.t -> Prop :=
  | Pure state :
    {{? codes, environment, state | LowM.Pure output ⇓ output | state ?}}
  | PureNone state :
    {{? codes, environment, state | LowM.Pure output ⇓ output | None ?}}
  | Primitive {B : Set} (primitive : Primitive.t B) (k : B -> LowM.t A) value
      state state_inter state' :
    inl (value, state_inter) = eval_primitive environment primitive state ->
    {{? codes, environment, Some state_inter | k value ⇓ output | state' ?}} ->
    {{? codes, environment, Some state | LowM.Primitive primitive k ⇓ output | state' ?}}
  | PrimitiveNone {B : Set} (primitive : Primitive.t B) (k : B -> LowM.t A) state state' :
    (forall (value : B),
      {{? codes, environment, None | k value ⇓ output | state' ?}}
    ) ->
    {{? codes, environment, state | LowM.Primitive primitive k ⇓ output | state' ?}}
  | CallFunction name arguments k results state state_inter state' :
    let function := Codes.get_function codes environment name in
    {{? codes, environment, state | function arguments ⇓ results | state_inter ?}} ->
    {{? codes, environment, state_inter | k results ⇓ output | state' ?}} ->
    {{? codes, environment, state | LowM.CallFunction name arguments k ⇓ output | state' ?}}
  | Let {B : Set} (e1 : LowM.t B) k state state_inter output_inter state' :
    {{? codes, environment, state | e1 ⇓ output_inter | state_inter ?}} ->
    {{? codes, environment, state_inter | k output_inter ⇓ output | state' ?}} ->
    {{? codes, environment, state | LowM.Let e1 k ⇓ output | state' ?}}
  | LetUnfold {B : Set} (e1 : LowM.t B) k state state' :
    {{? codes, environment, state | LowM.let_ e1 k ⇓ output | state' ?}} ->
    {{? codes, environment, state | LowM.Let e1 k ⇓ output | state' ?}}
  | Call {B : Set} (e1 : LowM.t B) k state state_inter output_inter state' :
    {{? codes, environment, state | e1 ⇓ output_inter | state_inter ?}} ->
    {{? codes, environment, state_inter | k output_inter ⇓ output | state' ?}} ->
    {{? codes, environment, state | LowM.Call e1 k ⇓ output | state' ?}}
  | CallUnfold {B : Set} (e1 : LowM.t B) k state state' :
    {{? codes, environment, state | LowM.let_ e1 k ⇓ output | state' ?}} ->
    {{? codes, environment, state | LowM.Call e1 k ⇓ output | state' ?}}

  where "{{? codes , environment , state | e ⇓ output | state' ?}}" :=
    (t codes environment output state e state').

  Lemma If codes environment {A : Set} (condition : bool) (e1 e2 : LowM.t A)
      state output1 output2 state'1 state'2 :
    {{? codes, environment, state | e1 ⇓ output1 | state'1 ?}} ->
    {{? codes, environment, state | e2 ⇓ output2 | state'2 ?}} ->
    {{? codes, environment, state |
      if condition then e1 else e2 ⇓
      if condition then output1 else output2
    | if condition then state'1 else state'2 ?}}.
  Proof.
    now destruct condition.
  Qed.

  Lemma PureEq codes environment {A : Set} (output output' : A) state state' :
    output = output' ->
    state = state' ->
    {{? codes, environment, state | LowM.Pure output ⇓ output' | state' ?}}.
  Proof.
    intros -> ->.
    now constructor.
  Qed.
End RunO.

Import RunO.

Module RunP.
  Reserved Notation "{{{ codes , environment , state | e ⇓ P_output_state' }}}"
    (at level 70, no associativity).

  Inductive t {A : Set} (codes : Codes.t) (environment : Environment.t)
      (state : State.t) (P_output_state' : A -> State.t -> Prop) :
      LowM.t A -> Prop :=
  | Pure (output : A) :
    P_output_state' output state ->
    {{{ codes, environment, state | LowM.Pure output ⇓ P_output_state' }}}
  | Primitive {B : Set} (primitive : Primitive.t B) (k : B -> LowM.t A) value state_inter :
    inl (value, state_inter) = eval_primitive environment primitive state ->
    {{{ codes, environment, state_inter | k value ⇓ P_output_state' }}} ->
    {{{ codes, environment, state | LowM.Primitive primitive k ⇓ P_output_state' }}}
  | CallFunction name arguments k P_results_state_inter :
    let function := Codes.get_function codes environment name in
    {{{ codes, environment, state | function arguments ⇓ P_results_state_inter }}} ->
    (forall results state_inter,
      P_results_state_inter results state_inter ->
      {{{ codes, environment, state_inter | k results ⇓ P_output_state' }}}
    ) ->
    {{{ codes, environment, state | LowM.CallFunction name arguments k ⇓ P_output_state' }}}
  | Let {B : Set} (e1 : LowM.t B) k P_output_state_inter :
    {{{ codes, environment, state | e1 ⇓ P_output_state_inter }}} ->
    (forall output_inter state_inter,
      P_output_state_inter output_inter state_inter ->
      {{{ codes, environment, state_inter | k output_inter ⇓ P_output_state' }}}
     ) ->
    {{{ codes, environment, state | LowM.Let e1 k ⇓ P_output_state' }}}
  | Call {B : Set} (e1 : LowM.t B) k P_output_state_inter :
    {{{ codes, environment, state | e1 ⇓ P_output_state_inter }}} ->
    (forall output_inter state_inter,
      P_output_state_inter output_inter state_inter ->
      {{{ codes, environment, state_inter | k output_inter ⇓ P_output_state' }}}
     ) ->
    {{{ codes, environment, state | LowM.Call e1 k ⇓ P_output_state' }}}

  where "{{{ codes , environment , state | e ⇓ P_output_state' }}}" :=
    (t codes environment state P_output_state' e).

  Ltac apply_pure :=
    now eapply RunP.Pure with (P_output_state' := fun output state' => output = _ /\ state' = _).
End RunP.

Import RunP.

(** The [eval] function follows the semantics given by [Run.t]. *)
(* Fixpoint eval_is_run {A : Set}
    (fuel : nat)
    (environment : Environment.t)
    (state : State.t)
    (e : LowM.t A)
    (output : A)
    (state' : State.t) :
  eval fuel environment state e = (inl output, state') ->
  {{ environment, state | e ⇓ output | state' }}.
Proof.
  destruct fuel as [|fuel]; [discriminate|].
  destruct e; cbn; intros H_eval.
  { (* Pure *)
    inversion H_eval; constructor.
  }
  { (* Primitive *)
    destruct primitive;
      cbn in H_eval;
      constructor;
      eapply eval_is_run;
      eassumption.
  }
  { (* DeclareFunction *)
    constructor.
    eapply eval_is_run.
    eassumption.
  }
  { (* CallFunction *)
    destruct eval as [[results | message] locals_inter] eqn:H_eval_inter in H_eval.
    { econstructor;
        eapply eval_is_run;
        eassumption.
    }
    { discriminate. }
  }
  { (* Let *)
    destruct eval as [[results | message] locals_inter] eqn:H_eval_inter in H_eval.
    { econstructor;
        eapply eval_is_run;
        eassumption.
    }
    { discriminate. }
  }
  { (* Impossible *)
    discriminate.
  }
Qed. *)

Definition eval_with_revert
    (fuel : nat)
    (codes : Codes.t)
    (environment : Environment.t)
    (e : M.t BlockUnit.t)
    (state : State.t) :
    (Result.t BlockUnit.t + string) * State.t :=
  let '(output, state') := eval fuel codes environment e state in
  match output with
  | inl (Result.Revert _ _) => (output, state' <| State.accounts := state.(State.accounts) |>)
  | _ => (output, state')
  end.

Module Compare.
  (** [None] if not a stack primitive, [Some None] for a stack primitive with error,
      and [Some (Some result)] otherwise. *)
  Definition eval_stack_primitive {A : Set} (primitive : Primitive.t A) :
      option (Stack.t -> option (A * Stack.t)) :=
    match primitive with
    | Primitive.OpenScope => Some (fun stack => Some (tt, Stack.open_scope stack))
    | Primitive.CloseScope =>
      Some (fun stack => match Stack.close_scope stack with
      | inr _ => None
      | inl stack => Some (tt, stack)
      end)
    | Primitive.GetVar name =>
      Some (fun stack => match Stack.get_var stack name with
      | inr _ => None
      | inl value => Some (value, stack)
      end)
    | Primitive.DeclareVars names values =>
      Some (fun stack => match Stack.declare_vars stack names values with
      | inr _ => None
      | inl stack => Some (tt, stack)
      end)
    | Primitive.AssignVars names values =>
      Some (fun stack => match Stack.assign_vars stack names values with
      | inr _ => None
      | inl stack => Some (tt, stack)
      end)
    | _ => None
    end.

  Lemma eval_stack_primitive_none_eq {A : Set} (primitive : Primitive.t A) :
    eval_stack_primitive primitive = None ->
    forall (environment : Environment.t) (state : State.t),
    match eval_primitive environment primitive state with
    | inl (_, state') => state'.(State.stack) = state.(State.stack)
    | inr _ => True
    end.
  Proof.
    destruct primitive; simpl; intros; try congruence; hauto l: on.
  Qed.

  Lemma eval_stack_primitive_some_eq {A : Set} (primitive : Primitive.t A) :
    match eval_stack_primitive primitive with
    | Some eval_stack =>
      forall (environment : Environment.t) (state : State.t),
      match eval_primitive environment primitive state, eval_stack state.(State.stack) with
      | inl (value, state'), Some (value', stack') =>
          value = value' /\ state' = state <| State.stack := stack' |>
      | inr _, None => True
      | _, _ => False
      end
    | None => True
    end.
  Proof.
    destruct primitive; simpl; intros; try apply I; hauto l: on.
  Qed.

  Definition map_on_output_state_of_eval_primitive {A : Set}
      (f : State.t -> State.t) (output : (A * State.t) + string) :
      (A * State.t) + string :=
    match output with
    | inl (value, state) => inl (value, f state)
    | inr _ => output
    end.

  Lemma eval_stack_primitive_none_does_not_use_stack {A : Set} (primitive : Primitive.t A) :
    eval_stack_primitive primitive = None ->
    forall (environment : Environment.t) (state : State.t) (stack : Stack.t),
    eval_primitive environment primitive (state <| State.stack := stack |>) =
    map_on_output_state_of_eval_primitive
      (fun state => state <| State.stack := stack |>)
      (eval_primitive environment primitive state).
  Proof.
    intros; destruct primitive; simpl in *; try congruence; hauto lq: on.
  Qed.

  Lemma eval_stack_primitive_some_only_use_stack {A : Set} (primitive : Primitive.t A) :
    eval_stack_primitive primitive <> None ->
    forall (environment : Environment.t) (state state' : State.t),
    eval_primitive environment primitive (state' <| State.stack := state.(State.stack) |>) =
    map_on_output_state_of_eval_primitive
      (fun state => state' <| State.stack := state.(State.stack) |>)
      (eval_primitive environment primitive state).
  Proof.
    intros; destruct primitive; simpl in *; try congruence; hauto q: on.
  Qed.

  Module Liftable.
    Class C (A A' : Set) := {
      lift : A' -> A;
      IsInjection : forall (a1 a2: A'), lift a1 = lift a2 -> a1 = a2;
    }.

    #[refine]
    Global Instance I_BlockUnit : C BlockUnit.t BlockUnit.t := {
      lift := fun v => v;
    }.
    Proof.
      sfirstorder.
    Defined.

    #[refine]
    Global Instance I_Unit : C BlockUnit.t unit := {
      lift := fun _ => BlockUnit.Tt;
    }.
    Proof.
      sauto lq: on.
    Defined.

    #[refine]
    Global Instance I_Tuple0 : C (list U256.t) unit := {
      lift := fun _ => [];
    }.
    Proof.
      sauto lq: on.
    Defined.

    #[refine]
    Global Instance I_Tuple1 : C (list U256.t) U256.t := {
      lift := fun v1 =>  [v1];
    }.
    Proof.
      hauto lq: on.
    Defined.

    #[refine]
    Global Instance I_Tuple2 : C (list U256.t) (U256.t * U256.t) := {
      lift := fun '(v1, v2) => [v1; v2];
    }.
    Proof.
      hauto lq: on.
    Defined.

    #[refine]
    Global Instance I_Tuple3 : C (list U256.t) (U256.t * U256.t * U256.t) := {
      lift := fun '(v1, v2, v3) => [v1; v2; v3];
    }.
    Proof.
      hauto lq: on.
    Defined.

    #[refine]
    Global Instance I_Tuple4 : C (list U256.t) (U256.t * U256.t * U256.t * U256.t) := {
      lift := fun '(v1, v2, v3, v4) => [v1; v2; v3; v4];
    }.
    Proof.
      hauto lq: on.
    Defined.

    #[refine]
    Global Instance I_Tuple5 : C (list U256.t) (U256.t * U256.t * U256.t * U256.t * U256.t) := {
      lift := fun '(v1, v2, v3, v4, v5) => [v1; v2; v3; v4; v5];
    }.
    Proof.
      hauto lq: on.
    Defined.
  End Liftable.

  Lemma result_map_injection {A A' : Set} `{Liftable.C A A'}
      (output1 output2 : Result.t A') :
    Result.map Liftable.lift output1 = Result.map Liftable.lift output2 ->
    output1 = output2.
  Proof.
    destruct H, output1, output2; cbn; hauto q: on.
  Qed.

  Definition is_function_name_pure (name : string) : bool :=
    match name with
    | "add"
    | "sub"
    | "mul"
    | "div"
    | "sdiv"
    | "mod"
    | "smod"
    | "exp"
    | "not"
    | "lt"
    | "gt"
    | "slt"
    | "sgt"
    | "eq"
    | "iszero"
    | "and"
    | "or"
    | "xor"
    | "byte"
    | "shl"
    | "shr"
    | "sar"
    | "addmod"
    | "mulmod"
    | "signextend" => true
    | _ => false
    end.

  (** We use this predicate to construct a proof that a program without stack is equivalent to the
      original version putting local variables on a stack of scopes.
      We do not yet have a proof that this predicate is correct. This is a work that we will need
      to do, but we first need to introduce concepts to reason at the meta-level of the language,
      introducing concepts like simulations between computations. *)
  Inductive t
      (codes : Codes.t) (environment : Environment.t) (stack : Stack.t) :
      forall {A A' : Set} `{Liftable.C A A'},
      Stack.t -> M.t A -> M.t A' -> Prop :=
  | Pure {A A' : Set} `{Liftable.C A A'}
      (output : Result.t A) (output' : Result.t A') :
    output = Result.map Liftable.lift output' ->
    t codes environment stack stack
      (LowM.Pure output) (LowM.Pure output')
  | PrimitiveNonStack {A A' : Set} `{Liftable.C A A'} {B : Set}
      (primitive : Primitive.t B) (k1 : B -> M.t A) (k2 : B -> M.t A') stack' :
    eval_stack_primitive primitive = None ->
    (forall value,
      t codes environment stack stack'
        (k1 value) (k2 value)
    ) ->
    t codes environment stack stack'
      (LowM.Primitive primitive k1) (LowM.Primitive primitive k2)
  | PrimitiveStack {A A' : Set} `{Liftable.C A A'} {B : Set}
      (primitive : Primitive.t B) (value : B) (k1 : B -> M.t A) (e2 : M.t A')
      eval_stack stack_inter stack' :
    eval_stack_primitive primitive = Some eval_stack ->
    eval_stack stack = Some (value, stack_inter) ->
    t codes environment stack_inter stack' (k1 value) e2 ->
    t codes environment stack stack' (LowM.Primitive primitive k1) e2
  | CallFunction {A A' : Set} `{Liftable.C A A'} {B : Set} `{Liftable.C (list U256.t) B}
      name arguments
      (k1 : Result.t (list U256.t) -> M.t A) (e2 : M.t B) (k2 : Result.t B -> M.t A') stack' :
    let f1 : list U256.t -> M.t (list U256.t) :=
      Codes.get_function codes environment name in
    t codes environment stack stack (f1 arguments) e2 ->
    (forall (value : Result.t B),
      t codes environment stack stack'
        (k1 (Result.map Liftable.lift value)) (k2 value)
    ) ->
    t codes environment stack stack'
      (LowM.CallFunction name arguments k1)
      (LowM.Call e2 k2)
  | LetUnfold {A A' : Set} `{Liftable.C A A'} {B B' : Set}
      (e1 : M.t B) (k1 : Result.t B -> M.t A) (e2 : M.t B') (k2 : Result.t B' -> M.t A') stack' :
    t codes environment stack stack'
      (LowM.let_ e1 k1)
      (LowM.let_ e2 k2) ->
    t codes environment stack stack'
      (LowM.Let e1 k1)
      (LowM.Let e2 k2)
  | Let {A A' : Set} `{Liftable.C A A'} {B B' : Set} `{Liftable.C B B'}
      (e1 : M.t B) (k1 : Result.t B -> M.t A) (e2 : M.t B') (k2 : Result.t B' -> M.t A')
      stack_inter stack' :
    t codes environment stack stack_inter
      e1 e2 ->
    (forall (value : Result.t B'),
      t codes environment stack_inter stack'
        (k1 (Result.map Liftable.lift value)) (k2 value)
    ) ->
    t codes environment stack stack'
      (LowM.Let e1 k1)
      (LowM.Let e2 k2)
  | ReturnUnit (body : M.t unit) :
    t codes environment stack stack
      (Stdlib.return_unit body)
      body
  | ReturnU256 (body : M.t U256.t) :
    t codes environment stack stack
      (Stdlib.return_u256 body)
      body.

  (** This is a beginning of proof for the comparison predicate above. We will need more
      preliminary work like defining what it means for a computation to simulate another one, how
      to reason on potentially non-terminating terms, clearly separating the stack operations in
      the semantics, ... *)
  Module WorkInProgressVerifytheComparePredicate.
  (*
  Module ImpliesOriginalEval.
    Definition t {A A' : Set} `{Liftable.C A A'}
        (codes : Codes.t) (environment : Environment.t) (stack stack' : Stack.t)
        (body_with_stack : M.t A) (body : M.t A') : Prop :=
      forall (fuel : nat) (state state' : State.t) (output : Result.t A'),
      eval fuel codes environment body state = (inl output, state') ->
      exists fuel_with_stack,
      eval fuel_with_stack codes environment body_with_stack (state <| State.stack := stack |>) =
        (inl (Result.map Liftable.lift output), (state' <| State.stack := stack' |>)).
  End ImpliesOriginalEval.

  Module DoesNotUseStack.
    Definition t {A' : Set}
        (codes : Codes.t) (environment : Environment.t) (body : M.t A') : Prop :=
      forall (fuel : nat) (state : State.t) (stack : Stack.t),
      eval fuel codes environment body (state <| State.stack := stack |>) =
      let '(result, state') := eval fuel codes environment body state in
      (result, state' <| State.stack := stack |>).
  End DoesNotUseStack.

  Module Def.
    Record t {A A' : Set} `{Liftable.C A A'}
        (codes : Codes.t) (environment : Environment.t) (stack stack' : Stack.t)
        (body_with_stack : M.t A) (body : M.t A') : Prop := {
      implies_original_eval :
        ImpliesOriginalEval.t codes environment stack stack' body_with_stack body;
      does_not_use_stack :
        DoesNotUseStack.t codes environment body;
    }.
  End Def.

  Lemma pure {A A' : Set} `{Liftable.C A A'}
      (codes : Codes.t) (environment : Environment.t) (stack : Stack.t)
      (output : Result.t A) (output' : Result.t A')
      (H_output : output = Result.map Liftable.lift output') :
    Def.t codes environment stack stack
      (LowM.Pure output) (LowM.Pure output').
  Proof.
    constructor.
    { unfold ImpliesOriginalEval.t; intros.
      exists fuel.
      destruct fuel; best.
    }
    { unfold DoesNotUseStack.t; intros.
      destruct fuel; best.
    }
  Qed.

  Lemma primitive_no_stack {A A' : Set} `{Liftable.C A A'}
      (codes : Codes.t) (environment : Environment.t) (stack : Stack.t)
      {B : Set} (primitive : Primitive.t B) k1 k2 stack'
      (H_primitive : eval_stack_primitive primitive = None)
      (H_k : forall value,
        Def.t codes environment stack stack'
          (k1 value) (k2 value)
      ) :
    Def.t codes environment stack stack'
      (LowM.Primitive primitive k1) (LowM.Primitive primitive k2).
  Proof.
    constructor.
    { unfold ImpliesOriginalEval.t; intros * H_eval.
      destruct fuel as [|fuel]; simpl in *; [congruence|].
      match goal with
      | H : _ |- _ => pose proof (eval_stack_primitive_none_eq _ H environment state)
      end.
      destruct eval_primitive eqn:?; [|congruence].
      Tactics.destruct_pairs.
      match goal with
      | value : B |- _ => destruct (H_k value)
      end.
      unfold ImpliesOriginalEval.t in implies_original_eval.
      epose proof (H_eval_k := implies_original_eval _ _ _ _ H_eval).
      destruct H_eval_k as [fuel_with_stack ?].
      exists (S fuel_with_stack).
      simpl.
      best.
      Unshelve.
      best.
    }
    { unfold DoesNotUseStack.t; intros.
      destruct fuel; simpl; [reflexivity|].
      rewrite eval_stack_primitive_none_does_not_use_stack by assumption.
      destruct eval_primitive; simpl; [|reflexivity].
      Tactics.destruct_pairs.
      match goal with
      | value : B |- _ => destruct (H_k value)
      end.
      apply does_not_use_stack.
    }
  Qed.

  Lemma primitive_stack {A A' : Set} `{Liftable.C A A'}
      (codes : Codes.t) (environment : Environment.t) (stack : Stack.t)
      {B : Set}
      (primitive : Primitive.t B) (value : B) (k1 : B -> M.t A) (e2 : M.t A')
      eval_stack stack_inter stack'
      (H_eval_stack : eval_stack_primitive primitive = Some eval_stack)
      (H_stack : eval_stack stack = Some (value, stack_inter))
      (H_k : Def.t codes environment stack_inter stack' (k1 value) e2) :
    Def.t codes environment stack stack' (LowM.Primitive primitive k1) e2.
  Proof.
    constructor.
    { unfold ImpliesOriginalEval.t; intros * H_state H_state' H_eval.
      epose proof (H_eval_eq := eval_stack_primitive_some_eq primitive).
      pose proof (H_eval_primitive_only_stack :=
        eval_stack_primitive_some_only_use_stack primitive ltac:(congruence) environment).
      destruct eval_stack_primitive; [|congruence].
      injection H_eval_stack; intros H_eval_stack_eq;
        rewrite H_eval_stack_eq in *; clear H_eval_stack_eq.
      pose proof (H_eval_eq environment state); clear H_eval_eq.
      rewrite H_state in *.
      rewrite H_stack in *.
      destruct eval_primitive eqn:?; [|easy].
      Tactics.destruct_pairs.
      subst.
      destruct H_k.
      unfold ImpliesOriginalEval.t, DoesNotUseStack.t in *.
      epose proof (H_k_instance :=
        implies_original_eval fuel (state<|State.stack:= stack_inter|>) state' output
        _ ltac:(reflexivity)).
  Admitted.

  Definition def {A A' : Set} `{Liftable.C A A'}
      (codes : Codes.t) (environment : Environment.t) (stack stack' : Stack.t)
      (body_with_stack : M.t A) (body : M.t A') : Prop :=
    forall (fuel : nat) (state state' : State.t) (output : Result.t A'),
    eval fuel codes environment body_with_stack (state <| State.stack := stack |>) =
      (inl (Result.map Liftable.lift output), state') ->
    state'.(State.stack) = stack' /\
    eval fuel codes environment body state =
      (inl output, state' <| State.stack := state.(State.stack) |>).

  Lemma pure {A A' : Set} `{Liftable.C A A'}
      (codes : Codes.t) (environment : Environment.t) (stack : Stack.t)
      (output : Result.t A) (output' : Result.t A')
      (H_output : output = Result.map Liftable.lift output') :
    def codes environment stack stack
      (LowM.Pure output) (LowM.Pure output').
  Proof.
    unfold def; intros.
    destruct fuel; simpl in *; best using result_map_injection.
  Qed.

  Definition def {A A' : Set} `{Liftable.C A A'}
      (codes : Codes.t) (environment : Environment.t) (stack stack' : Stack.t)
      (body_with_stack : M.t A) (body : M.t A') : Prop :=
    forall (state : State.t),
    exists (state' : State.t) (output : Result.t A'),
    {{ codes, environment, state <| State.stack := stack |> |
      body_with_stack ⇓
      Result.map Liftable.lift output
    | state' <| State.stack := stack' |> }}.

  Fixpoint def_implies {A A' : Set} `{Liftable.C A A'}
      (codes : Codes.t) (environment : Environment.t) (stack stack' : Stack.t)
      (body_with_stack : M.t A) (body : M.t A')
      (H_compare : t codes environment stack stack' body_with_stack body) :
    def codes environment stack stack' body_with_stack body.
  Proof.
    destruct H_compare;
      unfold def; intros; repeat eexists.
    { subst.
      apply Run.Pure.
    }
    { eapply Run.Primitive.

    }
  Qed.
  *)
  End WorkInProgressVerifytheComparePredicate.

  Module Tactic.
    Ltac stack_primitives :=
      repeat (eapply Compare.PrimitiveStack; [reflexivity|reflexivity|]).

    Ltac make_intro :=
      intros []; repeat (eapply Compare.PrimitiveStack; [reflexivity|reflexivity|]);
        try now apply Compare.Pure.


    Ltac expression helper :=
      repeat (
        (* Literal *)
        (now apply Compare.Pure) ||
        (* Variable *)
        Compare.Tactic.stack_primitives ||
        (* Impure function call *)
        (eapply Compare.CallFunction; [
          apply Compare.ReturnUnit ||
          apply Compare.ReturnU256 ||
          helper |
          Compare.Tactic.make_intro
        ])
      ).

    Ltac open_if :=
      unfold M.if_, M.if_unit; simpl;
        match goal with
        | |- Compare.t _ _ _ _ (if ?cond then _ else _) _ =>
          destruct cond; [now apply Compare.Pure|]
        end.

    Ltac open_switch_case :=
      match goal with
      | |- Compare.t _ _ _ _ ?left ?right =>
        let left' := eval hnf in left in
        change left with left';
        let right' := eval hnf in right in
        change right with right'
      end;
      match goal with
      | |- Compare.t _ _ _ _ (if ?cond then _ else _) _ =>
        destruct cond
      end.
  End Tactic.
End Compare.

Definition update_current_code_for_deploy (hex_name : Z) : M.t BlockUnit.t :=
  let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
  let address := environment.(Environment.address) in
  let* tt := LowM.Primitive (Primitive.UpdateCodeForDeploy address hex_name) M.pure in
  M.pure BlockUnit.Tt.

(** We design the testing primitives so that, in case of error, we can see what was wrong. *)
Module Test.
  (** Supposed to be equal to [inl]. *)
  Definition is_return (result : Result.t BlockUnit.t + string) (state : State.t) :
      list Z + (Result.t BlockUnit.t + string) :=
    match result with
    | inl (Result.Return start length) =>
      let output := Memory.get_bytes state.(State.memory) start length in
      inl output
    | _ => inr result
    end.

  Module Status.
    Inductive t : Set :=
    | Success
    | Failure
    | OutOfGas.
  End Status.

  (** Supposed to be equal to [inl]. *)
  Definition extract_output
      (result : Result.t BlockUnit.t + string)
      (state : State.t)
      (status : Status.t) :
      list Z + (Result.t BlockUnit.t + string) :=
    match status with
    | Status.Success =>
      match result with
      | inl (Result.Return start length) =>
        let output := Memory.get_bytes state.(State.memory) start length in
        inl output
      | _ => inr result
      end
    | Status.Failure =>
      match result with
      | inl (Result.Revert start length) =>
        let output := Memory.get_bytes state.(State.memory) start length in
        inl output
      | _ => inr result
      end
    | Status.OutOfGas =>
      (* We rely on the fact that most tests that end with an "out of gas" error also make
         an "out of fuel" error, even if these two measures of the execution steps are different. *)
      match result with
      | inr "out of fuel" => inl []
      | _ => inr result
      end
    end.
End Test.

Definition declared_vars (state : State.t) : list (list (string * U256.t)) :=
  state.(State.stack).
