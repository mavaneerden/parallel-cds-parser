# Parallel CDS parser
Parallel versions of the Closing a Descriptor Set (CDS) parsing algorithm.

## Optimisation macros
Define these macros to use certain optimisations.

### Pool of threads
- `OPTIMISATION_POOL_GLL_P`: Implements the `P` set from the GLL algorithm.
- `OPTIMISATION_POOL_SHARED_LOCKS`: Uses read-write locks, used for Version 2.
- `OPTIMISATION_POOL_QUEUES`: Uses separate worklists for each thread, used for Version 3.

### Tree of threads
- `OPTIMISATION_TREE_FUTURE`: Uses futures and promises to collect the output.
- `OPTIMISATION_TREE_GRANULAR_GLOBAL`: Attempt at making the global descriptor set more granular. Results in incorrect output for some grammars.
- `OPTIMISATION_TREE_BETTER_LOCAL_SET`: Attempt at moving new descriptors from a global set to a local set. Results in incorrect output for some grammars.
- `OPTIMISATION_TREE_GLOBAL_DESCRIPTORS`: Uses global descriptor set instead of local, used for Version 1.
- `OPTIMISATION_TREE_COST_REDUCTION_GLOBAL_DESCRIPTORS`: Reduces cost by checking the global descriptor set again, used for Version 2.
