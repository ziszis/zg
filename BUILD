cc_library(
    name = 'aggregators',
    hdrs = ['aggregators.h'],
    srcs = ['aggregators.cc'],
    deps = [
        ':base',
        ':output',
        ':types',
        '@com_google_absl//absl/strings:str_format',
    ],
)

cc_library(
    name = 'base',
    hdrs = ['base.h'],
    srcs = ['base.cc'],
    deps = [
        '@com_google_absl//absl/strings',
    ],
)

cc_library(
    name = 'composite-key',
    hdrs = ['composite-key.h'],
    deps = [
        ':table',
        ':varint',
        '@com_google_absl//absl/container:flat_hash_map',
        '@com_google_absl//absl/container:flat_hash_set',
    ],
)

cc_library(
    name = 'input',
    hdrs = ['input.h'],
    srcs = ['input.cc'],
    deps = [
        ':types',
    ],
)

cc_library(
    name = 'multi-aggregation',
    hdrs = ['multi-aggregation.h'],
    srcs = ['multi-aggregation.cc'],
    deps = [
        ':base',
        ':table',
        ':no-keys',
        ':single-key',
        ':composite-key',
    ],
)

cc_library(
    name = 'no-keys',
    hdrs = ['no-keys.h'],
    deps = [
        ':table',
    ],
)

cc_library(
    name = 'output',
    hdrs = ['output.h'],
    srcs = ['output.cc'],
    deps = [
        ':base',
    ],
)

cc_library(
    name = 'single-key',
    hdrs = ['single-key.h'],
    deps = [
        ':table',
        '@com_google_absl//absl/container:flat_hash_map',
        '@com_google_absl//absl/container:flat_hash_set',
    ],
)

cc_library(
    name = 'spec',
    hdrs = ['spec.h'],
    srcs = ['spec.cc'],
    deps = [
        ':aggregators',
        ':base',
        ':multi-aggregation',
        ':no-keys',
        ':single-key',
        ':composite-key',
        ':table',
    ],
)

cc_library(
    name = 'table',
    hdrs = ['table.h'],
    deps = [
        ':output',
        ':types',
    ],
)

cc_library(
    name = 'types',
    hdrs = ['types.h'],
    srcs = ['types.cc'],
    deps = [
        ':base',
        '@com_google_absl//absl/strings',
    ],
)

cc_library(
    name = 'varint',
    hdrs = ['varint.h'],
    srcs = ['varint.cc'],
    deps = [
        ':base',
    ],
)

cc_binary(
    name = 'varint_bench',
    srcs = ['varint_bench.cc'],
    deps = [
        ':varint',
        '@com_github_google_benchmark//:benchmark_main',
    ],
)

cc_test(
    name = 'varint_test',
    srcs = ['varint_test.cc'],
    deps = [
        ':varint',
        '@com_google_test//:gtest_main',
    ],
)

cc_binary(
    name = 'zg',
    srcs = ['zg.cc'],
    deps = [
        ':base',
        ':input',
        ':spec',
        ':types',
    ],
)
