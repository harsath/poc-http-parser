### Benchmarks per-{commit, parsing-method}
All benchmarks are tested under CPU with spec in `cpu_spec.txt`.
#### HTTP Zero Memory Allocation Parser:
- At(Commit Hash): d2cc8aa972ae73a2781e6135d1cebc9f0ccf9c8b
- Bench: 10000000 GET requests(see `./http_zero_mem_alloc_parser_bench.c`) in ~5.401514 seconds
#### HTTP State Machine:
- At(Commit Hash): 6c02a4f7e8a68dc49b3e760d2103e668f1cdbc1b
- Bench: 10000000 GET requests(see `http_parser_state_machine_bench.c`) in ~15.009198 seconds
#### HTTP Parsing(method two):
- At(Commit Hash): 67a9fb03c28f6e7683815d894203a9d87d4e6b56
- Bench: 10000000 GET requests(see `http_parser_meth_two_bench.c`) in ~15.259364 seconds
