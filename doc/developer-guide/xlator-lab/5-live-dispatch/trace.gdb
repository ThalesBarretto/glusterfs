set pagination off
set breakpoint pending on

# WIND side: our xlator's lookup handler
break skel_lookup
commands
  printf "\n>>> WIND: skel_lookup entered (this=%s); about to STACK_WIND lookup to FIRST_CHILD\n", this->name
  continue
end

# UNWIND side: the default callback skel_lookup passed to STACK_WIND
break default_lookup_cbk
commands
  printf "\n>>> UNWIND: default_lookup_cbk fired (skel's cbk). Stack (note skel_lookup still below = cbk ran inside the wind):\n"
  bt 7
  continue
end

run
