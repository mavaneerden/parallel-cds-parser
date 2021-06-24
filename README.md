# Descriptor-processing parser


## Optimisation macros
Define these macros to use certain optimisations.
### Pool of threads
- `OPTIMISATION_POOL_GLL_P`: Implements the `P` set from the GLL algorithm.

### Tree of threads
- `OPTIMISATION_TREE_FUTURE`: Uses futures and promises to collect the output.