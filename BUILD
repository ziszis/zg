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
    name = 'filter-table',
    hdrs = ['filter-table.h'],
    srcs = ['filter-table.cc'],
    deps = [
        ':spec',
        ':table',
        '@com_github_google_re2//:re2',
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
        ':composite-key',
        ':no-keys',
        ':output',
        ':single-key',
        ':table',
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
        ':table',
    ],
)

cc_library(
    name = 'pipeline',
    hdrs = ['pipeline.h'],
    srcs = ['pipeline.cc'],
    deps = [
        ':aggregators',
        ':base',
        ':composite-key',
        ':filter-table',
        ':multi-aggregation',
        ':no-keys',
        ':output',
        ':single-key',
        ':spec',
        ':table',
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
        '@com_google_absl//absl/strings',
    ],
)

cc_test(
    name = 'spec_test',
    srcs = ['spec_test.cc'],
    deps = [
        ':spec',
        '@com_google_test//:gtest_main',
    ],
)

cc_library(
    name = 'spec-parser',
    hdrs = ['spec-parser.h'],
    srcs = ['spec-parser.cc'],
    deps = [
        ':base',
        ':spec',
        '@com_google_absl//absl/strings',
    ],
)

cc_test(
    name = 'spec-parser_test',
    srcs = ['spec-parser_test.cc'],
    deps = [
        ':spec-parser',
        '@com_google_test//:gtest_main',
    ],
)

cc_library(
    name = 'table',
    hdrs = ['table.h'],
    deps = [
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
        ':pipeline',
        ':spec',
        ':spec-parser',
        ':types',
    ],
)
