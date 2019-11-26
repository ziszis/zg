cc_library(
    name = 'aggregators',
    hdrs = ['aggregators.h'],
    deps = [
        ':base',
        '@com_google_absl//absl/strings',
    ],
)

cc_library(
    name = 'base',
    hdrs = ['base.h'],
    srcs = ['base.cc'],
    deps = [
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
    name = 'output',
    hdrs = ['output.h'],
    srcs = ['output.cc'],
    deps = [
        ':base',
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
        ':aggregators',
        ':base',
        ':input',
        ':output',
        ':types',
        ':table',
        '@com_google_absl//absl/strings',
    ],
)
