cc_library(
    name = 'aggregators',
    hdrs = ['aggregators.h'],
    srcs = ['aggregators.cc'],
    deps = [
        ':base',
        ':output',
        ':types',
        '@com_google_absl//absl/strings',
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
    ],
)

cc_library(
    name = 'no-aggregation',
    hdrs = ['no-aggregation.h'],
    srcs = ['no-aggregation.cc'],
    deps = [
        ':base',
        ':table',
        '@com_google_absl//absl/container:flat_hash_set',
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
    name = 'single-aggregation',
    hdrs = ['single-aggregation.h'],
    deps = [
        ':base',
        ':table',
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
        ':no-aggregation',
        ':single-aggregation',
        ':table',
    ],
)

cc_library(
    name = 'table',
    hdrs = ['table.h'],
    deps = [
        ':output',
        ':types',
        '@com_google_absl//absl/container:flat_hash_map',
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
